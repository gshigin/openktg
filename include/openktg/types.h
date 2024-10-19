/****************************************************************************/
/***                                                                      ***/
/***   Written by Fabian Giesen.                                          ***/
/***   I hereby place this code in the public domain.                     ***/
/***                                                                      ***/
/****************************************************************************/

// This is a very stripped down version of this header file and contains only
// the bare essentials for the texture generator.

#ifndef __TP_TYPES_HPP__
#define __TP_TYPES_HPP__

#include <cassert>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <algorithm>
#include <numbers>

/****************************************************************************/
/***                                                                      ***/
/***   Basic Types and Functions                                          ***/
/***                                                                      ***/
/****************************************************************************/

// remove most in favour of built-in types

using sU8   = std::uint8_t;   // for packed arrays
using sU16  = std::uint16_t;  // for packed arrays
using sU32  = std::uint32_t;  // for packed arrays and bitfields
using sU64  = std::uint64_t;  // use as needed
using sS8   = std::int8_t;    // for packed arrays
using sS16  = std::int16_t;   // for packed arrays
using sS32  = std::int32_t;   // for packed arrays
using sS64  = std::int64_t;   // use as needed
using sInt  = int;            // use this most
using sDInt = std::ptrdiff_t; // type for pointer diff

using sF32 = float;           // basic floating point
using sF64 = double;          // use as needed

using sBool = bool;           // use for boolean function results

/****************************************************************************/

#define sTRUE   true
#define sFALSE  false

/****************************************************************************/

template <class T> constexpr T sMin(T a, T b) 
{
    return (a < b) ? a : b;
}

template <class T> constexpr T sMax(T a, T b)              
{
    return (a > b) ? a : b;
}

template <class T> constexpr T sSign(T a) 
{
    return (a == 0) ? static_cast<T>(0) : (a > 0) ? static_cast<T>(1) : static_cast<T>(-1);
}

template <class T> constexpr T sClamp(T a, T min, T max) 
{
    return std::clamp(a, min, max);
}

template <class T> constexpr void sSwap(T &a,T &b)
{ 
    std::swap(a, b); 
}

template <class T> constexpr T sAlign(T a, T b) 
{
    return (T)((((sDInt)a) + b - 1) & (~(b - 1)));
}

template <class T> constexpr T sSquare(T a)                  {return a * a;}

/****************************************************************************/

constexpr double sPI = std::numbers::pi_v<double>;
constexpr double sPI2 = 2 * sPI;
constexpr float sPIF = std::numbers::pi_v<float>;
constexpr float  sPI2F = 2 * sPIF;
constexpr double sSQRT2 = std::numbers::sqrt2_v<double>;
constexpr float sSQRT2F = std::numbers::sqrt2_v<float>;

inline sInt sAbs(sInt i)                                  { return std::abs(i); }
inline void sSetMem(void *dd,sInt s,sInt c)               { std::memset(dd,s,c); }
inline void sCopyMem(void *dd,const void *ss,sInt c)      { std::memcpy(dd,ss,c); }
inline sInt sCmpMem(const void *dd,const void *ss,sInt c) { return std::memcmp(dd,ss,c); }

inline sF64 sFATan(sF64 f)         { return std::atan(f); }
inline sF64 sFATan2(sF64 a,sF64 b) { return std::atan2(a,b); }
inline sF64 sFCos(sF64 f)          { return std::cos(f); }
inline sF64 sFAbs(sF64 f)          { return std::fabs(f); }
inline sF64 sFLog(sF64 f)          { return std::log(f); }
inline sF64 sFLog10(sF64 f)        { return std::log10(f); }
inline sF64 sFSin(sF64 f)          { return std::sin(f); }
inline sF64 sFSqrt(sF64 f)         { return std::sqrt(f); }
inline sF64 sFTan(sF64 f)          { return std::tan(f); }

inline sF64 sFACos(sF64 f)         { return std::acos(f); }
inline sF64 sFASin(sF64 f)         { return std::asin(f); }
inline sF64 sFCosH(sF64 f)         { return std::cosh(f); }
inline sF64 sFSinH(sF64 f)         { return std::sinh(f); }
inline sF64 sFTanH(sF64 f)         { return std::tanh(f); }

inline sF64 sFInvSqrt(sF64 f)      { return 1.0/std::sqrt(f); }

inline sF64 sFFloor(sF64 f)        { return std::floor(f); }

inline sF64 sFPow(sF64 a,sF64 b)   { return std::pow(a,b); }
inline sF64 sFMod(sF64 a,sF64 b)   { return std::fmod(a,b); }
inline sF64 sFExp(sF64 f)          { return std::exp(f); }

/****************************************************************************/
/***                                                                      ***/
/***   Debugging                                                          ***/
/***                                                                      ***/
/****************************************************************************/

#define sVERIFY(x)    {assert(x);}
#define sVERIFYFALSE  {assert(false);}

/****************************************************************************/

#endif // __TP_TYPES_HPP__
