#pragma once

#include <cstdint>

namespace openktg::utility
{
    // Linearly interpolate between a and b with t=0..65536 [0,1]
    // 0 <= a, b < 65536.
    constexpr std::uint16_t lerp(std::uint16_t a, std::uint16_t b, std::uint32_t t)
    {
        // t = std::clamp<std::uint32_t>(t, 0x0, 0x10000);
        // if(b < a)
        // {
        //     return lerp(b, a, 0x10000 - t);
        // }
        // // a <= b
        // return a + static_cast<std::uint16_t>((t * static_cast<std::uint32_t>(b - a)) >> 16);
        return a + ((t * (b - a)) >> 16); // for some reason it just works
    }

    // Multiply intensities.
    // Returns the result of round(a*b/65535.0)
    constexpr std::uint16_t mult_intens(std::uint16_t a, std::uint16_t b)
    {
        // 0x8000 added for right rounding
        const std::uint32_t x = static_cast<std::uint32_t>(a) * static_cast<std::uint32_t>(b) + 0x8000;
        return (x + (x >> 16)) >> 16;
    }
} // namespace openktg::utility