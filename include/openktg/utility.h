#pragma once

#include <algorithm>
#include <bit>
#include <cstdint>

namespace openktg::utility
{
// Return true if x is a power of 2, false otherwise
constexpr auto is_pow_of_2(std::int32_t x) noexcept -> bool
{
    if (x > 0) [[likely]]
    {
        return (x & (x - 1)) == 0;
    }
    return false;
}

// Returns floor(log2(x))
constexpr auto floor_log_2(std::int32_t x) noexcept -> std::int32_t
{
    if (x <= 0) [[unlikely]]
    {
        return -1;
    }
    return 31 - std::countl_zero(static_cast<std::uint32_t>(x));
}

// Linearly interpolate between a and b with t=0..65536 [0,1]
// 0 <= a, b < 65536.
constexpr auto lerp(std::uint16_t a, std::uint16_t b, std::uint32_t t) noexcept -> std::uint16_t
{
    t = std::clamp<std::uint32_t>(t, 0x0, 0x10000);
    return a + ((t * (static_cast<std::int32_t>(b) - static_cast<std::int32_t>(a))) >> 16);
}

// Multiply intensities.
// Returns the result of round(a*b/65535.0)
constexpr auto mult_intens(std::uint16_t a, std::uint16_t b) noexcept -> std::uint16_t
{
    // 0x8000 added for right rounding
    const std::uint32_t x = static_cast<std::uint32_t>(a) * static_cast<std::uint32_t>(b) + 0x8000;
    return (x + (x >> 16)) >> 16;
}
} // namespace openktg::utility