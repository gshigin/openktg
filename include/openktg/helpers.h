#pragma once

#include <cmath>

#include <openktg/macro.h>
#include <openktg/types.h>
#include <openktg/utility.h>

/****************************************************************************/
/***                                                                      ***/
/***   Helpers                                                            ***/
/***                                                                      ***/
/****************************************************************************/

// Return sTRUE if x is a power of 2, sFALSE otherwise
OKTG(always_inline) auto IsPowerOf2(sInt x) noexcept -> sBool
{
    return openktg::utility::is_pow_of_2(x);
}

// Returns floor(log2(x))
OKTG(always_inline) auto FloorLog2(sInt x) noexcept -> sInt
{
    return openktg::utility::floor_log_2(x);
}

// Multiply intensities.
// Returns the result of round(a*b/65535.0)
OKTG(always_inline) auto MulIntens(sU32 a, sU32 b) noexcept -> sU32
{
    return openktg::utility::mul_intens(a, b);
}

// Returns the result of round(a*b/65536)
OKTG(always_inline) auto MulShift16(sInt a, sInt b) noexcept -> sInt
{
    return openktg::utility::mul_shift_16(a, b);
}

// Returns the result of round(a*b/256)
OKTG(always_inline) auto UMulShift8(sU32 a, sU32 b) noexcept -> sU32
{
    return openktg::utility::unsigned_mul_shift_8(a, b);
}

// Linearly interpolate between a and b with t=0..65536 [0,1]
// 0<=a,b<65536.
OKTG(always_inline) auto Lerp(sInt t, sInt a, sInt b) noexcept -> sInt
{
    return openktg::utility::lerp(a, b, t);
}

OKTG(always_inline) auto LerpF(sF32 t, sF32 a, sF32 b) noexcept -> sF32
{
    return openktg::utility::lerp(a, b, t);
}