#include <openktg/types.h>
#include <openktg/pixel.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

#include <cstdint>

using namespace openktg;

namespace legacy
{
    union Pixel
    {
        struct
        {
            std::uint16_t r,g,b,a; // OpenGL byte order
        };
        std::uint64_t v;         // the whole value

        void Init(std::uint8_t r,std::uint8_t g,std::uint8_t b,std::uint8_t a);
        void Init(std::uint32_t rgba); // 0xaarrggbb (D3D style)

        void Lerp(int t,const Pixel &x,const Pixel &y); // t=0..65536
        
        void CompositeAdd(const Pixel &b);
        void CompositeMulC(const Pixel &b);
        void CompositeROver(const Pixel &b);
        void CompositeScreen(const Pixel &b);
    };

    static int Lerp(int t,int a,int b)
    {
        return a + ((t * (b-a)) >> 16);
    }

    static std::uint32_t MulIntens(std::uint32_t a,std::uint32_t b)
    {
        std::uint32_t x = a*b + 0x8000;
        return (x + (x >> 16)) >> 16;
    }

    void Pixel::Init(std::uint8_t _r,std::uint8_t _g,std::uint8_t _b,std::uint8_t _a)
    {
        r = (_r << 8) | _r;
        g = (_g << 8) | _g;
        b = (_b << 8) | _b;
        a = (_a << 8) | _a;
    }

    void Pixel::Init(std::uint32_t rgba)
    {
        std::uint8_t rv,gv,bv,av;

        rv = (rgba >> 16) & 0xff;
        gv = (rgba >>  8) & 0xff;
        bv = (rgba >>  0) & 0xff;
        av = (rgba >> 24) & 0xff;

        a = (av << 8) | av;
        r = MulIntens((rv << 8) | rv,a);
        g = MulIntens((gv << 8) | gv,a);
        b = MulIntens((bv << 8) | bv,a);
    }

    void Pixel::Lerp(int t,const Pixel &x,const Pixel &y)
    {
        r = legacy::Lerp(t,x.r,y.r);
        g = legacy::Lerp(t,x.g,y.g);
        b = legacy::Lerp(t,x.b,y.b);
        a = legacy::Lerp(t,x.a,y.a);
    }

    void Pixel::CompositeAdd(const Pixel &x)
    {
        r = sClamp<int>(r + x.r,0,65535);
        g = sClamp<int>(g + x.g,0,65535);
        b = sClamp<int>(b + x.b,0,65535);
        a = sClamp<int>(a + x.a,0,65535);
    }

    void Pixel::CompositeMulC(const Pixel &x)
    {
        r = MulIntens(r,x.r);
        g = MulIntens(g,x.g);
        b = MulIntens(b,x.b);
        a = MulIntens(a,x.a);
    }

    void Pixel::CompositeROver(const Pixel &x)
    {
        int transIn = 65535 - x.a;
        r = MulIntens(transIn,r) + x.r;
        g = MulIntens(transIn,g) + x.g;
        b = MulIntens(transIn,b) + x.b;
        a = MulIntens(transIn,a) + x.a;
    }

    void Pixel::CompositeScreen(const Pixel &x)
    {
        r += MulIntens(x.r,65535-r);
        g += MulIntens(x.g,65535-g);
        b += MulIntens(x.b,65535-b);
        a += MulIntens(x.a,65535-a);
    }
}