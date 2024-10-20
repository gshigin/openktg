#include <openktg/pixel.h>
#include <openktg/types.h>
#include <openktg/utility.h>

namespace openktg
{
    pixel::pixel(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a) 
        : r_((r << 8) | r)
        , g_((g << 8) | g)
        , b_((b << 8) | b)
        , a_((a << 8) | a) {}

    pixel::pixel(std::uint32_t rgba)
    {
        std::uint16_t rv, gv, bv, av;

        rv = (rgba >> 16) & 0xff;
        gv = (rgba >>  8) & 0xff;
        bv = (rgba >>  0) & 0xff;
        av = (rgba >> 24) & 0xff;

        a_ = (av << 8) | av;
        r_ = utility::mult_intens((rv << 8) | rv, a_);
        g_ = utility::mult_intens((gv << 8) | gv, a_);
        b_ = utility::mult_intens((bv << 8) | bv, a_);
    }

    pixel& pixel::lerp(pixel other, uint16_t t)
    {
        r_ = utility::lerp(r_, other.r_, t);
        g_ = utility::lerp(g_, other.g_, t);
        b_ = utility::lerp(b_, other.b_, t);
        a_ = utility::lerp(a_, other.a_, t);

        return *this;
    }

    pixel& pixel::compositeAdd(pixel other)
    {
        r_ = sClamp<std::uint32_t>(static_cast<std::uint32_t>(r_) + static_cast<std::uint32_t>(other.r_), 0, 65535);
        g_ = sClamp<std::uint32_t>(static_cast<std::uint32_t>(g_) + static_cast<std::uint32_t>(other.g_), 0, 65535);
        b_ = sClamp<std::uint32_t>(static_cast<std::uint32_t>(b_) + static_cast<std::uint32_t>(other.b_), 0, 65535);
        a_ = sClamp<std::uint32_t>(static_cast<std::uint32_t>(a_) + static_cast<std::uint32_t>(other.a_), 0, 65535);

        return *this;
    }

    pixel& pixel::compositeMulC(pixel other)
    {
        r_ = utility::mult_intens(r_, other.r_);
        g_ = utility::mult_intens(g_, other.g_);
        b_ = utility::mult_intens(b_, other.b_);
        a_ = utility::mult_intens(a_, other.a_);

        return *this;
    }

    pixel& pixel::compositeROver(pixel other)
    {
        std::uint16_t transIn = 65535 - other.a_;
        r_ = utility::mult_intens(transIn, r_) + other.r_;
        g_ = utility::mult_intens(transIn, g_) + other.g_;
        b_ = utility::mult_intens(transIn, b_) + other.b_;
        a_ = utility::mult_intens(transIn, a_) + other.a_;

        return *this;
    }

    pixel& pixel::compositeScreen(pixel other)
    {
        r_ += utility::mult_intens(other.r_, 65535 - r_);
        g_ += utility::mult_intens(other.g_, 65535 - g_);
        b_ += utility::mult_intens(other.b_, 65535 - b_);
        a_ += utility::mult_intens(other.a_, 65535 - a_);

        return *this;
    }

    pixel lerp(pixel lhs, pixel rhs, uint16_t t) // t=0..65536
    {
        return lhs.lerp( rhs, t );
    }
    pixel compositeAdd(pixel lhs, pixel rhs)
    {
        return lhs.compositeAdd( rhs );
    }
    pixel compositeMulC(pixel lhs, pixel rhs)
    {
        return lhs.compositeMulC( rhs );
    }
    pixel compositeROver(pixel lhs, pixel rhs)
    {
        return lhs.compositeROver( rhs );
    }
    pixel compositeScreen(pixel lhs, pixel rhs)
    {
        return lhs.compositeScreen( rhs );
    }
}