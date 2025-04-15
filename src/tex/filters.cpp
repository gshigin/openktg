#include <algorithm>
#include <cassert>

#include <openktg/tex/filters.h>
#include <openktg/core/pixel.h>
#include <openktg/core/texture.h>
#include <openktg/core/matrix.h>
#include <openktg/tex/sampling.h>
#include <openktg/util/utility.h>

void ColorMatrixTransform(openktg::texture &input, const openktg::texture &x, const openktg::matrix44<float> &matrix, bool clampPremult)
{
    assert(texture_size_matches(input, x));

    openktg::matrix44<int> m;
    std::transform(matrix.data.begin(), matrix.data.end(), m.data.begin(), [](const auto &fv) { return fv * 65536.0f; });

    for (int32_t i = 0; i < input.pixel_count(); i++)
    {
        openktg::pixel &out = input.data()[i];
        const openktg::pixel &in = x.data()[i];

        // some kind of pixel matrix multiplication
        int32_t r = openktg::util::mul_shift_16(m(0, 0), in.r()) + openktg::util::mul_shift_16(m(0, 1), in.g()) + openktg::util::mul_shift_16(m(0, 2), in.b()) + openktg::util::mul_shift_16(m(0, 3), in.a());
        int32_t g = openktg::util::mul_shift_16(m(1, 0), in.r()) + openktg::util::mul_shift_16(m(1, 1), in.g()) + openktg::util::mul_shift_16(m(1, 2), in.b()) + openktg::util::mul_shift_16(m(1, 3), in.a());
        int32_t b = openktg::util::mul_shift_16(m(2, 0), in.r()) + openktg::util::mul_shift_16(m(2, 1), in.g()) + openktg::util::mul_shift_16(m(2, 2), in.b()) + openktg::util::mul_shift_16(m(2, 3), in.a());
        int32_t a = openktg::util::mul_shift_16(m(3, 0), in.r()) + openktg::util::mul_shift_16(m(3, 1), in.g()) + openktg::util::mul_shift_16(m(3, 2), in.b()) + openktg::util::mul_shift_16(m(3, 3), in.a());

        a = std::clamp<int32_t>(a, 0, 65535);
        r = std::clamp<int32_t>(r, 0, 65535);
        g = std::clamp<int32_t>(g, 0, 65535);
        b = std::clamp<int32_t>(b, 0, 65535);

        out = openktg::pixel{static_cast<openktg::red16_t>(r), static_cast<openktg::green16_t>(g), static_cast<openktg::blue16_t>(b),
                             static_cast<openktg::alpha16_t>(a)};

        if (clampPremult)
        {
            out.clamp_premult();
        }
    }
}

