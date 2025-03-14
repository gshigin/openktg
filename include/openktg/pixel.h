#pragma once

#include <openktg/types.h>

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