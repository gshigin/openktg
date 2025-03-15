#pragma once

#include <openktg/types.h>
#include <openktg/utility.h>

#include <cstdint>

// Pixel. Uses whole 16bit value range (0-65535).
// 0=>0.0, 65535=>1.0.
struct Pixel
{
    sU16 r, g, b, a; // OpenGL byte order

    void Init(sU8 r, sU8 g, sU8 b, sU8 a);
    void Init(sU32 rgba); // 0xaarrggbb (D3D style)

    void Lerp(sInt t, const Pixel &x, const Pixel &y); // t=0..65536

    void CompositeAdd(const Pixel &b);
    void CompositeMulC(const Pixel &b);
    void CompositeROver(const Pixel &b);
    void CompositeScreen(const Pixel &b);
};

// refactoring

namespace openktg
{
class alignas(std::uint64_t) pixel
{
  public:
    pixel() = default;
    pixel(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a);
    pixel(std::uint32_t rgba); // from 0xaarrggbb (D3D style)

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
    auto operator~() -> pixel;

    auto operator==(pixel) -> bool;

    auto lerp(pixel, uint16_t) -> pixel &; // t=0..65536
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
auto operator*(pixel lhs, std::uint16_t) -> pixel;
auto operator*(std::uint16_t, pixel lhs) -> pixel;

auto lerp(pixel lhs, pixel rhs, uint16_t t) -> pixel; // t=0..65536

auto compositeAdd(pixel lhs, pixel rhs) -> pixel;
auto compositeMulC(pixel lhs, pixel rhs) -> pixel;
auto compositeROver(pixel lhs, pixel rhs) -> pixel;
auto compositeScreen(pixel lhs, pixel rhs) -> pixel;
} // namespace openktg
