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

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <numbers>

/****************************************************************************/
/***                                                                      ***/
/***   Basic Types and Functions                                          ***/
/***                                                                      ***/
/****************************************************************************/

// remove most in favour of built-in types

using sU8 = std::uint8_t;     // for packed arrays
using sU16 = std::uint16_t;   // for packed arrays
using sU32 = std::uint32_t;   // for packed arrays and bitfields
using sU64 = std::uint64_t;   // use as needed
using sS8 = std::int8_t;      // for packed arrays
using sS16 = std::int16_t;    // for packed arrays
using sS32 = std::int32_t;    // for packed arrays
using sS64 = std::int64_t;    // use as needed
using sInt = int;             // use this most
using sDInt = std::ptrdiff_t; // type for pointer diff

using sF32 = float;  // basic floating point
using sF64 = double; // use as needed

using sBool = bool; // use for boolean function results

/****************************************************************************/

#define sTRUE true
#define sFALSE false

/****************************************************************************/

template <class T> constexpr auto sMin(T a, T b) noexcept -> T
{
    return (a < b) ? a : b;
}

template <class T> constexpr auto sMax(T a, T b) noexcept -> T
{
    return (a > b) ? a : b;
}

template <class T> constexpr auto sSign(T a) noexcept -> T
{
    return (a == 0) ? static_cast<T>(0) : (a > 0) ? static_cast<T>(1) : static_cast<T>(-1);
}

template <class T> constexpr auto sClamp(T a, T min, T max) -> T
{
    return std::clamp(a, min, max);
}

template <class T> constexpr void sSwap(T &a, T &b) noexcept
{
    std::swap(a, b);
}

template <class T> constexpr auto sAlign(T a, T b) noexcept -> T
{
    return (T)((((sDInt)a) + b - 1) & (~(b - 1)));
}

template <class T> constexpr auto sSquare(T a) noexcept -> T
{
    return a * a;
}

/****************************************************************************/

constexpr double sPI = std::numbers::pi_v<double>;
constexpr double sPI2 = 2 * sPI;
constexpr float sPIF = std::numbers::pi_v<float>;
constexpr float sPI2F = 2 * sPIF;
constexpr double sSQRT2 = std::numbers::sqrt2_v<double>;
constexpr float sSQRT2F = std::numbers::sqrt2_v<float>;

inline auto sAbs(sInt i) noexcept -> sInt
{
    return std::abs(i);
}
inline void sSetMem(void *dd, sInt s, sInt c) noexcept
{
    std::memset(dd, s, c);
}
inline void sCopyMem(void *dd, const void *ss, sInt c) noexcept
{
    std::memcpy(dd, ss, c);
}
inline auto sCmpMem(const void *dd, const void *ss, sInt c) noexcept -> sInt
{
    return std::memcmp(dd, ss, c);
}

inline auto sFATan(sF64 f) noexcept -> sF64
{
    return std::atan(f);
}
inline auto sFATan2(sF64 a, sF64 b) noexcept -> sF64
{
    return std::atan2(a, b);
}
inline auto sFCos(sF64 f) noexcept -> sF64
{
    return std::cos(f);
}
inline auto sFAbs(sF64 f) noexcept -> sF64
{
    return std::fabs(f);
}
inline auto sFLog(sF64 f) noexcept -> sF64
{
    return std::log(f);
}
inline auto sFLog10(sF64 f) noexcept -> sF64
{
    return std::log10(f);
}
inline auto sFSin(sF64 f) noexcept -> sF64
{
    return std::sin(f);
}
inline auto sFSqrt(sF64 f) noexcept -> sF64
{
    return std::sqrt(f);
}
inline auto sFTan(sF64 f) noexcept -> sF64
{
    return std::tan(f);
}

inline auto sFACos(sF64 f) noexcept -> sF64
{
    return std::acos(f);
}
inline auto sFASin(sF64 f) noexcept -> sF64
{
    return std::asin(f);
}
inline auto sFCosH(sF64 f) noexcept -> sF64
{
    return std::cosh(f);
}
inline auto sFSinH(sF64 f) noexcept -> sF64
{
    return std::sinh(f);
}
inline auto sFTanH(sF64 f) noexcept -> sF64
{
    return std::tanh(f);
}

inline auto sFInvSqrt(sF64 f) noexcept -> sF64
{
    return 1.0 / std::sqrt(f);
}

inline auto sFFloor(sF64 f) noexcept -> sF64
{
    return std::floor(f);
}

inline auto sFPow(sF64 a, sF64 b) noexcept -> sF64
{
    return std::pow(a, b);
}
inline auto sFMod(sF64 a, sF64 b) noexcept -> sF64
{
    return std::fmod(a, b);
}
inline auto sFExp(sF64 f) noexcept -> sF64
{
    return std::exp(f);
}

/****************************************************************************/

#endif // __TP_TYPES_HPP__
