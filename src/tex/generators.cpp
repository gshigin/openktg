#include <cassert>

#include <openktg/core/pixel.h>
#include <openktg/core/texture.h>
#include <openktg/noise/perlin.h>
#include <openktg/tex/generators.h>
#include <openktg/tex/sampling.h>
#include <openktg/util/utility.h>

void Noise(openktg::texture &input, const openktg::texture &grad, int32_t freqX, int32_t freqY, int32_t oct, float fadeoff, int32_t seed, int32_t mode)
{
    assert(oct > 0);

    seed = PerlinNoise::P(seed);

    int32_t offset;
    float scaling;

    if (mode & NoiseNormalize)
        scaling = (fadeoff - 1.0f) / (std::pow(fadeoff, oct) - 1.0f);
    else
        scaling = std::min(1.0f, 1.0f / fadeoff);

    if (mode & NoiseAbs) // absolute mode
    {
        offset = 0;
        scaling *= (1 << 24);
    }
    else
    {
        offset = 1 << 23;
        scaling *= (1 << 23);
    }

    int32_t offsX = (1 << (16 - input.shift_x() + freqX)) >> 1;
    int32_t offsY = (1 << (16 - input.shift_y() + freqY)) >> 1;

    openktg::pixel *out = input.data();
    for (int32_t y = 0; y < input.height(); y++)
    {
        for (int32_t x = 0; x < input.width(); x++)
        {
            int32_t n = offset;
            float s = scaling;

            int32_t px = (x << (16 - input.shift_x() + freqX)) + offsX;
            int32_t py = (y << (16 - input.shift_y() + freqY)) + offsY;
            int32_t mx = (1 << freqX) - 1;
            int32_t my = (1 << freqY) - 1;

            for (int32_t i = 0; i < oct; i++)
            {
                float nv = (mode & NoiseBandlimit) ? PerlinNoise::Noise2(px, py, mx, my, seed) : PerlinNoise::GNoise2(px, py, mx, my, seed);
                if (mode & NoiseAbs)
                    nv = std::fabs(nv);

                n += nv * s;
                s *= fadeoff;

                px += px;
                py += py;
                mx += mx + 1;
                my += my + 1;
            }

            SampleGradient(grad, *out, n);
            out++;
        }
    }
}

void GlowRect(openktg::texture &input, const openktg::texture &bgTex, const openktg::texture &grad, float orgx, float orgy, float ux, float uy, float vx,
              float vy, float rectu, float rectv)
{
    assert(texture_size_matches(input, bgTex));

    // copy background over (if we're not the background texture already)
    if (&input != &bgTex)
    {
        input = bgTex;
    }

    // calculate bounding rect
    int32_t minX = std::max<int32_t>(0, floor((orgx - std::fabs(ux) - std::fabs(vx)) * input.width()));
    int32_t minY = std::max<int32_t>(0, floor((orgy - std::fabs(uy) - std::fabs(vy)) * input.height()));
    int32_t maxX = std::min<int32_t>(input.width() - 1, ceil((orgx + std::fabs(ux) + std::fabs(vx)) * input.width()));
    int32_t maxY = std::min<int32_t>(input.height() - 1, ceil((orgy + std::fabs(uy) + std::fabs(vy)) * input.height()));

    // solve for u0,v0 and deltas (cramer's rule)
    float detM = ux * vy - uy * vx;
    if (std::fabs(detM) * input.width() * input.height() < 0.25f) // smaller than a pixel? skip it.
        return;

    float invM = (1 << 16) / detM;
    float rmx = (minX + 0.5f) / input.width() - orgx;
    float rmy = (minY + 0.5f) / input.height() - orgy;
    int32_t u0 = (rmx * vy - rmy * vx) * invM;
    int32_t v0 = (ux * rmy - uy * rmx) * invM;
    int32_t dudx = vy * invM / input.width();
    int32_t dvdx = -uy * invM / input.width();
    int32_t dudy = -vx * invM / input.height();
    int32_t dvdy = ux * invM / input.height();
    int32_t ruf = std::min<int32_t>(rectu * 65536.0f, 65535);
    int32_t rvf = std::min<int32_t>(rectv * 65536.0f, 65535);
    float gus = 1.0f / (65536.0f - ruf);
    float gvs = 1.0f / (65536.0f - rvf);

    for (int32_t y = minY; y <= maxY; y++)
    {
        openktg::pixel *out = &input.at(minX, y); // input.Data[y * input.width() + minX];
        int32_t u = u0;
        int32_t v = v0;

        for (int32_t x = minX; x <= maxX; x++)
        {
            if (u > -65536 && u < 65536 && v > -65536 && v < 65536)
            {
                openktg::pixel col;

                int32_t du = std::max(std::abs(u) - ruf, 0);
                int32_t dv = std::max(std::abs(v) - rvf, 0);

                if (!du && !dv)
                {
                    SampleGradient(grad, col, 0);
                    *out = compositeROver(*out, col);
                }
                else
                {
                    float dus = du * gus;
                    float dvs = dv * gvs;
                    float dist = dus * dus + dvs * dvs;

                    if (dist < 1.0f)
                    {
                        SampleGradient(grad, col, (1 << 24) * std::sqrt(dist));
                        *out = compositeROver(*out, col);
                    }
                }
            }

            u += dudx;
            v += dvdx;
            out++;
        }

        u0 += dudy;
        v0 += dvdy;
    }
}

