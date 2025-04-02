#pragma once

#include <array>
#include <cmath>
#include <span>
#include <type_traits>

namespace openktg::inline core
{

template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

// Simple 4x4 matrix type
template <arithmetic T> struct matrix44
{
    std::array<T, 16> data; // row-major

    matrix44() = default;
    explicit matrix44(T value)
    {
        data.fill(value);
    }
    matrix44(std::initializer_list<T> init);

    auto operator()(size_t i, size_t j) noexcept -> T &;
    auto operator()(size_t i, size_t j) const noexcept -> const T &;

    auto row(size_t i) noexcept -> std::span<T, 4>;
    auto row(size_t i) const noexcept -> std::span<const T, 4>;

    auto column(size_t j) const -> std::array<T, 4>;

    static auto identity() noexcept -> matrix44;
    static auto zero() noexcept -> matrix44;
    static auto scale(T sx, T sy, T sz) noexcept -> matrix44;
    static auto translation(T tx, T ty, T tz) noexcept -> matrix44;
    static auto rotation_z(T angle) noexcept -> matrix44;
    auto transpose() const -> matrix44;

    auto operator+=(const matrix44 &rhs) -> matrix44 &;
    auto operator-=(const matrix44 &rhs) -> matrix44 &;
    auto operator*=(T scalar) -> matrix44 &;
    auto operator/=(T scalar) -> matrix44 &;
};

template <arithmetic T> auto operator+(matrix44<T> lhs, const matrix44<T> &rhs) -> matrix44<T>
{
    return lhs += rhs;
}

template <arithmetic T> auto operator-(matrix44<T> lhs, const matrix44<T> &rhs) -> matrix44<T>
{
    return lhs -= rhs;
}

template <arithmetic T> auto operator*(const matrix44<T> &a, const matrix44<T> &b) -> matrix44<T>
{
    matrix44<T> result;
    for (size_t i = 0; i < 4; ++i)
        for (size_t j = 0; j < 4; ++j)
            result(i, j) = a(i, 0) * b(0, j) + a(i, 1) * b(1, j) + a(i, 2) * b(2, j) + a(i, 3) * b(3, j);
    return result;
}

template <arithmetic T> auto operator*(matrix44<T> m, T scalar) -> matrix44<T>
{
    return m *= scalar;
}

template <arithmetic T> auto operator*(T scalar, matrix44<T> m) -> matrix44<T>
{
    return m *= scalar;
}

template <arithmetic T> auto operator/(matrix44<T> m, T scalar) -> matrix44<T>
{
    return m /= scalar;
}

template <arithmetic T> auto operator*(const matrix44<T> &m, const std::array<T, 4> &v) -> std::array<T, 4>
{
    std::array<T, 4> result;
    for (size_t i = 0; i < 4; ++i)
        result[i] = m(i, 0) * v[0] + m(i, 1) * v[1] + m(i, 2) * v[2] + m(i, 3) * v[3];
    return result;
}
} // namespace openktg::inline core
