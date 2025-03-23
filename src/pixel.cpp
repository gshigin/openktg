#include <algorithm>
#include <tuple>
#include <utility>

#include <openktg/pixel.h>
#include <openktg/types.h>
#include <openktg/utility.h>

namespace openktg
{
pixel::pixel(red8_t r, green8_t g, blue8_t b, alpha8_t a)
    : r_((static_cast<uint16_t>(r) << 8) | static_cast<uint16_t>(r)), g_((static_cast<uint16_t>(g) << 8) | static_cast<uint16_t>(g)),
      b_((static_cast<uint16_t>(b) << 8) | static_cast<uint16_t>(b)), a_((static_cast<uint16_t>(a) << 8) | static_cast<uint16_t>(a))
{
}

pixel::pixel(red16_t r, green16_t g, blue16_t b, alpha16_t a)
    : r_(std::to_underlying(r)), g_(std::to_underlying(g)), b_(std::to_underlying(b)), a_(std::to_underlying(a))
{
}

pixel::pixel(color32_t argb)
{
    std::uint16_t rv, gv, bv, av;

    rv = (std::to_underlying(argb) >> 16) & 0xff;
    gv = (std::to_underlying(argb) >> 8) & 0xff;
    bv = (std::to_underlying(argb) >> 0) & 0xff;
    av = (std::to_underlying(argb) >> 24) & 0xff;

    a_ = (av << 8) | av;
    r_ = utility::mul_intens((rv << 8) | rv, a_);
    g_ = utility::mul_intens((gv << 8) | gv, a_);
    b_ = utility::mul_intens((bv << 8) | bv, a_);
}

pixel::pixel(color64_t argb64)
{
    r_ = (std::to_underlying(argb64) >> 32) & 0xffff;
    g_ = (std::to_underlying(argb64) >> 16) & 0xffff;
    b_ = (std::to_underlying(argb64) >> 0) & 0xffff;
    a_ = (std::to_underlying(argb64) >> 48) & 0xffff;
}

auto pixel::operator+=(pixel other) -> pixel &
{
    r_ = std::min<std::uint32_t>(static_cast<std::uint32_t>(r_) + static_cast<std::uint32_t>(other.r_), 65535);
    g_ = std::min<std::uint32_t>(static_cast<std::uint32_t>(g_) + static_cast<std::uint32_t>(other.g_), 65535);
    b_ = std::min<std::uint32_t>(static_cast<std::uint32_t>(b_) + static_cast<std::uint32_t>(other.b_), 65535);
    a_ = std::min<std::uint32_t>(static_cast<std::uint32_t>(a_) + static_cast<std::uint32_t>(other.a_), 65535);

    return *this;
}

auto pixel::operator-=(pixel other) -> pixel &
{
    r_ = std::clamp<std::int32_t>(static_cast<std::int32_t>(r_) - static_cast<std::int32_t>(other.r_), 0, 65535);
    g_ = std::clamp<std::int32_t>(static_cast<std::int32_t>(g_) - static_cast<std::int32_t>(other.g_), 0, 65535);
    b_ = std::clamp<std::int32_t>(static_cast<std::int32_t>(b_) - static_cast<std::int32_t>(other.b_), 0, 65535);
    a_ = std::clamp<std::int32_t>(static_cast<std::int32_t>(a_) - static_cast<std::int32_t>(other.a_), 0, 65535);

    return *this;
}

auto pixel::operator*=(pixel other) -> pixel &
{
    r_ = utility::mul_intens(r_, other.r_);
    g_ = utility::mul_intens(g_, other.g_);
    b_ = utility::mul_intens(b_, other.b_);
    a_ = utility::mul_intens(a_, other.a_);

    return *this;
}

auto pixel::operator*=(std::uint16_t scalar) -> pixel &
{
    r_ = utility::mul_intens(r_, scalar);
    g_ = utility::mul_intens(g_, scalar);
    b_ = utility::mul_intens(b_, scalar);
    a_ = utility::mul_intens(a_, scalar);

    return *this;
}

auto pixel::operator|=(pixel other) -> pixel &
{
    r_ = std::max(r_, other.r_);
    g_ = std::max(g_, other.g_);
    b_ = std::max(b_, other.b_);
    a_ = std::max(a_, other.a_);

    return *this;
}
auto pixel::operator&=(pixel other) -> pixel &
{
    r_ = std::min(r_, other.r_);
    g_ = std::min(g_, other.g_);
    b_ = std::min(b_, other.b_);
    a_ = std::min(a_, other.a_);

    return *this;
}

auto pixel::operator~() const -> pixel
{
    pixel pnew;
    pnew.r_ = ~r_;
    pnew.g_ = ~g_;
    pnew.b_ = ~b_;
    pnew.a_ = ~a_;

    return pnew;
}

auto pixel::operator==(pixel other) const -> bool
{
    return std::tie(r_, g_, b_, a_) == std::tie(other.r_, other.g_, other.b_, other.a_);
}

auto pixel::lerp(pixel other, uint16_t t) -> pixel &
{
    r_ = utility::lerp(r_, other.r_, t);
    g_ = utility::lerp(g_, other.g_, t);
    b_ = utility::lerp(b_, other.b_, t);
    a_ = utility::lerp(a_, other.a_, t);

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

auto lerp(pixel lhs, pixel rhs, uint16_t t) -> pixel // t=0..65536 [0..1]
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
    const std::uint16_t inverse_alpha = ~rhs.a();
    return lhs * inverse_alpha + rhs;
}
auto compositeScreen(pixel lhs, pixel rhs) -> pixel
{
    return lhs += rhs * (~lhs);
}
} // namespace openktg

