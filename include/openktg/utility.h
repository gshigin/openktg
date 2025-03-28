#pragma once

#include <openktg/macro.h>

#include <algorithm>
#include <bit>
#include <cmath>
#include <concepts>
#include <cstdint>

namespace openktg::utility
{
// Expands 8bit value to 16bit by concating value twice
OKTG(always_inline) constexpr auto expand8to16(std::uint8_t x) noexcept -> std::uint16_t
{
    return (std::uint16_t{x} << 8) | x;
}

// Return true if x is a power of 2, false otherwise
OKTG(always_inline) constexpr auto is_pow_of_2(std::int32_t x) noexcept -> bool
{
    if (x > 0) [[likely]]
    {
        return (x & (x - 1)) == 0;
    }
    return false;
}

// Returns floor(log2(x))
OKTG(always_inline) constexpr auto floor_log_2(std::int32_t x) noexcept -> std::int32_t
{
    if (x <= 0) [[unlikely]]
    {
        return -1;
    }
    return 31 - std::countl_zero(static_cast<std::uint32_t>(x));
}

template <std::floating_point T> OKTG(always_inline) constexpr auto lerp(T a, T b, T t) noexcept -> T
{
    return std::fma(t, b - a, a);
}

// Linearly interpolate between a and b with t=0..65536 [0,1]
// 0 <= a, b < 65536.
OKTG(always_inline) constexpr auto lerp(std::uint16_t a, std::uint16_t b, std::uint32_t t) noexcept -> std::uint16_t
{
    t = std::clamp<std::uint32_t>(t, 0x0, 0x10000);
    return a + ((t * (static_cast<std::int32_t>(b) - static_cast<std::int32_t>(a))) >> 16);
}

// Multiply intensities.
// Returns the result of round(a*b/65535.0)
OKTG(always_inline) constexpr auto mul_intens(std::uint16_t a, std::uint16_t b) noexcept -> std::uint16_t
{
    // 0x8000 added for right rounding
    const std::uint32_t x = static_cast<std::uint32_t>(a) * static_cast<std::uint32_t>(b) + 0x8000;
    return (x + (x >> 16)) >> 16;
}

// Returns the result of round(a*b/65536)
OKTG(always_inline) auto mul_shift_16(std::int32_t a, std::int32_t b) noexcept -> std::int32_t
{
    return static_cast<std::int32_t>((static_cast<std::int64_t>(a) * static_cast<std::int64_t>(b) + 0x8000) >> 16);
}

// Returns the result of round(a*b/256)
OKTG(always_inline) auto unsigned_mul_shift_8(std::uint32_t a, std::uint32_t b) noexcept -> std::uint32_t
{
    return (static_cast<std::uint64_t>(a) * static_cast<std::uint64_t>(b) + 0x80) >> 8;
}
} // namespace openktg::utility