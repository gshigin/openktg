#pragma once

#include <cstdint>

namespace openktg::inline core::inline color
{
// 8 bit channels
enum struct red8_t : std::uint8_t
{
};
enum struct green8_t : std::uint8_t
{
};
enum struct blue8_t : std::uint8_t
{
};
enum struct alpha8_t : std::uint8_t
{
};
// 32 bit ARGB
enum struct color32_t : std::uint32_t
{
};

// 16 bit channel
enum struct red16_t : std::uint16_t
{
};
enum struct green16_t : std::uint16_t
{
};
enum struct blue16_t : std::uint16_t
{
};
enum struct alpha16_t : std::uint16_t
{
};
// 64 bit ARGB
enum struct color64_t : std::uint64_t
{
};

inline namespace literals
{
constexpr auto operator"" _r(unsigned long long int val) -> red8_t
{
    return static_cast<red8_t>(val);
}
constexpr auto operator"" _g(unsigned long long int val) -> green8_t
{
    return static_cast<green8_t>(val);
}
constexpr auto operator"" _b(unsigned long long int val) -> blue8_t
{
    return static_cast<blue8_t>(val);
}
constexpr auto operator"" _a(unsigned long long int val) -> alpha8_t
{
    return static_cast<alpha8_t>(val);
}

constexpr auto operator"" _r16(unsigned long long int val) -> red16_t
{
    return static_cast<red16_t>(val);
}
constexpr auto operator"" _g16(unsigned long long int val) -> green16_t
{
    return static_cast<green16_t>(val);
}
constexpr auto operator"" _b16(unsigned long long int val) -> blue16_t
{
    return static_cast<blue16_t>(val);
}
constexpr auto operator"" _a16(unsigned long long int val) -> alpha16_t
{
    return static_cast<alpha16_t>(val);
}

constexpr auto operator"" _argb(unsigned long long int val) -> color32_t
{
    return static_cast<color32_t>(val);
}

constexpr auto operator"" _argb64(unsigned long long int val) -> color64_t
{
    return static_cast<color64_t>(val);
}
} // namespace literals
} // namespace openktg::inline core::inline color