void CoordMatrixTransform(openktg::texture &input, const openktg::texture &in, const openktg::matrix44<float> &matrix, int32_t mode)
{
    assert(texture_size_matches(input, in));

    int32_t scaleX = 1 << (24 - input.shift_x());
    int32_t scaleY = 1 << (24 - input.shift_y());

    int32_t dudx = matrix(0, 0) * scaleX;
    int32_t dudy = matrix(0, 1) * scaleY;
    int32_t dvdx = matrix(1, 0) * scaleX;
    int32_t dvdy = matrix(1, 1) * scaleY;

    int32_t u0 = matrix(0, 3) * (1 << 24) + ((dudx + dudy) >> 1);
    int32_t v0 = matrix(1, 3) * (1 << 24) + ((dvdx + dvdy) >> 1);
    openktg::core::pixel *out = input.data();

    for (int32_t y = 0; y < input.height(); y++)
    {
        int32_t u = u0;
        int32_t v = v0;

        for (int32_t x = 0; x < input.width(); x++)
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
    assert(texture_size_matches(input, inTex));

    for (int32_t i = 0; i < input.pixel_count(); i++)
    {
        const openktg::core::pixel &in = inTex.data()[i];
        openktg::core::pixel &out = input.data()[i];

        if (in.a() == 65535) // alpha==1, everything easy.
        {
            openktg::core::pixel colR, colG, colB;

            SampleGradient(mapR, colR, (in.r() << 8) + ((in.r() + 128) >> 8));
            SampleGradient(mapG, colG, (in.g() << 8) + ((in.g() + 128) >> 8));
            SampleGradient(mapB, colB, (in.b() << 8) + ((in.b() + 128) >> 8));

            out = openktg::core::pixel(static_cast<openktg::red16_t>(std::min(colR.r() + colG.r() + colB.r(), 65535)),
                                       static_cast<openktg::green16_t>(std::min(colR.g() + colG.g() + colB.g(), 65535)),
                                       static_cast<openktg::blue16_t>(std::min(colR.b() + colG.b() + colB.b(), 65535)), static_cast<openktg::alpha16_t>(in.a()));
        }
        else if (in.a()) // alpha!=0
        {
            openktg::core::pixel colR, colG, colB;
            uint32_t invA = (65535U << 16) / in.a();

            SampleGradient(mapR, colR, openktg::util::unsigned_mul_shift_8(std::min(in.r(), in.a()), invA));
            SampleGradient(mapG, colG, openktg::util::unsigned_mul_shift_8(std::min(in.g(), in.a()), invA));
            SampleGradient(mapB, colB, openktg::util::unsigned_mul_shift_8(std::min(in.b(), in.a()), invA));

            out = openktg::core::pixel(static_cast<openktg::red16_t>(openktg::util::mul_intens(std::min(colR.r() + colG.r() + colB.r(), 65535), in.a())),
                                       static_cast<openktg::green16_t>(openktg::util::mul_intens(std::min(colR.g() + colG.g() + colB.g(), 65535), in.a())),
                                       static_cast<openktg::blue16_t>(openktg::util::mul_intens(std::min(colR.b() + colG.b() + colB.b(), 65535), in.a())),
                                       static_cast<openktg::alpha16_t>(in.a()));
        }
        else // alpha==0
            out = in;
    }
}

void CoordRemap(openktg::texture &input, const openktg::texture &in, const openktg::texture &remapTex, float strengthU, float strengthV, int32_t mode)
{
    assert(texture_size_matches(input, remapTex));

    const openktg::core::pixel *remap = remapTex.data();
    openktg::core::pixel *out = input.data();

    int32_t u0 = input.min_x();
    int32_t v0 = input.min_y();
    int32_t scaleU = (1 << 24) * strengthU;
    int32_t scaleV = (1 << 24) * strengthV;
    int32_t stepU = 1 << (24 - input.shift_x());
    int32_t stepV = 1 << (24 - input.shift_y());

    for (int32_t y = 0; y < input.height(); y++)
    {
        int32_t u = u0;
        int32_t v = v0;

        for (int32_t x = 0; x < input.width(); x++)
        {
            int32_t dispU = u + openktg::util::mul_shift_16(scaleU, (remap->r() - 32768) * 2);
            int32_t dispV = v + openktg::util::mul_shift_16(scaleV, (remap->g() - 32768) * 2);
            SampleFiltered(in, *out, dispU, dispV, mode);

            u += stepU;
            remap++;
            out++;
        }

        v0 += stepV;
    }
}

void Derive(openktg::texture &input, const openktg::texture &in, DeriveOp op, float strength)
{
    assert(texture_size_matches(input, in));

    openktg::core::pixel *out = input.data();

    for (int32_t y = 0; y < input.height(); y++)
    {
        for (int32_t x = 0; x < input.width(); x++)
        {
            // int32_t dx2 = in.Data[y * input.width() + ((x + 1) & (input.width() - 1))].r() - in.Data[y * input.width() + ((x - 1) & (input.width() - 1))].r();
            // int32_t dy2 = in.Data[x + ((y + 1) & (input.height() - 1)) * input.width()].r() - in.Data[x + ((y - 1) & (input.height() - 1)) * input.width()].r();
            int32_t dx2 = in.at((x + 1) & (input.width() - 1), y).r() - in.at((x - 1) & (input.width() - 1), y).r();
            int32_t dy2 = in.at(x, (y + 1) & (input.height() - 1)).r() - in.at(x, (y - 1) & (input.height() - 1)).r();
            float dx = dx2 * strength / (2 * 65535.0f);
            float dy = dy2 * strength / (2 * 65535.0f);

            switch (op)
            {
            case DeriveGradient: {
                *out = openktg::core::pixel{static_cast<openktg::red16_t>(std::clamp<int32_t>(dx * 32768.0f + 32768.0f, 0, 65535)),
                                            static_cast<openktg::green16_t>(std::clamp<int32_t>(dy * 32768.0f + 32768.0f, 0, 65535)),
                                            static_cast<openktg::blue16_t>(0), static_cast<openktg::alpha16_t>(65535)};
                break;
            }
            case DeriveNormals: {
                // (1 0 dx)^T x (0 1 dy)^T = (-dx -dy 1)
                float scale = 32768.0f * openktg::util::rsqrt(1.0f + dx * dx + dy * dy);

                *out = openktg::core::pixel{static_cast<openktg::red16_t>(std::clamp<int32_t>(-dx * scale + 32768.0f, 0, 65535)),
                                            static_cast<openktg::green16_t>(std::clamp<int32_t>(-dy * scale + 32768.0f, 0, 65535)),
                                            static_cast<openktg::blue16_t>(std::clamp<int32_t>(scale + 32768.0f, 0, 65535)), static_cast<openktg::alpha16_t>(65535)};
                break;
            }
            }
            out++;
        }
    }
}

// Wrap computation on pixel coordinates
static auto WrapCoord(int32_t x, int32_t width, int32_t mode) -> int32_t
{
    if (mode == 0) // wrap
        return x & (width - 1);
    else
        return std::clamp(x, 0, width - 1);
}

// Size is half of edge length in pixels, 26.6 fixed point
static void Blur1DBuffer(openktg::core::pixel *dst, const openktg::core::pixel *src, int32_t width, int32_t sizeFixed, int32_t wrapMode)
{
    assert(sizeFixed > 32); // kernel should be wider than one pixel
    int32_t frac = (sizeFixed - 32) & 63;
    int32_t offset = (sizeFixed + 32) >> 6;

    assert(((offset - 1) * 64 + frac + 32) == sizeFixed);
    uint32_t denom = sizeFixed * 2;
    uint32_t bias = denom / 2;

    // initialize accumulators
    uint32_t accu[4];
    if (wrapMode == 0) // wrap around
    {
        // leftmost and rightmost pixels (the partially covered ones)
        int32_t xl = WrapCoord(-offset, width, wrapMode);
        int32_t xr = WrapCoord(offset, width, wrapMode);
        accu[0] = frac * (src[xl].r() + src[xr].r()) + bias;
        accu[1] = frac * (src[xl].g() + src[xr].g()) + bias;
        accu[2] = frac * (src[xl].b() + src[xr].b()) + bias;
        accu[3] = frac * (src[xl].a() + src[xr].a()) + bias;

        // inner part of filter kernel
        for (int32_t x = -offset + 1; x <= offset - 1; x++)
        {
            int32_t xc = WrapCoord(x, width, wrapMode);

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
        int32_t xr = WrapCoord(offset, width, wrapMode);
        accu[0] += frac * src[xr].r();
        accu[1] += frac * src[xr].g();
        accu[2] += frac * src[xr].b();
        accu[3] += frac * src[xr].a();

        // inner part of filter kernel (the right half)
        for (int32_t x = 1; x <= offset - 1; x++)
        {
            int32_t xc = WrapCoord(x, width, wrapMode);

            accu[0] += src[xc].r() << 6;
            accu[1] += src[xc].g() << 6;
            accu[2] += src[xc].b() << 6;
            accu[3] += src[xc].a() << 6;
        }
    }

    // generate output pixels
    for (int32_t x = 0; x < width; x++)
    {
        // write out state of accumulator
        dst[x] = openktg::core::pixel(static_cast<openktg::red16_t>(accu[0] / denom), static_cast<openktg::green16_t>(accu[1] / denom),
                                      static_cast<openktg::blue16_t>(accu[2] / denom), static_cast<openktg::alpha16_t>(accu[3] / denom));

        // update accumulator
        int32_t xl0 = WrapCoord(x - offset + 0, width, wrapMode);
        int32_t xl1 = WrapCoord(x - offset + 1, width, wrapMode);
        int32_t xr0 = WrapCoord(x + offset + 0, width, wrapMode);
        int32_t xr1 = WrapCoord(x + offset + 1, width, wrapMode);

        accu[0] += 64 * (src[xr0].r() - src[xl1].r()) + frac * (src[xr1].r() - src[xr0].r() - src[xl0].r() + src[xl1].r());
        accu[1] += 64 * (src[xr0].g() - src[xl1].g()) + frac * (src[xr1].g() - src[xr0].g() - src[xl0].g() + src[xl1].g());
        accu[2] += 64 * (src[xr0].b() - src[xl1].b()) + frac * (src[xr1].b() - src[xr0].b() - src[xl0].b() + src[xl1].b());
        accu[3] += 64 * (src[xr0].a() - src[xl1].a()) + frac * (src[xr1].a() - src[xr0].a() - src[xl0].a() + src[xl1].a());
    }
}

void Blur(openktg::texture &input, const openktg::texture &inImg, float sizex, float sizey, int32_t order, int32_t wrapMode)
{
    assert(texture_size_matches(input, inImg));
    

    int32_t sizePixX = std::clamp(sizex, 0.0f, 1.0f) * 64 * inImg.width() / 2;
    int32_t sizePixY = std::clamp(sizey, 0.0f, 1.0f) * 64 * inImg.height() / 2;

    // no blur at all? just copy!
    if (order < 1 || (sizePixX <= 32 && sizePixY <= 32))
        input = inImg;
    else
    {
        // allocate pixel buffers
        int32_t bufSize = std::max(input.width(), input.height());
        openktg::core::pixel *buf1 = new openktg::core::pixel[bufSize];
        openktg::core::pixel *buf2 = new openktg::core::pixel[bufSize];
        const openktg::texture *in = &inImg;

        // horizontal blur
        if (sizePixX > 32)
        {
            // go through image row by row
            for (int32_t y = 0; y < input.height(); y++)
            {
                // copy pixels into buffer 1
                std::memcpy(buf1, &in->data()[y * input.width()], input.width() * sizeof(openktg::core::pixel));

                // blur order times, ping-ponging between buffers
                for (int32_t i = 0; i < order; i++)
                {
                    Blur1DBuffer(buf2, buf1, input.width(), sizePixX, (wrapMode & ClampU) ? 1 : 0);
                    std::swap(buf1, buf2);
                }

                // copy pixels back
                std::memcpy(&input.data()[y * input.width()], buf1, input.width() * sizeof(openktg::core::pixel));
            }

            in = &input;
        }

        // vertical blur
        if (sizePixY > 32)
        {
            // go through image column by column
            for (int32_t x = 0; x < input.width(); x++)
            {
                // copy pixels into buffer 1
                const openktg::core::pixel *src = &in->data()[x];
                openktg::core::pixel *dst = buf1;

                for (int32_t y = 0; y < input.height(); y++)
                {
                    *dst++ = *src;
                    src += input.width();
                }

                // blur order times, ping-ponging between buffers
                for (int32_t i = 0; i < order; i++)
                {
                    Blur1DBuffer(buf2, buf1, input.height(), sizePixY, (wrapMode & ClampV) ? 1 : 0);
                    std::swap(buf1, buf2);
                }

                // copy pixels back
                src = buf1;
                dst = &input.data()[x];

                for (int32_t y = 0; y < input.height(); y++)
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