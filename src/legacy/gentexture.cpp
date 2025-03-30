/****************************************************************************/
/***                                                                      ***/
/***   Written by Fabian Giesen.                                          ***/
/***   I hereby place this code in the public domain.                     ***/
/***                                                                      ***/
/****************************************************************************/

#include <algorithm>
#include <cassert>

#include <openktg/core/pixel.h>
#include <openktg/legacy/gentexture.h>
#include <openktg/noise/perlin.h>
#include <openktg/util/helpers.h>

/****************************************************************************/
/***                                                                      ***/
/***   GenTexture                                                         ***/
/***                                                                      ***/
/****************************************************************************/

GenTexture::GenTexture()
{
    Data = 0;
    XRes = 0;
    YRes = 0;

    UpdateSize();
}

GenTexture::GenTexture(sInt xres, sInt yres)
{
    Data = 0;
    XRes = 0;
    YRes = 0;

    Init(xres, yres);
}

GenTexture::GenTexture(const GenTexture &x)
{
    XRes = x.XRes;
    YRes = x.YRes;
    UpdateSize();

    Data = new openktg::pixel[NPixels];
    sCopyMem(Data, x.Data, NPixels * sizeof(openktg::pixel));
}

GenTexture::~GenTexture()
{
    delete[] Data;
}

void GenTexture::Init(sInt xres, sInt yres)
{
    if (XRes != xres || YRes != yres)
    {
        delete[] Data;

        assert(IsPowerOf2(xres));
        assert(IsPowerOf2(yres));

        XRes = xres;
        YRes = yres;
        UpdateSize();

        Data = new openktg::pixel[NPixels];
    }
}

void GenTexture::UpdateSize()
{
    NPixels = XRes * YRes;
    ShiftX = FloorLog2(XRes);
    ShiftY = FloorLog2(YRes);

    MinX = 1 << (24 - 1 - ShiftX);
    MinY = 1 << (24 - 1 - ShiftY);
}

void GenTexture::Swap(GenTexture &x)
{
    sSwap(Data, x.Data);
    sSwap(XRes, x.XRes);
    sSwap(YRes, x.YRes);
    sSwap(NPixels, x.NPixels);
    sSwap(ShiftX, x.ShiftX);
    sSwap(ShiftY, x.ShiftY);
    sSwap(MinX, x.MinX);
    sSwap(MinY, x.MinY);
}

GenTexture &GenTexture::operator=(const GenTexture &x)
{
    GenTexture t = x;

    Swap(t);
    return *this;
}

sBool GenTexture::SizeMatchesWith(const GenTexture &x) const
{
    return XRes == x.XRes && YRes == x.YRes;
}

// ---- Sampling helpers
void GenTexture::SampleNearest(openktg::pixel &result, sInt x, sInt y, sInt wrapMode) const
{
    if (wrapMode & 1)
        x = sClamp(x, MinX, 0x1000000 - MinX);
    if (wrapMode & 2)
        y = sClamp(y, MinY, 0x1000000 - MinY);

    x &= 0xffffff;
    y &= 0xffffff;

    sInt ix = x >> (24 - ShiftX);
    sInt iy = y >> (24 - ShiftY);

    result = Data[(iy << ShiftX) + ix];
}

void GenTexture::SampleBilinear(openktg::pixel &result, sInt x, sInt y, sInt wrapMode) const
{
    if (wrapMode & 1)
        x = sClamp(x, MinX, 0x1000000 - MinX);
    if (wrapMode & 2)
        y = sClamp(y, MinY, 0x1000000 - MinY);

    x = (x - MinX) & 0xffffff;
    y = (y - MinY) & 0xffffff;

    sInt x0 = x >> (24 - ShiftX);
    sInt x1 = (x0 + 1) & (XRes - 1);
    sInt y0 = y >> (24 - ShiftY);
    sInt y1 = (y0 + 1) & (YRes - 1);
    sInt fx = sU32(x << (ShiftX + 8)) >> 16;
    sInt fy = sU32(y << (ShiftY + 8)) >> 16;

    openktg::pixel t0 = lerp(Data[(y0 << ShiftX) + x0], Data[(y0 << ShiftX) + x1], fx);
    openktg::core::pixel t1 = lerp(Data[(y1 << ShiftX) + x0], Data[(y1 << ShiftX) + x1], fx);
    result = lerp(t0, t1, fy);
}

void GenTexture::SampleFiltered(openktg::pixel &result, sInt x, sInt y, sInt filterMode) const
{
    if (filterMode & FilterBilinear)
        SampleBilinear(result, x, y, filterMode);
    else
        SampleNearest(result, x, y, filterMode);
}

void GenTexture::SampleGradient(openktg::pixel &result, sInt x) const
{
    x = sClamp(x, 0, 1 << 24);
    x -= x >> ShiftX; // x=(1<<24) -> Take rightmost pixel

    sInt x0 = x >> (24 - ShiftX);
    sInt x1 = (x0 + 1) & (XRes - 1);
    sInt fx = sU32(x << (ShiftX + 8)) >> 16;

    result = lerp(Data[x0], Data[x1], fx);
}