struct CellPoint
{
    int32_t x;
    int32_t y;
    int32_t distY;
    int32_t node;
};

void Cells(openktg::texture &input, const openktg::texture &grad, const CellCenter *centers, int32_t nCenters, float amp, int32_t mode)
{
    assert(((mode & 1) == 0) ? nCenters >= 1 : nCenters >= 2);

    openktg::pixel *out = input.data();
    CellPoint *points = NULL;

    points = new CellPoint[nCenters];

    // convert cell center coordinates to fixed point
    static const int32_t scaleF = 14; // should be <=14 for 32-bit ints.
    static const int32_t scale = 1 << scaleF;

    for (int32_t i = 0; i < nCenters; i++)
    {
        points[i].x = int32_t(centers[i].x * scale + 0.5f) & (scale - 1);
        points[i].y = int32_t(centers[i].y * scale + 0.5f) & (scale - 1);
        points[i].distY = -1;
        points[i].node = i;
    }

    int32_t stepX = 1 << (scaleF - input.shift_x());
    int32_t stepY = 1 << (scaleF - input.shift_y());
    int32_t yc = stepY >> 1;

    amp = amp * (1 << 24);

    for (int32_t y = 0; y < input.height(); y++)
    {
        int32_t xc = stepX >> 1;

        // calculate new y distances
        for (int32_t i = 0; i < nCenters; i++)
        {
            int32_t dy = (yc - points[i].y) & (scale - 1);
            points[i].distY = openktg::util::square(std::min(dy, scale - dy));
        }

        // (insertion) sort by y-distance
        for (int32_t i = 1; i < nCenters; i++)
        {
            CellPoint v = points[i];
            int32_t j = i;

            while (j && points[j - 1].distY > v.distY)
            {
                points[j] = points[j - 1];
                j--;
            }

            points[j] = v;
        }

        int32_t best, best2;
        int32_t besti, best2i;

        best = best2 = openktg::util::square(scale);
        besti = best2i = -1;

        for (int32_t x = 0; x < input.width(); x++)
        {
            int32_t t, dx;

            // update "best point" stats
            if (besti != -1 && best2i != -1)
            {
                dx = (xc - points[besti].x) & (scale - 1);
                best = openktg::util::square(std::min(dx, scale - dx)) + points[besti].distY;

                dx = (xc - points[best2i].x) & (scale - 1);
                best2 = openktg::util::square(std::min(dx, scale - dx)) + points[best2i].distY;
                if (best2 < best)
                {
                    std::swap(best, best2);
                    std::swap(besti, best2i);
                }
            }

            // search for better points
            for (int32_t i = 0; i < nCenters && best2 > points[i].distY; i++)
            {
                int32_t dx = (xc - points[i].x) & (scale - 1);
                dx = openktg::util::square(std::min(dx, scale - dx));

                int32_t dist = dx + points[i].distY;
                if (dist < best)
                {
                    best2 = best;
                    best2i = besti;
                    best = dist;
                    besti = i;
                }
                else if (dist > best && dist < best2)
                {
                    best2 = dist;
                    best2i = i;
                }
            }

            // color the pixel accordingly
            float d0 = std::sqrt(best) / scale;

            if ((mode & 1) == CellInner) // inner
                t = std::clamp<int32_t>(d0 * amp, 0, 1 << 24);
            else // outer
            {
                float d1 = std::sqrt(best2) / scale;

                if (d0 + d1 > 0.0f)
                    t = std::clamp<int32_t>(d0 / (d1 + d0) * 2 * amp, 0, 1 << 24);
                else
                    t = 0;
            }

            SampleGradient(grad, *out, t);
            *out *= centers[points[besti].node].color;

            out++;
            xc += stepX;
        }

        yc += stepY;
    }

    delete[] points;
}