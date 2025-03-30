#pragma once

#include <array>

#include <concepts>
#include <openktg/pixel.h>
#include <openktg/types.h>
#include <type_traits>

namespace openktg
{
template <typename T>
    requires std::is_arithmetic_v<T>
struct matrix44
{
    std::array<T, 16> data;

    auto get(size_t i, size_t j) -> T &
    {
        return data[4 * i + j];
    }
    [[nodiscard]] auto get(size_t i, size_t j) const -> const T &
    {
        return data[4 * i + j];
    }
};
} // namespace openktg

// Simple 4x4 matrix type
using Matrix44 = sF32[4][4];

// 4x4 matrix multiply
static void MatMult(openktg::matrix44<float> &dest, const openktg::matrix44<float> &a, const openktg::matrix44<float> &b)
{
    for (sInt i = 0; i < 4; i++)
        for (sInt j = 0; j < 4; j++)
            dest.get(i, j) = a.get(i, 0) * b.get(0, j) + a.get(i, 1) * b.get(1, j) + a.get(i, 2) * b.get(2, j) + a.get(i, 3) * b.get(3, j);
}

// Create a scaling matrix
static void MatScale(openktg::matrix44<float> &dest, sF32 sx, sF32 sy, sF32 sz)
{
    dest.data.fill(0);
    dest.get(0, 0) = sx;
    dest.get(1, 1) = sy;
    dest.get(2, 2) = sz;
    dest.get(3, 3) = 1.0f;
}

// Create a translation matrix
static void MatTranslate(openktg::matrix44<float> &dest, sF32 tx, sF32 ty, sF32 tz)
{
    MatScale(dest, 1.0f, 1.0f, 1.0f);
    dest.get(3, 0) = tx;
    dest.get(3, 1) = ty;
    dest.get(3, 2) = tz;
}

// Create a z-axis rotation matrix
static void MatRotateZ(openktg::matrix44<float> &dest, sF32 angle)
{
    sF32 s = sFSin(angle);
    sF32 c = sFCos(angle);

    MatScale(dest, 1.0f, 1.0f, 1.0f);
    dest.get(0, 0) = c;
    dest.get(0, 1) = s;
    dest.get(1, 0) = -s;
    dest.get(1, 1) = c;
}