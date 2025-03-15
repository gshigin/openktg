#include <openktg/pixel.h>
#include <openktg/types.h>

#include <tuple>

#include <openktg/helpers.h>

/****************************************************************************/
/***                                                                      ***/
/***   Pixel                                                              ***/
/***                                                                      ***/
/****************************************************************************/

void Pixel::Init(sU8 _r, sU8 _g, sU8 _b, sU8 _a)
{
    r = (_r << 8) | _r;
    g = (_g << 8) | _g;
    b = (_b << 8) | _b;
    a = (_a << 8) | _a;
}

void Pixel::Init(sU32 rgba)
{
    sU8 rv, gv, bv, av;

    rv = (rgba >> 16) & 0xff;
    gv = (rgba >> 8) & 0xff;
    bv = (rgba >> 0) & 0xff;
    av = (rgba >> 24) & 0xff;

    a = (av << 8) | av;
    r = MulIntens((rv << 8) | rv, a);
    g = MulIntens((gv << 8) | gv, a);
    b = MulIntens((bv << 8) | bv, a);
}

void Pixel::Lerp(sInt t, const Pixel &x, const Pixel &y)
{
    r = ::Lerp(t, x.r, y.r);
    g = ::Lerp(t, x.g, y.g);
    b = ::Lerp(t, x.b, y.b);
    a = ::Lerp(t, x.a, y.a);
}

void Pixel::CompositeAdd(const Pixel &x)
{
    r = sClamp<sInt>(r + x.r, 0, 65535);
    g = sClamp<sInt>(g + x.g, 0, 65535);
    b = sClamp<sInt>(b + x.b, 0, 65535);
    a = sClamp<sInt>(a + x.a, 0, 65535);
}

void Pixel::CompositeMulC(const Pixel &x)
{
    r = MulIntens(r, x.r);
    g = MulIntens(g, x.g);
    b = MulIntens(b, x.b);
    a = MulIntens(a, x.a);
}

void Pixel::CompositeROver(const Pixel &x)
{
    sInt transIn = 65535 - x.a;
    r = MulIntens(transIn, r) + x.r;
    g = MulIntens(transIn, g) + x.g;
    b = MulIntens(transIn, b) + x.b;
    a = MulIntens(transIn, a) + x.a;
}

void Pixel::CompositeScreen(const Pixel &x)
{
    r += MulIntens(x.r, 65535 - r);
    g += MulIntens(x.g, 65535 - g);
    b += MulIntens(x.b, 65535 - b);
    a += MulIntens(x.a, 65535 - a);
}

// refactoring

#include <openktg/utility.h>

namespace openktg
{
pixel::pixel(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a)
    : r_((static_cast<uint16_t>(r) << 8) | r), g_((static_cast<uint16_t>(g) << 8) | g), b_((static_cast<uint16_t>(b) << 8) | b),
      a_((static_cast<uint16_t>(a) << 8) | a)
{
}

pixel::pixel(std::uint32_t rgba)
{
    std::uint16_t rv, gv, bv, av;

    rv = (rgba >> 16) & 0xff;
    gv = (rgba >> 8) & 0xff;
    bv = (rgba >> 0) & 0xff;
    av = (rgba >> 24) & 0xff;

    a_ = (av << 8) | av;
    r_ = utility::mult_intens((rv << 8) | rv, a_);
    g_ = utility::mult_intens((gv << 8) | gv, a_);
    b_ = utility::mult_intens((bv << 8) | bv, a_);
}

auto pixel::operator+=(pixel other) -> pixel &
{
    r_ = std::clamp<std::uint32_t>(static_cast<std::uint32_t>(r_) + static_cast<std::uint32_t>(other.r_), 0, 65535);
    g_ = std::clamp<std::uint32_t>(static_cast<std::uint32_t>(g_) + static_cast<std::uint32_t>(other.g_), 0, 65535);
    b_ = std::clamp<std::uint32_t>(static_cast<std::uint32_t>(b_) + static_cast<std::uint32_t>(other.b_), 0, 65535);
    a_ = std::clamp<std::uint32_t>(static_cast<std::uint32_t>(a_) + static_cast<std::uint32_t>(other.a_), 0, 65535);

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
    r_ = utility::mult_intens(r_, other.r_);
    g_ = utility::mult_intens(g_, other.g_);
    b_ = utility::mult_intens(b_, other.b_);
    a_ = utility::mult_intens(a_, other.a_);

    return *this;
}

auto pixel::operator*=(std::uint16_t scalar) -> pixel &
{
    r_ = std::clamp<std::uint32_t>(static_cast<std::uint32_t>(r_) * static_cast<std::uint32_t>(scalar), 0, 65535);
    g_ = std::clamp<std::uint32_t>(static_cast<std::uint32_t>(g_) * static_cast<std::uint32_t>(scalar), 0, 65535);
    b_ = std::clamp<std::uint32_t>(static_cast<std::uint32_t>(b_) * static_cast<std::uint32_t>(scalar), 0, 65535);
    a_ = std::clamp<std::uint32_t>(static_cast<std::uint32_t>(a_) * static_cast<std::uint32_t>(scalar), 0, 65535);

    return *this;
}

auto pixel::operator~() -> pixel
{
    pixel pnew;
    pnew.r_ = ~r_;
    pnew.g_ = ~g_;
    pnew.b_ = ~b_;
    pnew.a_ = ~a_;

    return pnew;
}

auto pixel::operator==(pixel other) -> bool
{
    return std::tie(r_, g_, b_, a_) == std::tie(other.r_, other.g_, other.b_, other.a_);
}

pixel &pixel::lerp(pixel other, uint16_t t)
{
    r_ = utility::lerp(r_, other.r_, t);
    g_ = utility::lerp(g_, other.g_, t);
    b_ = utility::lerp(b_, other.b_, t);
    a_ = utility::lerp(a_, other.a_, t);

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

auto lerp(pixel lhs, pixel rhs, uint16_t t) -> pixel // t=0..65536 [0..1]
{
    return lhs.lerp(rhs, t);
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