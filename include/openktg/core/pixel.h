#pragma once

#include <cstdint>

#include <openktg/core/color.h>

namespace openktg::inline core
{

class alignas(std::uint64_t) pixel
{
  public:
    pixel() = default;
    pixel(red8_t r, green8_t g, blue8_t b, alpha8_t a);
    pixel(red16_t r, green16_t g, blue16_t b, alpha16_t a);
    pixel(color32_t argb); // from 0xaarrggbb (D3D style)
    pixel(color64_t argb64);

    pixel(const pixel &) = default;
    auto operator=(const pixel &) -> pixel & = default;
    pixel(pixel &&) noexcept = default;
    auto operator=(pixel &&) noexcept -> pixel & = default;
    ~pixel() = default;

    [[nodiscard]] auto r() const -> std::uint16_t
    {
        return r_;
    }
    [[nodiscard]] auto g() const -> std::uint16_t
    {
        return g_;
    }
    [[nodiscard]] auto b() const -> std::uint16_t
    {
        return b_;
    }
    [[nodiscard]] auto a() const -> std::uint16_t
    {
        return a_;
    }

    // combineAdd
    auto operator+=(pixel) -> pixel &;
    // combineDub
    auto operator-=(pixel) -> pixel &;
    // mulIntens
    auto operator*=(pixel) -> pixel &;
    // pixel * scalar
    auto operator*=(std::uint16_t) -> pixel &;
    // inverse pixel
    auto operator~() const -> pixel;
    // max
    auto operator|=(pixel) -> pixel &;
    auto operator&=(pixel) -> pixel &;

    auto operator==(pixel) const -> bool;

    auto lerp(pixel, std::uint32_t) -> pixel &; // t=0..65536
    auto clamp_premult() -> pixel &;
    auto set_alpha(alpha16_t) -> pixel &;
    auto set_alpha(alpha8_t) -> pixel &;

  private:
    // OpenGL byte order
    std::uint16_t r_;
    std::uint16_t g_;
    std::uint16_t b_;
    std::uint16_t a_;
};

auto operator+(pixel lhs, pixel rhs) -> pixel;
auto operator-(pixel lhs, pixel rhs) -> pixel;
auto operator*(pixel lhs, pixel rhs) -> pixel;
auto operator|(pixel lhs, pixel rhs) -> pixel;
auto operator&(pixel lhs, pixel rhs) -> pixel;
auto operator*(pixel lhs, std::uint16_t) -> pixel;
auto operator*(std::uint16_t, pixel lhs) -> pixel;

auto lerp(pixel lhs, pixel rhs, uint32_t t) -> pixel; // t=0..65536

// composite
auto clampPremult(pixel p) -> pixel;
auto compositeAdd(pixel lhs, pixel rhs) -> pixel;
auto compositeMulC(pixel lhs, pixel rhs) -> pixel;
auto compositeROver(pixel lhs, pixel rhs) -> pixel;
auto compositeScreen(pixel lhs, pixel rhs) -> pixel;

// combine
auto combineOver(pixel lhs, pixel rhs) -> pixel;
auto combineMultiply(pixel lhs, pixel rhs) -> pixel;
auto combineScreen(pixel lhs, pixel rhs) -> pixel;
auto combineDarken(pixel lhs, pixel rhs) -> pixel;
auto combineLighten(pixel lhs, pixel rhs) -> pixel;
} // namespace openktg::inline core
