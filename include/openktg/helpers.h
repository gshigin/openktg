#pragma once

#include <openktg/types.h>

/****************************************************************************/
/***                                                                      ***/
/***   Helpers                                                            ***/
/***                                                                      ***/
/****************************************************************************/

// Return sTRUE if x is a power of 2, sFALSE otherwise
static sBool IsPowerOf2(sInt x)
{
    return (x & (x - 1)) == 0;
}

// Returns floor(log2(x))
static sInt FloorLog2(sInt x)
{
    sInt res = 0;

    if (x & 0xffff0000)
        x >>= 16, res += 16;
    if (x & 0x0000ff00)
        x >>= 8, res += 8;
    if (x & 0x000000f0)
        x >>= 4, res += 4;
    if (x & 0x0000000c)
        x >>= 2, res += 2;
    if (x & 0x00000002)
        res++;

    return res;
}

// Multiply intensities.
// Returns the result of round(a*b/65535.0)
static sU32 MulIntens(sU32 a, sU32 b)
{
    sU32 x = a * b + 0x8000;
    return (x + (x >> 16)) >> 16;
}

// Returns the result of round(a*b/65536)
static sInt MulShift16(sInt a, sInt b)
{
    return (sS64(a) * sS64(b) + 0x8000) >> 16;
}

// Returns the result of round(a*b/256)
static sU32 UMulShift8(sU32 a, sU32 b)
{
    return (sU64(a) * sU64(b) + 0x80) >> 8;
}

// Linearly interpolate between a and b with t=0..65536 [0,1]
// 0<=a,b<65536.
static sInt Lerp(sInt t, sInt a, sInt b)
{
    return a + ((t * (b - a)) >> 16);
}

static sF32 LerpF(sF32 t, sF32 a, sF32 b)
{
    return a + t * (b - a);
}