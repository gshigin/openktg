#pragma once

#include <openktg/types.h>
#include <openktg/utility.h>

#include <cmath>

/****************************************************************************/
/***                                                                      ***/
/***   Helpers                                                            ***/
/***                                                                      ***/
/****************************************************************************/

// Return sTRUE if x is a power of 2, sFALSE otherwise
static inline auto IsPowerOf2(sInt x) -> sBool
{
    return openktg::utility::is_pow_of_2(x);
}

// Returns floor(log2(x))
static inline auto FloorLog2(sInt x) -> sInt
{
    return openktg::utility::floor_log_2(x);
}

// Multiply intensities.
// Returns the result of round(a*b/65535.0)
static inline auto MulIntens(sU32 a, sU32 b) -> sU32
{
    return openktg::utility::mult_intens(a, b);
}

// Returns the result of round(a*b/65536)
static inline auto MulShift16(sInt a, sInt b) -> sInt
{
    return (sS64(a) * sS64(b) + 0x8000) >> 16;
}

// Returns the result of round(a*b/256)
static inline auto UMulShift8(sU32 a, sU32 b) -> sU32
{
    return (sU64(a) * sU64(b) + 0x80) >> 8;
}

// Linearly interpolate between a and b with t=0..65536 [0,1]
// 0<=a,b<65536.
static inline auto Lerp(sInt t, sInt a, sInt b) -> sInt
{
    return openktg::utility::lerp(a, b, t);
}

static inline auto LerpF(sF32 t, sF32 a, sF32 b) -> sF32
{
    return std::lerp(a, b, t);
}