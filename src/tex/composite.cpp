#include <cassert>

#include <openktg/core/pixel.h>
#include <openktg/core/texture.h>
#include <openktg/tex/composite.h>
#include <openktg/tex/sampling.h>
#include <openktg/util/utility.h>

void Ternary(openktg::texture &input, const openktg::texture &in1Tex, const openktg::texture &in2Tex, const openktg::texture &in3Tex, TernaryOp op)
{
    assert(texture_size_matches(input, in1Tex));
    assert(texture_size_matches(input, in2Tex));
    assert(texture_size_matches(input, in3Tex));

    for (int32_t i = 0; i < input.pixel_count(); i++)
    {
        openktg::core::pixel &out = input.data()[i];
        const openktg::core::pixel &in1 = in1Tex.data()[i];
        const openktg::core::pixel &in2 = in2Tex.data()[i];
        const openktg::core::pixel &in3 = in3Tex.data()[i];

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

void Paste(openktg::texture &input, const openktg::texture &bgTex, const openktg::texture &inTex, float orgx, float orgy, float ux, float uy, float vx,
           float vy, CombineOp op, int32_t mode)
{
    assert(texture_size_matches(input, bgTex));

    // copy background over (if this image is not the background already)
    if (&input != &bgTex)
        input = bgTex;

    // calculate bounding rect
    int32_t minX = std::max<int32_t>(0, floor((orgx + std::min(ux, 0.0f) + std::min(vx, 0.0f)) * input.width()));
    int32_t minY = std::max<int32_t>(0, floor((orgy + std::min(uy, 0.0f) + std::min(vy, 0.0f)) * input.height()));
    int32_t maxX = std::min<int32_t>(input.width() - 1, ceil((orgx + std::max(ux, 0.0f) + std::max(vx, 0.0f)) * input.width()));
    int32_t maxY = std::min<int32_t>(input.height() - 1, ceil((orgy + std::max(uy, 0.0f) + std::max(vy, 0.0f)) * input.height()));

    // solve for u0,v0 and deltas (Cramer's rule)
    float detM = ux * vy - uy * vx;
    if (fabs(detM) * input.width() * input.height() < 0.25f) // smaller than a pixel? skip it.
        return;

    float invM = (1 << 24) / detM;
    float rmx = (minX + 0.5f) / input.width() - orgx;
    float rmy = (minY + 0.5f) / input.height() - orgy;
    int32_t u0 = (rmx * vy - rmy * vx) * invM;
    int32_t v0 = (ux * rmy - uy * rmx) * invM;
    int32_t dudx = vy * invM / input.width();
    int32_t dvdx = -uy * invM / input.width();
    int32_t dudy = -vx * invM / input.height();
    int32_t dvdy = ux * invM / input.height();

    for (int32_t y = minY; y <= maxY; y++)
    {
        openktg::core::pixel *out = &input.at(minX, y);
        int32_t u = u0;
        int32_t v = v0;

        for (int32_t x = minX; x <= maxX; x++)
        {
            if (u >= 0 && u < 0x1000000 && v >= 0 && v < 0x1000000)
            {
                openktg::core::pixel in;
                int32_t transIn, transOut;

                SampleFiltered(inTex, in, u, v, ClampU | ClampV | ((mode & 1) ? FilterBilinear : FilterNearest));

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

void Bump(openktg::texture &input, const openktg::texture &surface, const openktg::texture &normals, const openktg::texture *specular,
          const openktg::texture *falloffMap, float px, float py, float pz, float dx, float dy, float dz, const openktg::core::pixel &ambient,
          const openktg::core::pixel &diffuse, bool directional)
{
    assert(texture_size_matches(input, normals));
    assert(texture_size_matches(input, surface));

    float L[3], H[3]; // light/halfway vector
    float invX, invY;

    float scale = openktg::util::rsqrt(dx * dx + dy * dy + dz * dz);
    dx *= scale;
    dy *= scale;
    dz *= scale;

    if (directional)
    {
        L[0] = -dx;
        L[1] = -dy;
        L[2] = -dz;

        scale = openktg::util::rsqrt(2.0f + 2.0f * L[2]); // 1/sqrt((L + <0,0,1>)^2)
        H[0] = L[0] * scale;
        H[1] = L[1] * scale;
        H[2] = (L[2] + 1.0f) * scale;
    }

    invX = 1.0f / input.width();
    invY = 1.0f / input.height();
    openktg::core::pixel *out = input.data();
    const openktg::core::pixel *surf = surface.data();
    const openktg::core::pixel *normal = normals.data();

    for (int32_t y = 0; y < input.height(); y++)
    {
        for (int32_t x = 0; x < input.width(); x++)
        {
            // determine vectors to light
            if (!directional)
            {
                L[0] = px - (x + 0.5f) * invX;
                L[1] = py - (y + 0.5f) * invY;
                L[2] = pz;

                float scale = openktg::util::rsqrt(L[0] * L[0] + L[1] * L[1] + L[2] * L[2]);
                L[0] *= scale;
                L[1] *= scale;
                L[2] *= scale;

                // determine halfway vector
                if (specular)
                {
                    float scale = openktg::util::rsqrt(2.0f + 2.0f * L[2]); // 1/sqrt((L + <0,0,1>)^2)
                    H[0] = L[0] * scale;
                    H[1] = L[1] * scale;
                    H[2] = (L[2] + 1.0f) * scale;
                }
            }

            // fetch normal
            float N[3];
            N[0] = (normal->r() - 0x8000) / 32768.0f;
            N[1] = (normal->g() - 0x8000) / 32768.0f;
            N[2] = (normal->b() - 0x8000) / 32768.0f;

            // get falloff term if specified
            openktg::core::pixel falloff;
            if (falloffMap)
            {
                float spotTerm = std::max<float>(dx * L[0] + dy * L[1] + dz * L[2], 0.0f);
                SampleGradient(*falloffMap, falloff, spotTerm * (1 << 24));
            }

            // lighting calculation
            float NdotL = std::max<float>(N[0] * L[0] + N[1] * L[1] + N[2] * L[2], 0.0f);
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
                float NdotH = std::max<float>(N[0] * H[0] + N[1] * H[1] + N[2] * H[2], 0.0f);
                SampleGradient(*specular, addTerm, NdotH * (1 << 24));
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

void LinearCombine(openktg::texture &input, const openktg::core::pixel &color, float constWeight, const LinearInput *inputs, int32_t nInputs)
{
    int32_t w[256], uo[256], vo[256];

    assert(nInputs <= 255);
    assert(constWeight >= -127.0f && constWeight <= 127.0f);

    // convert weights and offsets to fixed point
    for (int32_t i = 0; i < nInputs; i++)
    {
        assert(inputs[i].Weight >= -127.0f && inputs[i].Weight <= 127.0f);
        assert(inputs[i].UShift >= -127.0f && inputs[i].UShift <= 127.0f);
        assert(inputs[i].VShift >= -127.0f && inputs[i].VShift <= 127.0f);

        w[i] = inputs[i].Weight * 65536.0f;
        uo[i] = inputs[i].UShift * (1 << 24);
        vo[i] = inputs[i].VShift * (1 << 24);
    }

    // compute preweighted constant color
    int32_t c_r, c_g, c_b, c_a, t;

    t = constWeight * 65536.0f;
    c_r = openktg::util::mul_shift_16(t, color.r());
    c_g = openktg::util::mul_shift_16(t, color.g());
    c_b = openktg::util::mul_shift_16(t, color.b());
    c_a = openktg::util::mul_shift_16(t, color.a());

    // calculate output image
    int32_t u0 = input.min_x();
    int32_t v0 = input.min_y();
    int32_t stepU = 1 << (24 - input.shift_x());
    int32_t stepV = 1 << (24 - input.shift_y());
    openktg::core::pixel *out = input.data();

    for (int32_t y = 0; y < input.height(); y++)
    {
        int32_t u = u0;
        int32_t v = v0;

        for (int32_t x = 0; x < input.width(); x++)
        {
            int32_t acc_r, acc_g, acc_b, acc_a;

            // initialize accumulator with start value
            acc_r = c_r;
            acc_g = c_g;
            acc_b = c_b;
            acc_a = c_a;

            // accumulate inputs
            for (int32_t j = 0; j < nInputs; j++)
            {
                const LinearInput &in = inputs[j];
                openktg::core::pixel inPix;

                SampleFiltered(*in.Tex, inPix, u + uo[j], v + vo[j], in.FilterMode);

                acc_r += openktg::util::mul_shift_16(w[j], inPix.r());
                acc_g += openktg::util::mul_shift_16(w[j], inPix.g());
                acc_b += openktg::util::mul_shift_16(w[j], inPix.b());
                acc_a += openktg::util::mul_shift_16(w[j], inPix.a());
            }

            // store (with clamping)
            *out = openktg::core::pixel{
                static_cast<openktg::red16_t>(std::clamp(acc_r, 0, 65535)),
                static_cast<openktg::green16_t>(std::clamp(acc_g, 0, 65535)),
                static_cast<openktg::blue16_t>(std::clamp(acc_b, 0, 65535)),
                static_cast<openktg::alpha16_t>(std::clamp(acc_a, 0, 65535)),
            };

            // advance to next pixel
            u += stepU;
            out++;
        }

        v0 += stepV;
    }
}