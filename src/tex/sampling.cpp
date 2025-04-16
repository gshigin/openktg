#include <algorithm>

#include <openktg/core/pixel.h>
#include <openktg/core/texture.h>
#include <openktg/tex/sampling.h>

void SampleNearest(const openktg::texture &input, openktg::pixel &result, int32_t x, int32_t y, int32_t wrapMode)
{
    if (wrapMode & 1)
        x = std::clamp<int32_t>(x, input.min_x(), 0x1000000 - input.min_x());
    if (wrapMode & 2)
        y = std::clamp<int32_t>(y, input.min_y(), 0x1000000 - input.min_y());

    x &= 0xffffff;
    y &= 0xffffff;

    int32_t ix = x >> (24 - input.shift_x());
    int32_t iy = y >> (24 - input.shift_y());

    result = input.data()[(iy << input.shift_x()) + ix];
}

void SampleBilinear(const openktg::texture &input, openktg::pixel &result, int32_t x, int32_t y, int32_t wrapMode)
{
    if (wrapMode & 1)
        x = std::clamp<int>(x, input.min_x(), 0x1000000 - input.min_x());
    if (wrapMode & 2)
        y = std::clamp<int>(y, input.min_y(), 0x1000000 - input.min_y());

    x = (x - input.min_x()) & 0xffffff;
    y = (y - input.min_y()) & 0xffffff;

    int32_t x0 = x >> (24 - input.shift_x());
    int32_t x1 = (x0 + 1) & (input.width() - 1);
    int32_t y0 = y >> (24 - input.shift_y());
    int32_t y1 = (y0 + 1) & (input.height() - 1);
    int32_t fx = static_cast<uint32_t>(x << (input.shift_x() + 8)) >> 16;
    int32_t fy = static_cast<uint32_t>(y << (input.shift_y() + 8)) >> 16;

    openktg::pixel t0 = lerp(input.at(x0, y0), input.at(x1, y0), fx);
    openktg::pixel t1 = lerp(input.at(x0, y1), input.at(x1, y1), fx);
    result = lerp(t0, t1, fy);
}

void SampleFiltered(const openktg::texture &input, openktg::pixel &result, int32_t x, int32_t y, int32_t filterMode)
{
    if (filterMode & FilterBilinear)
        SampleBilinear(input, result, x, y, filterMode);
    else
        SampleNearest(input, result, x, y, filterMode);
}

void SampleGradient(const openktg::texture &input, openktg::pixel &result, int32_t x)
{
    x = std::clamp(x, 0, 1 << 24);
    x -= x >> input.shift_x(); // x=(1<<24) -> Take rightmost pixel

    int32_t x0 = x >> (24 - input.shift_x());
    int32_t x1 = (x0 + 1) & (input.width() - 1);
    int32_t fx = static_cast<uint32_t>(x << (input.shift_x() + 8)) >> 16;

    result = lerp(input.data()[x0], input.data()[x1], fx);
}