// ---- The operators themselves

void GenTexture::Noise(const GenTexture &grad, sInt freqX, sInt freqY, sInt oct, sF32 fadeoff, sInt seed, sInt mode)
{
    assert(oct > 0);

    seed = PerlinNoise::P(seed);

    sInt offset;
    sF32 scaling;

    if (mode & NoiseNormalize)
        scaling = (fadeoff - 1.0f) / (sFPow(fadeoff, oct) - 1.0f);
    else
        scaling = sMin(1.0f, 1.0f / fadeoff);

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

    sInt offsX = (1 << (16 - ShiftX + freqX)) >> 1;
    sInt offsY = (1 << (16 - ShiftY + freqY)) >> 1;

    openktg::pixel *out = Data;
    for (sInt y = 0; y < YRes; y++)
    {
        for (sInt x = 0; x < XRes; x++)
        {
            sInt n = offset;
            sF32 s = scaling;

            sInt px = (x << (16 - ShiftX + freqX)) + offsX;
            sInt py = (y << (16 - ShiftY + freqY)) + offsY;
            sInt mx = (1 << freqX) - 1;
            sInt my = (1 << freqY) - 1;

            for (sInt i = 0; i < oct; i++)
            {
                sF32 nv = (mode & NoiseBandlimit) ? PerlinNoise::Noise2(px, py, mx, my, seed) : PerlinNoise::GNoise2(px, py, mx, my, seed);
                if (mode & NoiseAbs)
                    nv = sFAbs(nv);

                n += nv * s;
                s *= fadeoff;

                px += px;
                py += py;
                mx += mx + 1;
                my += my + 1;
            }

            grad.SampleGradient(*out, n);
            out++;
        }
    }
}