/*
switch(op)
{
case CombineAdd:
    out->r = sMin(out->r + in.r,65535);
    out->g = sMin(out->g + in.g,65535);
    out->b = sMin(out->b + in.b,65535);
    out->a = sMin(out->a + in.a,65535);
    break;

case CombineSub:
    out->r = sMax<sInt>(out->r - in.r,0);
    out->g = sMax<sInt>(out->g - in.g,0);
    out->b = sMax<sInt>(out->b - in.b,0);
    out->a = sMax<sInt>(out->a - in.a,0);
    break;

case CombineMulC:
    out->r = MulIntens(out->r,in.r);
    out->g = MulIntens(out->g,in.g);
    out->b = MulIntens(out->b,in.b);
    out->a = MulIntens(out->a,in.a);
    break;

case CombineMin:
    out->r = sMin(out->r,in.r);
    out->g = sMin(out->g,in.g);
    out->b = sMin(out->b,in.b);
    out->a = sMin(out->a,in.a);
    break;

case CombineMax:
    out->r = sMax(out->r,in.r);
    out->g = sMax(out->g,in.g);
    out->b = sMax(out->b,in.b);
    out->a = sMax(out->a,in.a);
    break;

case CombineSetAlpha:
    out->a = in.r;
    break;

case CombinePreAlpha:
    out->r = MulIntens(out->r,in.r);
    out->g = MulIntens(out->g,in.r);
    out->b = MulIntens(out->b,in.r);
    out->a = in.g;
    break;

case CombineOver:
    transIn = 65535 - in.a;

    out->r = MulIntens(transIn,out->r) + in.r;
    out->g = MulIntens(transIn,out->g) + in.g;
    out->b = MulIntens(transIn,out->b) + in.b;
    out->a += MulIntens(in.a,65535-out->a);
    break;

case CombineMultiply:
    transIn = 65535 - in.a;
    transOut = 65535 - out->a;

    out->r = MulIntens(transIn,out->r) + MulIntens(transOut,in.r) + MulIntens(in.r,out->r);
    out->g = MulIntens(transIn,out->g) + MulIntens(transOut,in.g) + MulIntens(in.g,out->g);
    out->b = MulIntens(transIn,out->b) + MulIntens(transOut,in.b) + MulIntens(in.b,out->b);
    out->a += MulIntens(in.a,transOut);
    break;

case CombineScreen:
    out->r += MulIntens(in.r,65535-out->r);
    out->g += MulIntens(in.g,65535-out->g);
    out->b += MulIntens(in.b,65535-out->b);
    out->a += MulIntens(in.a,65535-out->a);
    break;

case CombineDarken:
    out->r += in.r - sMax(MulIntens(in.r,out->a),MulIntens(out->r,in.a));
    out->g += in.g - sMax(MulIntens(in.g,out->a),MulIntens(out->g,in.a));
    out->b += in.b - sMax(MulIntens(in.b,out->a),MulIntens(out->b,in.a));
    out->a += MulIntens(in.a,65535-out->a);
    break;

case CombineLighten:
    out->r += in.r - sMin(MulIntens(in.r,out->a),MulIntens(out->r,in.a));
    out->g += in.g - sMin(MulIntens(in.g,out->a),MulIntens(out->g,in.a));
    out->b += in.b - sMin(MulIntens(in.b,out->a),MulIntens(out->b,in.a));
    out->a += MulIntens(in.a,65535-out->a);
    break;
}
}
*/