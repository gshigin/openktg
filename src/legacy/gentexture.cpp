/****************************************************************************/
/***                                                                      ***/
/***   Written by Fabian Giesen.                                          ***/
/***   I hereby place this code in the public domain.                     ***/
/***                                                                      ***/
/****************************************************************************/

#include <algorithm>
#include <cassert>

#include <openktg/core/matrix.h>
#include <openktg/core/pixel.h>
#include <openktg/core/texture.h>
#include <openktg/legacy/gentexture.h>
#include <openktg/noise/perlin.h>
#include <openktg/tex/sampling.h>
#include <openktg/util/helpers.h>

auto SizeMatchesWith(const openktg::texture &x, const openktg::texture &y) -> sBool
{
    return y.width() == x.width() && y.height() == x.height();
}

// ---- The operators themselves

void ColorMatrixTransform(openktg::texture &input, const openktg::texture &x, const openktg::matrix44<float> &matrix, sBool clampPremult)
{
    // sInt m[4][4];
    openktg::matrix44<int> m;
    std::transform(matrix.data.begin(), matrix.data.end(), m.data.begin(), [](const auto &fv) { return fv * 65536.0f; });

    assert(SizeMatchesWith(x));

    std::transform(matrix.data.begin(), matrix.data.end(), m.data.begin(), [](const auto &fv) { return fv * 65536.0f; });

    for (sInt i = 0; i < input.pixel_count(); i++)
    {
        openktg::pixel &out = input.data()[i];
        const openktg::pixel &in = x.data()[i];

        // some kind of pixel matrix multiplication
        sInt r = MulShift16(m(0, 0), in.r()) + MulShift16(m(0, 1), in.g()) + MulShift16(m(0, 2), in.b()) + MulShift16(m(0, 3), in.a());
        sInt g = MulShift16(m(1, 0), in.r()) + MulShift16(m(1, 1), in.g()) + MulShift16(m(1, 2), in.b()) + MulShift16(m(1, 3), in.a());
        sInt b = MulShift16(m(2, 0), in.r()) + MulShift16(m(2, 1), in.g()) + MulShift16(m(2, 2), in.b()) + MulShift16(m(2, 3), in.a());
        sInt a = MulShift16(m(3, 0), in.r()) + MulShift16(m(3, 1), in.g()) + MulShift16(m(3, 2), in.b()) + MulShift16(m(3, 3), in.a());

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

void CoordMatrixTransform(openktg::texture &input, const openktg::texture &in, const openktg::matrix44<float> &matrix, sInt mode)
{
    sInt scaleX = 1 << (24 - input.shift_x());
    sInt scaleY = 1 << (24 - input.shift_y());

    sInt dudx = matrix(0, 0) * scaleX;
    sInt dudy = matrix(0, 1) * scaleY;
    sInt dvdx = matrix(1, 0) * scaleX;
    sInt dvdy = matrix(1, 1) * scaleY;

    sInt u0 = matrix(0, 3) * (1 << 24) + ((dudx + dudy) >> 1);
    sInt v0 = matrix(1, 3) * (1 << 24) + ((dvdx + dvdy) >> 1);
    openktg::core::pixel *out = input.data();

    for (sInt y = 0; y < input.height(); y++)
    {
        sInt u = u0;
        sInt v = v0;

        for (sInt x = 0; x < input.width(); x++)
        {
            SampleFiltered(in, *out, u, v, mode);

            u += dudx;
            v += dvdx;
            out++;
        }

        u0 += dudy;
        v0 += dvdy;
    }
}

void ColorRemap(openktg::texture &input, const openktg::texture &inTex, const openktg::texture &mapR, const openktg::texture &mapG,
                const openktg::texture &mapB)
{
    assert(SizeMatchesWith(inTex));

    for (sInt i = 0; i < input.pixel_count(); i++)
    {
        const openktg::core::pixel &in = inTex.data()[i];
        openktg::core::pixel &out = input.data()[i];

        if (in.a() == 65535) // alpha==1, everything easy.
        {
            openktg::core::pixel colR, colG, colB;

            SampleGradient(mapR, colR, (in.r() << 8) + ((in.r() + 128) >> 8));
            SampleGradient(mapG, colG, (in.g() << 8) + ((in.g() + 128) >> 8));
            SampleGradient(mapB, colB, (in.b() << 8) + ((in.b() + 128) >> 8));

            out = openktg::core::pixel(static_cast<openktg::red16_t>(sMin(colR.r() + colG.r() + colB.r(), 65535)),
                                       static_cast<openktg::green16_t>(sMin(colR.g() + colG.g() + colB.g(), 65535)),
                                       static_cast<openktg::blue16_t>(sMin(colR.b() + colG.b() + colB.b(), 65535)), static_cast<openktg::alpha16_t>(in.a()));
        }
        else if (in.a()) // alpha!=0
        {
            openktg::core::pixel colR, colG, colB;
            sU32 invA = (65535U << 16) / in.a();

            SampleGradient(mapR, colR, UMulShift8(sMin(in.r(), in.a()), invA));
            SampleGradient(mapG, colG, UMulShift8(sMin(in.g(), in.a()), invA));
            SampleGradient(mapB, colB, UMulShift8(sMin(in.b(), in.a()), invA));

            out = openktg::core::pixel(static_cast<openktg::red16_t>(MulIntens(sMin(colR.r() + colG.r() + colB.r(), 65535), in.a())),
                                       static_cast<openktg::green16_t>(MulIntens(sMin(colR.g() + colG.g() + colB.g(), 65535), in.a())),
                                       static_cast<openktg::blue16_t>(MulIntens(sMin(colR.b() + colG.b() + colB.b(), 65535), in.a())),
                                       static_cast<openktg::alpha16_t>(in.a()));
        }
        else // alpha==0
            out = in;
    }
}

void CoordRemap(openktg::texture &input, const openktg::texture &in, const openktg::texture &remapTex, sF32 strengthU, sF32 strengthV, sInt mode)
{
    assert(SizeMatchesWith(remapTex));

    const openktg::core::pixel *remap = remapTex.data();
    openktg::core::pixel *out = input.data();

    sInt u0 = input.min_x();
    sInt v0 = input.min_y();
    sInt scaleU = (1 << 24) * strengthU;
    sInt scaleV = (1 << 24) * strengthV;
    sInt stepU = 1 << (24 - input.shift_x());
    sInt stepV = 1 << (24 - input.shift_y());

    for (sInt y = 0; y < input.height(); y++)
    {
        sInt u = u0;
        sInt v = v0;

        for (sInt x = 0; x < input.width(); x++)
        {
            sInt dispU = u + MulShift16(scaleU, (remap->r() - 32768) * 2);
            sInt dispV = v + MulShift16(scaleV, (remap->g() - 32768) * 2);
            SampleFiltered(in, *out, dispU, dispV, mode);

            u += stepU;
            remap++;
            out++;
        }

        v0 += stepV;
    }
}

void Derive(openktg::texture &input, const openktg::texture &in, DeriveOp op, sF32 strength)
{
    assert(SizeMatchesWith(input, in));

    openktg::core::pixel *out = input.data();

    for (sInt y = 0; y < input.height(); y++)
    {
        for (sInt x = 0; x < input.width(); x++)
        {
            // sInt dx2 = in.Data[y * input.width() + ((x + 1) & (input.width() - 1))].r() - in.Data[y * input.width() + ((x - 1) & (input.width() - 1))].r();
            // sInt dy2 = in.Data[x + ((y + 1) & (input.height() - 1)) * input.width()].r() - in.Data[x + ((y - 1) & (input.height() - 1)) * input.width()].r();
            sInt dx2 = in.at((x + 1) & (input.width() - 1), y).r() - in.at((x - 1) & (input.width() - 1), y).r();
            sInt dy2 = in.at(x, (y + 1) & (input.height() - 1)).r() - in.at(x, (y - 1) & (input.height() - 1)).r();
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
static auto WrapCoord(sInt x, sInt width, sInt mode) -> sInt
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

void Blur(openktg::texture &input, const openktg::texture &inImg, sF32 sizex, sF32 sizey, sInt order, sInt wrapMode)
{
    assert(SizeMatchesWith(inImg));

    sInt sizePixX = sClamp(sizex, 0.0f, 1.0f) * 64 * inImg.width() / 2;
    sInt sizePixY = sClamp(sizey, 0.0f, 1.0f) * 64 * inImg.height() / 2;

    // no blur at all? just copy!
    if (order < 1 || (sizePixX <= 32 && sizePixY <= 32))
        input = inImg;
    else
    {
        // allocate pixel buffers
        sInt bufSize = sMax(input.width(), input.height());
        openktg::core::pixel *buf1 = new openktg::core::pixel[bufSize];
        openktg::core::pixel *buf2 = new openktg::core::pixel[bufSize];
        const openktg::texture *in = &inImg;

        // horizontal blur
        if (sizePixX > 32)
        {
            // go through image row by row
            for (sInt y = 0; y < input.height(); y++)
            {
                // copy pixels into buffer 1
                sCopyMem(buf1, &in->data()[y * input.width()], input.width() * sizeof(openktg::core::pixel));

                // blur order times, ping-ponging between buffers
                for (sInt i = 0; i < order; i++)
                {
                    Blur1DBuffer(buf2, buf1, input.width(), sizePixX, (wrapMode & ClampU) ? 1 : 0);
                    sSwap(buf1, buf2);
                }

                // copy pixels back
                sCopyMem(&input.data()[y * input.width()], buf1, input.width() * sizeof(openktg::core::pixel));
            }

            in = &input;
        }

        // vertical blur
        if (sizePixY > 32)
        {
            // go through image column by column
            for (sInt x = 0; x < input.width(); x++)
            {
                // copy pixels into buffer 1
                const openktg::core::pixel *src = &in->data()[x];
                openktg::core::pixel *dst = buf1;

                for (sInt y = 0; y < input.height(); y++)
                {
                    *dst++ = *src;
                    src += input.width();
                }

                // blur order times, ping-ponging between buffers
                for (sInt i = 0; i < order; i++)
                {
                    Blur1DBuffer(buf2, buf1, input.height(), sizePixY, (wrapMode & ClampV) ? 1 : 0);
                    sSwap(buf1, buf2);
                }

                // copy pixels back
                src = buf1;
                dst = &input.data()[x];

                for (sInt y = 0; y < input.height(); y++)
                {
                    *dst = *src++;
                    dst += input.width();
                }
            }
        }

        // clean up
        delete[] buf1;
        delete[] buf2;
    }
}

void Ternary(openktg::texture &input, const openktg::texture &in1Tex, const openktg::texture &in2Tex, const openktg::texture &in3Tex, TernaryOp op)
{
    assert(SizeMatchesWith(in1Tex) && SizeMatchesWith(in2Tex) && SizeMatchesWith(in3Tex));

    for (sInt i = 0; i < input.pixel_count(); i++)
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

void Paste(openktg::texture &input, const openktg::texture &bgTex, const openktg::texture &inTex, sF32 orgx, sF32 orgy, sF32 ux, sF32 uy, sF32 vx, sF32 vy,
           CombineOp op, sInt mode)
{
    assert(SizeMatchesWith(input, bgTex));

    // copy background over (if this image is not the background already)
    if (&input != &bgTex)
        input = bgTex;

    // calculate bounding rect
    sInt minX = sMax<sInt>(0, floor((orgx + sMin(ux, 0.0f) + sMin(vx, 0.0f)) * input.width()));
    sInt minY = sMax<sInt>(0, floor((orgy + sMin(uy, 0.0f) + sMin(vy, 0.0f)) * input.height()));
    sInt maxX = sMin<sInt>(input.width() - 1, ceil((orgx + sMax(ux, 0.0f) + sMax(vx, 0.0f)) * input.width()));
    sInt maxY = sMin<sInt>(input.height() - 1, ceil((orgy + sMax(uy, 0.0f) + sMax(vy, 0.0f)) * input.height()));

    // solve for u0,v0 and deltas (Cramer's rule)
    sF32 detM = ux * vy - uy * vx;
    if (fabs(detM) * input.width() * input.height() < 0.25f) // smaller than a pixel? skip it.
        return;

    sF32 invM = (1 << 24) / detM;
    sF32 rmx = (minX + 0.5f) / input.width() - orgx;
    sF32 rmy = (minY + 0.5f) / input.height() - orgy;
    sInt u0 = (rmx * vy - rmy * vx) * invM;
    sInt v0 = (ux * rmy - uy * rmx) * invM;
    sInt dudx = vy * invM / input.width();
    sInt dvdx = -uy * invM / input.width();
    sInt dudy = -vx * invM / input.height();
    sInt dvdy = ux * invM / input.height();

    for (sInt y = minY; y <= maxY; y++)
    {
        openktg::core::pixel *out = &input.at(minX, y);
        sInt u = u0;
        sInt v = v0;

        for (sInt x = minX; x <= maxX; x++)
        {
            if (u >= 0 && u < 0x1000000 && v >= 0 && v < 0x1000000)
            {
                openktg::core::pixel in;
                sInt transIn, transOut;

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
          const openktg::texture *falloffMap, sF32 px, sF32 py, sF32 pz, sF32 dx, sF32 dy, sF32 dz, const openktg::core::pixel &ambient,
          const openktg::core::pixel &diffuse, sBool directional)
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

    invX = 1.0f / input.width();
    invY = 1.0f / input.height();
    openktg::core::pixel *out = input.data();
    const openktg::core::pixel *surf = surface.data();
    const openktg::core::pixel *normal = normals.data();

    for (sInt y = 0; y < input.height(); y++)
    {
        for (sInt x = 0; x < input.width(); x++)
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
                SampleGradient(*falloffMap, falloff, spotTerm * (1 << 24));
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

void LinearCombine(openktg::texture &input, const openktg::core::pixel &color, sF32 constWeight, const LinearInput *inputs, sInt nInputs)
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
    sInt u0 = input.min_x();
    sInt v0 = input.min_y();
    sInt stepU = 1 << (24 - input.shift_x());
    sInt stepV = 1 << (24 - input.shift_y());
    openktg::core::pixel *out = input.data();

    for (sInt y = 0; y < input.height(); y++)
    {
        sInt u = u0;
        sInt v = v0;

        for (sInt x = 0; x < input.width(); x++)
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

                SampleFiltered(*in.Tex, inPix, u + uo[j], v + vo[j], in.FilterMode);

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