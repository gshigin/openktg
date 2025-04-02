#include <utility>

#include <openktg/core/pixel.h>
#include <openktg/core/types.h>
#include <openktg/util/utility.h>

namespace openktg::inline core
{
pixel::pixel(red8_t r, green8_t g, blue8_t b, alpha8_t a)
    : r_(util::expand8to16(std::to_underlying(r))), g_(util::expand8to16(std::to_underlying(g))), b_(util::expand8to16(std::to_underlying(b))),
      a_(util::expand8to16(std::to_underlying(a)))
{
}

pixel::pixel(red16_t r, green16_t g, blue16_t b, alpha16_t a)
    : r_(std::to_underlying(r)), g_(std::to_underlying(g)), b_(std::to_underlying(b)), a_(std::to_underlying(a))
{
}

pixel::pixel(color32_t argb)
{
    const std::uint32_t val = std::to_underlying(argb);

    const std::uint8_t rv = (val >> 16) & 0xFF;
    const std::uint8_t gv = (val >> 8) & 0xFF;
    const std::uint8_t bv = (val >> 0) & 0xFF;
    const std::uint8_t av = (val >> 24) & 0xFF;

    a_ = util::expand8to16(av);
    r_ = util::mul_intens(util::expand8to16(rv), a_);
    g_ = util::mul_intens(util::expand8to16(gv), a_);
    b_ = util::mul_intens(util::expand8to16(bv), a_);
}

pixel::pixel(color64_t argb64)
{
    const std::uint64_t val = std::to_underlying(argb64);
    r_ = (val >> 32) & 0xFFFF;
    g_ = (val >> 16) & 0xFFFF;
    b_ = (val >> 0) & 0xFFFF;
    a_ = (val >> 48) & 0xFFFF;
}

auto pixel::operator+=(pixel other) -> pixel &
{
    r_ = (r_ > 0xFFFF - other.r_) ? 0xFFFF : r_ + other.r_;
    g_ = (g_ > 0xFFFF - other.g_) ? 0xFFFF : g_ + other.g_;
    b_ = (b_ > 0xFFFF - other.b_) ? 0xFFFF : b_ + other.b_;
    a_ = (a_ > 0xFFFF - other.a_) ? 0xFFFF : a_ + other.a_;

    return *this;
}

auto pixel::operator-=(pixel other) -> pixel &
{
    r_ = (r_ > other.r_) ? (r_ - other.r_) : 0;
    g_ = (g_ > other.g_) ? (g_ - other.g_) : 0;
    b_ = (b_ > other.b_) ? (b_ - other.b_) : 0;
    a_ = (a_ > other.a_) ? (a_ - other.a_) : 0;

    return *this;
}

auto pixel::operator*=(pixel other) -> pixel &
{
    r_ = util::mul_intens(r_, other.r_);
    g_ = util::mul_intens(g_, other.g_);
    b_ = util::mul_intens(b_, other.b_);
    a_ = util::mul_intens(a_, other.a_);

    return *this;
}

auto pixel::operator*=(std::uint16_t scalar) -> pixel &
{
    r_ = util::mul_intens(r_, scalar);
    g_ = util::mul_intens(g_, scalar);
    b_ = util::mul_intens(b_, scalar);
    a_ = util::mul_intens(a_, scalar);

    return *this;
}

auto pixel::operator|=(pixel other) -> pixel &
{
    r_ = r_ > other.r_ ? r_ : other.r_;
    g_ = g_ > other.g_ ? g_ : other.g_;
    b_ = b_ > other.b_ ? b_ : other.b_;
    a_ = a_ > other.a_ ? a_ : other.a_;

    return *this;
}

auto pixel::operator&=(pixel other) -> pixel &
{
    r_ = r_ < other.r_ ? r_ : other.r_;
    g_ = g_ < other.g_ ? g_ : other.g_;
    b_ = b_ < other.b_ ? b_ : other.b_;
    a_ = a_ < other.a_ ? a_ : other.a_;

    return *this;
}

auto pixel::operator~() const -> pixel
{
    return pixel{static_cast<red16_t>(~r_), static_cast<green16_t>(~g_), static_cast<blue16_t>(~b_), static_cast<alpha16_t>(~a_)};
}

auto pixel::operator==(pixel other) const -> bool
{
    return (r_ == other.r_) && (g_ == other.g_) && (b_ == other.b_) && (a_ == other.a_);
}

auto pixel::lerp(pixel other, std::uint32_t t) -> pixel &
{
    r_ = util::lerp(r_, other.r_, t);
    g_ = util::lerp(g_, other.g_, t);
    b_ = util::lerp(b_, other.b_, t);
    a_ = util::lerp(a_, other.a_, t);

    return *this;
}

auto pixel::clamp_premult() -> pixel &
{
    r_ = std::min<std::uint16_t>(r_, a_);
    g_ = std::min<std::uint16_t>(g_, a_);
    b_ = std::min<std::uint16_t>(b_, a_);

    return *this;
}

auto pixel::set_alpha(alpha16_t a) -> pixel &
{
    a_ = std::to_underlying(a);
    return *this;
}

auto pixel::set_alpha(alpha8_t a) -> pixel &
{
    a_ = static_cast<uint16_t>(a) << 8 | static_cast<uint16_t>(a);
    return *this;
}

auto operator+(pixel lhs, pixel rhs) -> pixel
{
    return lhs += rhs;
}

auto operator-(pixel lhs, pixel rhs) -> pixel
{
    return lhs -= rhs;
}

auto operator*(pixel lhs, pixel rhs) -> pixel
{
    return lhs *= rhs;
}

auto operator*(pixel lhs, std::uint16_t scalar) -> pixel
{
    return lhs *= scalar;
}

auto operator*(std::uint16_t scalar, pixel rhs) -> pixel
{
    return rhs *= scalar;
}

auto operator|(pixel lhs, pixel rhs) -> pixel
{
    return lhs |= rhs;
}

auto operator&(pixel lhs, pixel rhs) -> pixel
{
    return lhs &= rhs;
}

auto lerp(pixel lhs, pixel rhs, uint32_t t) -> pixel // t=0..65536 [0..1]
{
    return lhs.lerp(rhs, t);
}

auto clampPremult(pixel p) -> pixel
{
    return p.clamp_premult();
}

// composile operations
auto compositeAdd(pixel lhs, pixel rhs) -> pixel
{
    return lhs += rhs;
}
auto compositeMulC(pixel lhs, pixel rhs) -> pixel
{
    return lhs *= rhs;
}
auto compositeROver(pixel lhs, pixel rhs) -> pixel
{
    return ~rhs.a() * lhs + rhs;
}
auto compositeScreen(pixel lhs, pixel rhs) -> pixel
{
    return lhs += rhs * (~lhs);
}

// combine
auto combineOver(pixel lhs, pixel rhs) -> pixel
{
    return lhs + rhs * (~lhs.a());
}
auto combineMultiply(pixel lhs, pixel rhs) -> pixel
{
    return ((lhs * rhs) + (~lhs.a() * rhs) + (~rhs.a() * lhs)).set_alpha(static_cast<alpha16_t>(lhs.a() + rhs.a() - util::mul_intens(lhs.a(), rhs.a())));
}
auto combineScreen(pixel lhs, pixel rhs) -> pixel
{
    return rhs + lhs * (~rhs);
}
auto combineDarken(pixel lhs, pixel rhs) -> pixel
{
    return ((lhs | rhs) - ((lhs * rhs.a()) | (lhs.a() * rhs)) + (lhs & rhs)).set_alpha(static_cast<alpha16_t>(rhs.a() + util::mul_intens(lhs.a(), ~rhs.a())));
}
auto combineLighten(pixel lhs, pixel rhs) -> pixel
{
    return ((lhs & rhs) - ((lhs * rhs.a()) & (lhs.a() * rhs)) + (lhs | rhs)).set_alpha(static_cast<alpha16_t>(rhs.a() + util::mul_intens(lhs.a(), ~rhs.a())));
}
} // namespace openktg::inline core