void GenTexture::GlowRect(const GenTexture &bgTex, const GenTexture &grad, sF32 orgx, sF32 orgy, sF32 ux, sF32 uy, sF32 vx, sF32 vy, sF32 rectu, sF32 rectv)
{
    assert(SizeMatchesWith(bgTex));

    // copy background over (if we're not the background texture already)
    if (this != &bgTex)
        *this = bgTex;

    // calculate bounding rect
    sInt minX = sMax<sInt>(0, floor((orgx - sFAbs(ux) - sFAbs(vx)) * XRes));
    sInt minY = sMax<sInt>(0, floor((orgy - sFAbs(uy) - sFAbs(vy)) * YRes));
    sInt maxX = sMin<sInt>(XRes - 1, ceil((orgx + sFAbs(ux) + sFAbs(vx)) * XRes));
    sInt maxY = sMin<sInt>(YRes - 1, ceil((orgy + sFAbs(uy) + sFAbs(vy)) * YRes));

    // solve for u0,v0 and deltas (cramer's rule)
    sF32 detM = ux * vy - uy * vx;
    if (fabs(detM) * XRes * YRes < 0.25f) // smaller than a pixel? skip it.
        return;

    sF32 invM = (1 << 16) / detM;
    sF32 rmx = (minX + 0.5f) / XRes - orgx;
    sF32 rmy = (minY + 0.5f) / YRes - orgy;
    sInt u0 = (rmx * vy - rmy * vx) * invM;
    sInt v0 = (ux * rmy - uy * rmx) * invM;
    sInt dudx = vy * invM / XRes;
    sInt dvdx = -uy * invM / XRes;
    sInt dudy = -vx * invM / YRes;
    sInt dvdy = ux * invM / YRes;
    sInt ruf = sMin<sInt>(rectu * 65536.0f, 65535);
    sInt rvf = sMin<sInt>(rectv * 65536.0f, 65535);
    sF32 gus = 1.0f / (65536.0f - ruf);
    sF32 gvs = 1.0f / (65536.0f - rvf);

    for (sInt y = minY; y <= maxY; y++)
    {
        openktg::pixel *out = &Data[y * XRes + minX];
        sInt u = u0;
        sInt v = v0;

        for (sInt x = minX; x <= maxX; x++)
        {
            if (u > -65536 && u < 65536 && v > -65536 && v < 65536)
            {
                openktg::pixel col;

                sInt du = sMax(sAbs(u) - ruf, 0);
                sInt dv = sMax(sAbs(v) - rvf, 0);

                if (!du && !dv)
                {
                    grad.SampleGradient(col, 0);
                    *out = compositeROver(*out, col);
                }
                else
                {
                    sF32 dus = du * gus;
                    sF32 dvs = dv * gvs;
                    sF32 dist = dus * dus + dvs * dvs;

                    if (dist < 1.0f)
                    {
                        grad.SampleGradient(col, (1 << 24) * sFSqrt(dist));
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
    sInt x;
    sInt y;
    sInt distY;
    sInt node;
};

void GenTexture::Cells(const GenTexture &grad, const CellCenter *centers, sInt nCenters, sF32 amp, sInt mode)
{
    assert(((mode & 1) == 0) ? nCenters >= 1 : nCenters >= 2);

    openktg::pixel *out = Data;
    CellPoint *points = NULL;

    points = new CellPoint[nCenters];

    // convert cell center coordinates to fixed point
    static const sInt scaleF = 14; // should be <=14 for 32-bit ints.
    static const sInt scale = 1 << scaleF;

    for (sInt i = 0; i < nCenters; i++)
    {
        points[i].x = sInt(centers[i].x * scale + 0.5f) & (scale - 1);
        points[i].y = sInt(centers[i].y * scale + 0.5f) & (scale - 1);
        points[i].distY = -1;
        points[i].node = i;
    }

    sInt stepX = 1 << (scaleF - ShiftX);
    sInt stepY = 1 << (scaleF - ShiftY);
    sInt yc = stepY >> 1;

    amp = amp * (1 << 24);

    for (sInt y = 0; y < YRes; y++)
    {
        sInt xc = stepX >> 1;

        // calculate new y distances
        for (sInt i = 0; i < nCenters; i++)
        {
            sInt dy = (yc - points[i].y) & (scale - 1);
            points[i].distY = sSquare(sMin(dy, scale - dy));
        }

        // (insertion) sort by y-distance
        for (sInt i = 1; i < nCenters; i++)
        {
            CellPoint v = points[i];
            sInt j = i;

            while (j && points[j - 1].distY > v.distY)
            {
                points[j] = points[j - 1];
                j--;
            }

            points[j] = v;
        }

        sInt best, best2;
        sInt besti, best2i;

        best = best2 = sSquare(scale);
        besti = best2i = -1;

        for (sInt x = 0; x < XRes; x++)
        {
            sInt t, dx;

            // update "best point" stats
            if (besti != -1 && best2i != -1)
            {
                dx = (xc - points[besti].x) & (scale - 1);
                best = sSquare(sMin(dx, scale - dx)) + points[besti].distY;

                dx = (xc - points[best2i].x) & (scale - 1);
                best2 = sSquare(sMin(dx, scale - dx)) + points[best2i].distY;
                if (best2 < best)
                {
                    sSwap(best, best2);
                    sSwap(besti, best2i);
                }
            }

            // search for better points
            for (sInt i = 0; i < nCenters && best2 > points[i].distY; i++)
            {
                sInt dx = (xc - points[i].x) & (scale - 1);
                dx = sSquare(sMin(dx, scale - dx));

                sInt dist = dx + points[i].distY;
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
            sF32 d0 = sFSqrt(best) / scale;

            if ((mode & 1) == CellInner) // inner
                t = sClamp<sInt>(d0 * amp, 0, 1 << 24);
            else // outer
            {
                sF32 d1 = sFSqrt(best2) / scale;

                if (d0 + d1 > 0.0f)
                    t = sClamp<sInt>(d0 / (d1 + d0) * 2 * amp, 0, 1 << 24);
                else
                    t = 0;
            }

            grad.SampleGradient(*out, t);
            *out *= centers[points[besti].node].color;

            out++;
            xc += stepX;
        }

        yc += stepY;
    }

    delete[] points;
}

void GenTexture::ColorMatrixTransform(const GenTexture &x, const openktg::matrix44<float> &matrix, sBool clampPremult)
{
    // sInt m[4][4];
    openktg::matrix44<int> m;
    std::transform(matrix.data.begin(), matrix.data.end(), m.data.begin(), [](const auto &fv) { return fv * 65536.0f; });

    assert(SizeMatchesWith(x));

    // for (sInt i = 0; i < 4; i++)
    // {
    //     for (sInt j = 0; j < 4; j++)
    //     {
    //         assert(matrix[i][j] >= -127.0f && matrix[i][j] <= 127.0f);
    //         m[i][j] = matrix[i][j] * 65536.0f;
    //     }
    // }

    std::transform(matrix.data.begin(), matrix.data.end(), m.data.begin(), [](const auto &fv) { return fv * 65536.0f; });

    for (sInt i = 0; i < NPixels; i++)
    {
        openktg::pixel &out = Data[i];
        const openktg::pixel &in = x.Data[i];

        // some kind of pixel matrix multiplication
        sInt r = MulShift16(m.get(0, 0), in.r()) + MulShift16(m.get(0, 1), in.g()) + MulShift16(m.get(0, 2), in.b()) + MulShift16(m.get(0, 3), in.a());
        sInt g = MulShift16(m.get(1, 0), in.r()) + MulShift16(m.get(1, 1), in.g()) + MulShift16(m.get(1, 2), in.b()) + MulShift16(m.get(1, 3), in.a());
        sInt b = MulShift16(m.get(2, 0), in.r()) + MulShift16(m.get(2, 1), in.g()) + MulShift16(m.get(2, 2), in.b()) + MulShift16(m.get(2, 3), in.a());
        sInt a = MulShift16(m.get(3, 0), in.r()) + MulShift16(m.get(3, 1), in.g()) + MulShift16(m.get(3, 2), in.b()) + MulShift16(m.get(3, 3), in.a());

        a = sClamp<sInt>(a, 0, 65535);
        r = sClamp<sInt>(r, 0, 65535);
        g = sClamp<sInt>(g, 0, 65535);
        b = sClamp<sInt>(b, 0, 65535);

        out = openktg::pixel{static_cast<openktg::red16_t>(r), static_cast<openktg::green16_t>(g), static_cast<openktg::blue16_t>(b),
                             static_cast<openktg::alpha16_t>(a)};

        if (clampPremult)
        {
            out.clamp_premult();
        }
    }
}

void GenTexture::CoordMatrixTransform(const GenTexture &in, const openktg::matrix44<float> &matrix, sInt mode)
{
    sInt scaleX = 1 << (24 - ShiftX);
    sInt scaleY = 1 << (24 - ShiftY);

    sInt dudx = matrix.get(0, 0) * scaleX;
    sInt dudy = matrix.get(0, 1) * scaleY;
    sInt dvdx = matrix.get(1, 0) * scaleX;
    sInt dvdy = matrix.get(1, 1) * scaleY;

    sInt u0 = matrix.get(0, 3) * (1 << 24) + ((dudx + dudy) >> 1);
    sInt v0 = matrix.get(1, 3) * (1 << 24) + ((dvdx + dvdy) >> 1);
    openktg::core::pixel *out = Data;

    for (sInt y = 0; y < YRes; y++)
    {
        sInt u = u0;
        sInt v = v0;

        for (sInt x = 0; x < XRes; x++)
        {
            in.SampleFiltered(*out, u, v, mode);

            u += dudx;
            v += dvdx;
            out++;
        }

        u0 += dudy;
        v0 += dvdy;
    }
}

void GenTexture::ColorRemap(const GenTexture &inTex, const GenTexture &mapR, const GenTexture &mapG, const GenTexture &mapB)
{
    assert(SizeMatchesWith(inTex));

    for (sInt i = 0; i < NPixels; i++)
    {
        const openktg::core::pixel &in = inTex.Data[i];
        openktg::core::pixel &out = Data[i];

        if (in.a() == 65535) // alpha==1, everything easy.
        {
            openktg::core::pixel colR, colG, colB;

            mapR.SampleGradient(colR, (in.r() << 8) + ((in.r() + 128) >> 8));
            mapG.SampleGradient(colG, (in.g() << 8) + ((in.g() + 128) >> 8));
            mapB.SampleGradient(colB, (in.b() << 8) + ((in.b() + 128) >> 8));

            out = openktg::core::pixel(static_cast<openktg::red16_t>(sMin(colR.r() + colG.r() + colB.r(), 65535)),
                                       static_cast<openktg::green16_t>(sMin(colR.g() + colG.g() + colB.g(), 65535)),
                                       static_cast<openktg::blue16_t>(sMin(colR.b() + colG.b() + colB.b(), 65535)), static_cast<openktg::alpha16_t>(in.a()));
        }
        else if (in.a()) // alpha!=0
        {
            openktg::core::pixel colR, colG, colB;
            sU32 invA = (65535U << 16) / in.a();

            mapR.SampleGradient(colR, UMulShift8(sMin(in.r(), in.a()), invA));
            mapG.SampleGradient(colG, UMulShift8(sMin(in.g(), in.a()), invA));
            mapB.SampleGradient(colB, UMulShift8(sMin(in.b(), in.a()), invA));

            out = openktg::core::pixel(static_cast<openktg::red16_t>(MulIntens(sMin(colR.r() + colG.r() + colB.r(), 65535), in.a())),
                                       static_cast<openktg::green16_t>(MulIntens(sMin(colR.g() + colG.g() + colB.g(), 65535), in.a())),
                                       static_cast<openktg::blue16_t>(MulIntens(sMin(colR.b() + colG.b() + colB.b(), 65535), in.a())),
                                       static_cast<openktg::alpha16_t>(in.a()));
        }
        else // alpha==0
            out = in;
    }
}

void GenTexture::CoordRemap(const GenTexture &in, const GenTexture &remapTex, sF32 strengthU, sF32 strengthV, sInt mode)
{
    assert(SizeMatchesWith(remapTex));

    const openktg::core::pixel *remap = remapTex.Data;
    openktg::core::pixel *out = Data;

    sInt u0 = MinX;
    sInt v0 = MinY;
    sInt scaleU = (1 << 24) * strengthU;
    sInt scaleV = (1 << 24) * strengthV;
    sInt stepU = 1 << (24 - ShiftX);
    sInt stepV = 1 << (24 - ShiftY);

    for (sInt y = 0; y < YRes; y++)
    {
        sInt u = u0;
        sInt v = v0;

        for (sInt x = 0; x < XRes; x++)
        {
            sInt dispU = u + MulShift16(scaleU, (remap->r() - 32768) * 2);
            sInt dispV = v + MulShift16(scaleV, (remap->g() - 32768) * 2);
            in.SampleFiltered(*out, dispU, dispV, mode);

            u += stepU;
            remap++;
            out++;
        }

        v0 += stepV;
    }
}

void GenTexture::Derive(const GenTexture &in, DeriveOp op, sF32 strength)
{
    assert(SizeMatchesWith(in));

    openktg::core::pixel *out = Data;

    for (sInt y = 0; y < YRes; y++)
    {
        for (sInt x = 0; x < XRes; x++)
        {
            sInt dx2 = in.Data[y * XRes + ((x + 1) & (XRes - 1))].r() - in.Data[y * XRes + ((x - 1) & (XRes - 1))].r();
            sInt dy2 = in.Data[x + ((y + 1) & (YRes - 1)) * XRes].r() - in.Data[x + ((y - 1) & (YRes - 1)) * XRes].r();
            sF32 dx = dx2 * strength / (2 * 65535.0f);
            sF32 dy = dy2 * strength / (2 * 65535.0f);

            switch (op)
            {
            case DeriveGradient: {
                *out = openktg::core::pixel{static_cast<openktg::red16_t>(sClamp<sInt>(dx * 32768.0f + 32768.0f, 0, 65535)),
                                            static_cast<openktg::green16_t>(sClamp<sInt>(dy * 32768.0f + 32768.0f, 0, 65535)),
                                            static_cast<openktg::blue16_t>(0), static_cast<openktg::alpha16_t>(65535)};
                break;
            }
            case DeriveNormals: {
                // (1 0 dx)^T x (0 1 dy)^T = (-dx -dy 1)
                sF32 scale = 32768.0f * sFInvSqrt(1.0f + dx * dx + dy * dy);

                *out = openktg::core::pixel{static_cast<openktg::red16_t>(sClamp<sInt>(-dx * scale + 32768.0f, 0, 65535)),
                                            static_cast<openktg::green16_t>(sClamp<sInt>(-dy * scale + 32768.0f, 0, 65535)),
                                            static_cast<openktg::blue16_t>(sClamp<sInt>(scale + 32768.0f, 0, 65535)), static_cast<openktg::alpha16_t>(65535)};
                break;
            }
            }
            out++;
        }
    }
}

// Wrap computation on pixel coordinates
static sInt WrapCoord(sInt x, sInt width, sInt mode)
{
    if (mode == 0) // wrap
        return x & (width - 1);
    else
        return sClamp(x, 0, width - 1);
}

// Size is half of edge length in pixels, 26.6 fixed point
static void Blur1DBuffer(openktg::core::pixel *dst, const openktg::core::pixel *src, sInt width, sInt sizeFixed, sInt wrapMode)
{
    assert(sizeFixed > 32); // kernel should be wider than one pixel
    sInt frac = (sizeFixed - 32) & 63;
    sInt offset = (sizeFixed + 32) >> 6;

    assert(((offset - 1) * 64 + frac + 32) == sizeFixed);
    sU32 denom = sizeFixed * 2;
    sU32 bias = denom / 2;

    // initialize accumulators
    sU32 accu[4];
    if (wrapMode == 0) // wrap around
    {
        // leftmost and rightmost pixels (the partially covered ones)
        sInt xl = WrapCoord(-offset, width, wrapMode);
        sInt xr = WrapCoord(offset, width, wrapMode);
        accu[0] = frac * (src[xl].r() + src[xr].r()) + bias;
        accu[1] = frac * (src[xl].g() + src[xr].g()) + bias;
        accu[2] = frac * (src[xl].b() + src[xr].b()) + bias;
        accu[3] = frac * (src[xl].a() + src[xr].a()) + bias;

        // inner part of filter kernel
        for (sInt x = -offset + 1; x <= offset - 1; x++)
        {
            sInt xc = WrapCoord(x, width, wrapMode);

            accu[0] += src[xc].r() << 6;
            accu[1] += src[xc].g() << 6;
            accu[2] += src[xc].b() << 6;
            accu[3] += src[xc].a() << 6;
        }
    }
    else // clamp on edge
    {
        // on the left edge, the first pixel is repeated over and over
        accu[0] = src[0].r() * (sizeFixed + 32) + bias;
        accu[1] = src[0].g() * (sizeFixed + 32) + bias;
        accu[2] = src[0].b() * (sizeFixed + 32) + bias;
        accu[3] = src[0].a() * (sizeFixed + 32) + bias;

        // rightmost pixel
        sInt xr = WrapCoord(offset, width, wrapMode);
        accu[0] += frac * src[xr].r();
        accu[1] += frac * src[xr].g();
        accu[2] += frac * src[xr].b();
        accu[3] += frac * src[xr].a();

        // inner part of filter kernel (the right half)
        for (sInt x = 1; x <= offset - 1; x++)
        {
            sInt xc = WrapCoord(x, width, wrapMode);

            accu[0] += src[xc].r() << 6;
            accu[1] += src[xc].g() << 6;
            accu[2] += src[xc].b() << 6;
            accu[3] += src[xc].a() << 6;
        }
    }

    // generate output pixels
    for (sInt x = 0; x < width; x++)
    {
        // write out state of accumulator
        dst[x] = openktg::core::pixel(static_cast<openktg::red16_t>(accu[0] / denom), static_cast<openktg::green16_t>(accu[1] / denom),
                                      static_cast<openktg::blue16_t>(accu[2] / denom), static_cast<openktg::alpha16_t>(accu[3] / denom));

        // update accumulator
        sInt xl0 = WrapCoord(x - offset + 0, width, wrapMode);
        sInt xl1 = WrapCoord(x - offset + 1, width, wrapMode);
        sInt xr0 = WrapCoord(x + offset + 0, width, wrapMode);
        sInt xr1 = WrapCoord(x + offset + 1, width, wrapMode);

        accu[0] += 64 * (src[xr0].r() - src[xl1].r()) + frac * (src[xr1].r() - src[xr0].r() - src[xl0].r() + src[xl1].r());
        accu[1] += 64 * (src[xr0].g() - src[xl1].g()) + frac * (src[xr1].g() - src[xr0].g() - src[xl0].g() + src[xl1].g());
        accu[2] += 64 * (src[xr0].b() - src[xl1].b()) + frac * (src[xr1].b() - src[xr0].b() - src[xl0].b() + src[xl1].b());
        accu[3] += 64 * (src[xr0].a() - src[xl1].a()) + frac * (src[xr1].a() - src[xr0].a() - src[xl0].a() + src[xl1].a());
    }
}

void GenTexture::Blur(const GenTexture &inImg, sF32 sizex, sF32 sizey, sInt order, sInt wrapMode)
{
    assert(SizeMatchesWith(inImg));

    sInt sizePixX = sClamp(sizex, 0.0f, 1.0f) * 64 * inImg.XRes / 2;
    sInt sizePixY = sClamp(sizey, 0.0f, 1.0f) * 64 * inImg.YRes / 2;

    // no blur at all? just copy!
    if (order < 1 || (sizePixX <= 32 && sizePixY <= 32))
        *this = inImg;
    else
    {
        // allocate pixel buffers
        sInt bufSize = sMax(XRes, YRes);
        openktg::core::pixel *buf1 = new openktg::core::pixel[bufSize];
        openktg::core::pixel *buf2 = new openktg::core::pixel[bufSize];
        const GenTexture *in = &inImg;

        // horizontal blur
        if (sizePixX > 32)
        {
            // go through image row by row
            for (sInt y = 0; y < YRes; y++)
            {
                // copy pixels into buffer 1
                sCopyMem(buf1, &in->Data[y * XRes], XRes * sizeof(openktg::core::pixel));

                // blur order times, ping-ponging between buffers
                for (sInt i = 0; i < order; i++)
                {
                    Blur1DBuffer(buf2, buf1, XRes, sizePixX, (wrapMode & ClampU) ? 1 : 0);
                    sSwap(buf1, buf2);
                }

                // copy pixels back
                sCopyMem(&Data[y * XRes], buf1, XRes * sizeof(openktg::core::pixel));
            }

            in = this;
        }

        // vertical blur
        if (sizePixY > 32)
        {
            // go through image column by column
            for (sInt x = 0; x < XRes; x++)
            {
                // copy pixels into buffer 1
                const openktg::core::pixel *src = &in->Data[x];
                openktg::core::pixel *dst = buf1;

                for (sInt y = 0; y < YRes; y++)
                {
                    *dst++ = *src;
                    src += XRes;
                }

                // blur order times, ping-ponging between buffers
                for (sInt i = 0; i < order; i++)
                {
                    Blur1DBuffer(buf2, buf1, YRes, sizePixY, (wrapMode & ClampV) ? 1 : 0);
                    sSwap(buf1, buf2);
                }

                // copy pixels back
                src = buf1;
                dst = &Data[x];

                for (sInt y = 0; y < YRes; y++)
                {
                    *dst = *src++;
                    dst += XRes;
                }
            }
        }

        // clean up
        delete[] buf1;
        delete[] buf2;
    }
}

void GenTexture::Ternary(const GenTexture &in1Tex, const GenTexture &in2Tex, const GenTexture &in3Tex, TernaryOp op)
{
    assert(SizeMatchesWith(in1Tex) && SizeMatchesWith(in2Tex) && SizeMatchesWith(in3Tex));

    for (sInt i = 0; i < NPixels; i++)
    {
        openktg::core::pixel &out = Data[i];
        const openktg::core::pixel &in1 = in1Tex.Data[i];
        const openktg::core::pixel &in2 = in2Tex.Data[i];
        const openktg::core::pixel &in3 = in3Tex.Data[i];

        switch (op)
        {
        case TernaryLerp:
            out = (~in3.r() * in1) + (in3.r() * in2);
            break;

        case TernarySelect:
            out = (in3.r() >= 32768) ? in2 : in1;
            break;
        }
    }
}

void GenTexture::Paste(const GenTexture &bgTex, const GenTexture &inTex, sF32 orgx, sF32 orgy, sF32 ux, sF32 uy, sF32 vx, sF32 vy, CombineOp op, sInt mode)
{
    assert(SizeMatchesWith(bgTex));

    // copy background over (if this image is not the background already)
    if (this != &bgTex)
        *this = bgTex;

    // calculate bounding rect
    sInt minX = sMax<sInt>(0, floor((orgx + sMin(ux, 0.0f) + sMin(vx, 0.0f)) * XRes));
    sInt minY = sMax<sInt>(0, floor((orgy + sMin(uy, 0.0f) + sMin(vy, 0.0f)) * YRes));
    sInt maxX = sMin<sInt>(XRes - 1, ceil((orgx + sMax(ux, 0.0f) + sMax(vx, 0.0f)) * XRes));
    sInt maxY = sMin<sInt>(YRes - 1, ceil((orgy + sMax(uy, 0.0f) + sMax(vy, 0.0f)) * YRes));

    // solve for u0,v0 and deltas (Cramer's rule)
    sF32 detM = ux * vy - uy * vx;
    if (fabs(detM) * XRes * YRes < 0.25f) // smaller than a pixel? skip it.
        return;

    sF32 invM = (1 << 24) / detM;
    sF32 rmx = (minX + 0.5f) / XRes - orgx;
    sF32 rmy = (minY + 0.5f) / YRes - orgy;
    sInt u0 = (rmx * vy - rmy * vx) * invM;
    sInt v0 = (ux * rmy - uy * rmx) * invM;
    sInt dudx = vy * invM / XRes;
    sInt dvdx = -uy * invM / XRes;
    sInt dudy = -vx * invM / YRes;
    sInt dvdy = ux * invM / YRes;

    for (sInt y = minY; y <= maxY; y++)
    {
        openktg::core::pixel *out = &Data[y * XRes + minX];
        sInt u = u0;
        sInt v = v0;

        for (sInt x = minX; x <= maxX; x++)
        {
            if (u >= 0 && u < 0x1000000 && v >= 0 && v < 0x1000000)
            {
                openktg::core::pixel in;
                sInt transIn, transOut;

                inTex.SampleFiltered(in, u, v, ClampU | ClampV | ((mode & 1) ? FilterBilinear : FilterNearest));

                switch (op)
                {
                case CombineAdd: {
                    *out += in;
                    break;
                }

                case CombineSub: {
                    *out -= in;
                    break;
                }

                case CombineMulC: {
                    *out *= in;
                    break;
                }

                case CombineMin: {
                    *out &= in;
                    break;
                }

                case CombineMax: {
                    *out |= in;
                    break;
                }

                case CombineSetAlpha: {
                    out->set_alpha(static_cast<openktg::alpha16_t>(in.r()));
                    break;
                }

                case CombinePreAlpha: {
                    *out = *out * in.r();
                    out->set_alpha(static_cast<openktg::alpha16_t>(in.g()));
                    break;
                }

                case CombineOver: {
                    *out = openktg::combineOver(in, *out);
                    break;
                }

                case CombineMultiply: {
                    *out = openktg::combineMultiply(in, *out);
                    break;
                }

                case CombineScreen: {
                    *out = openktg::combineScreen(in, *out);
                    break;
                }

                case CombineDarken: {
                    *out = openktg::combineDarken(in, *out);
                    break;
                }

                case CombineLighten: {
                    *out = openktg::combineLighten(in, *out);
                    break;
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

void GenTexture::Bump(const GenTexture &surface, const GenTexture &normals, const GenTexture *specular, const GenTexture *falloffMap, sF32 px, sF32 py, sF32 pz,
                      sF32 dx, sF32 dy, sF32 dz, const openktg::core::pixel &ambient, const openktg::core::pixel &diffuse, sBool directional)
{
    assert(SizeMatchesWith(surface) && SizeMatchesWith(normals));

    sF32 L[3], H[3]; // light/halfway vector
    sF32 invX, invY;

    sF32 scale = sFInvSqrt(dx * dx + dy * dy + dz * dz);
    dx *= scale;
    dy *= scale;
    dz *= scale;

    if (directional)
    {
        L[0] = -dx;
        L[1] = -dy;
        L[2] = -dz;

        scale = sFInvSqrt(2.0f + 2.0f * L[2]); // 1/sqrt((L + <0,0,1>)^2)
        H[0] = L[0] * scale;
        H[1] = L[1] * scale;
        H[2] = (L[2] + 1.0f) * scale;
    }

    invX = 1.0f / XRes;
    invY = 1.0f / YRes;
    openktg::core::pixel *out = Data;
    const openktg::core::pixel *surf = surface.Data;
    const openktg::core::pixel *normal = normals.Data;

    for (sInt y = 0; y < YRes; y++)
    {
        for (sInt x = 0; x < XRes; x++)
        {
            // determine vectors to light
            if (!directional)
            {
                L[0] = px - (x + 0.5f) * invX;
                L[1] = py - (y + 0.5f) * invY;
                L[2] = pz;

                sF32 scale = sFInvSqrt(L[0] * L[0] + L[1] * L[1] + L[2] * L[2]);
                L[0] *= scale;
                L[1] *= scale;
                L[2] *= scale;

                // determine halfway vector
                if (specular)
                {
                    sF32 scale = sFInvSqrt(2.0f + 2.0f * L[2]); // 1/sqrt((L + <0,0,1>)^2)
                    H[0] = L[0] * scale;
                    H[1] = L[1] * scale;
                    H[2] = (L[2] + 1.0f) * scale;
                }
            }

            // fetch normal
            sF32 N[3];
            N[0] = (normal->r() - 0x8000) / 32768.0f;
            N[1] = (normal->g() - 0x8000) / 32768.0f;
            N[2] = (normal->b() - 0x8000) / 32768.0f;

            // get falloff term if specified
            openktg::core::pixel falloff;
            if (falloffMap)
            {
                sF32 spotTerm = sMax<sF32>(dx * L[0] + dy * L[1] + dz * L[2], 0.0f);
                falloffMap->SampleGradient(falloff, spotTerm * (1 << 24));
            }

            // lighting calculation
            sF32 NdotL = sMax<sF32>(N[0] * L[0] + N[1] * L[1] + N[2] * L[2], 0.0f);
            openktg::core::pixel ambDiffuse =
                openktg::core::pixel{static_cast<openktg::red16_t>(NdotL * diffuse.r()), static_cast<openktg::green16_t>(NdotL * diffuse.g()),
                                     static_cast<openktg::blue16_t>(NdotL * diffuse.b()), static_cast<openktg::alpha16_t>(NdotL * diffuse.a())};
            if (falloffMap)
            {
                ambDiffuse = openktg::compositeMulC(ambDiffuse, falloff);
            }

            ambDiffuse = openktg::compositeAdd(ambDiffuse, ambient);
            *out = *surf * ambDiffuse;

            if (specular)
            {
                openktg::core::pixel addTerm;
                sF32 NdotH = sMax<sF32>(N[0] * H[0] + N[1] * H[1] + N[2] * H[2], 0.0f);
                specular->SampleGradient(addTerm, NdotH * (1 << 24));
                if (falloffMap)
                {
                    addTerm = openktg::compositeMulC(addTerm, falloff);
                }

                auto new_alpha = out->a();
                *out += addTerm;
                out->set_alpha(static_cast<openktg::alpha16_t>(new_alpha));
                out->clamp_premult();
            }

            out++;
            surf++;
            normal++;
        }
    }
}

void GenTexture::LinearCombine(const openktg::core::pixel &color, sF32 constWeight, const LinearInput *inputs, sInt nInputs)
{
    sInt w[256], uo[256], vo[256];

    assert(nInputs <= 255);
    assert(constWeight >= -127.0f && constWeight <= 127.0f);

    // convert weights and offsets to fixed point
    for (sInt i = 0; i < nInputs; i++)
    {
        assert(inputs[i].Weight >= -127.0f && inputs[i].Weight <= 127.0f);
        assert(inputs[i].UShift >= -127.0f && inputs[i].UShift <= 127.0f);
        assert(inputs[i].VShift >= -127.0f && inputs[i].VShift <= 127.0f);

        w[i] = inputs[i].Weight * 65536.0f;
        uo[i] = inputs[i].UShift * (1 << 24);
        vo[i] = inputs[i].VShift * (1 << 24);
    }

    // compute preweighted constant color
    sInt c_r, c_g, c_b, c_a, t;

    t = constWeight * 65536.0f;
    c_r = MulShift16(t, color.r());
    c_g = MulShift16(t, color.g());
    c_b = MulShift16(t, color.b());
    c_a = MulShift16(t, color.a());

    // calculate output image
    sInt u0 = MinX;
    sInt v0 = MinY;
    sInt stepU = 1 << (24 - ShiftX);
    sInt stepV = 1 << (24 - ShiftY);
    openktg::core::pixel *out = Data;

    for (sInt y = 0; y < YRes; y++)
    {
        sInt u = u0;
        sInt v = v0;

        for (sInt x = 0; x < XRes; x++)
        {
            sInt acc_r, acc_g, acc_b, acc_a;

            // initialize accumulator with start value
            acc_r = c_r;
            acc_g = c_g;
            acc_b = c_b;
            acc_a = c_a;

            // accumulate inputs
            for (sInt j = 0; j < nInputs; j++)
            {
                const LinearInput &in = inputs[j];
                openktg::core::pixel inPix;

                in.Tex->SampleFiltered(inPix, u + uo[j], v + vo[j], in.FilterMode);

                acc_r += MulShift16(w[j], inPix.r());
                acc_g += MulShift16(w[j], inPix.g());
                acc_b += MulShift16(w[j], inPix.b());
                acc_a += MulShift16(w[j], inPix.a());
            }

            // store (with clamping)
            *out = openktg::core::pixel{
                static_cast<openktg::red16_t>(sClamp(acc_r, 0, 65535)),
                static_cast<openktg::green16_t>(sClamp(acc_g, 0, 65535)),
                static_cast<openktg::blue16_t>(sClamp(acc_b, 0, 65535)),
                static_cast<openktg::alpha16_t>(sClamp(acc_a, 0, 65535)),
            };

            // advance to next pixel
            u += stepU;
            out++;
        }

        v0 += stepV;
    }
}

void InitTexgen()
{
    // PerlinNoise::initializeTable();
}
