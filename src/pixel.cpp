#include <openktg/pixel.h>
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