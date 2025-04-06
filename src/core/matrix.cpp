#include <algorithm>
#include <cassert>
#include <cmath>

#include <openktg/core/matrix.h>

namespace openktg::inline core
{

template <arithmetic T> matrix44<T>::matrix44(std::initializer_list<T> init)
{
    std::copy_n(init.begin(), std::min(init.size(), data.size()), data.begin());
}

template <arithmetic T> auto matrix44<T>::operator()(size_t i, size_t j) noexcept -> T &
{
    assert(i < 4 && j < 4);
    return data[i * 4 + j];
}

template <arithmetic T> auto matrix44<T>::operator()(size_t i, size_t j) const noexcept -> const T &
{
    assert(i < 4 && j < 4);
    return data[i * 4 + j];
}

template <arithmetic T> auto matrix44<T>::row(size_t i) noexcept -> std::span<T, 4>
{
    assert(i < 4);
    return std::span<T, 4>(&data[i * 4], 4);
}

template <arithmetic T> auto matrix44<T>::row(size_t i) const noexcept -> std::span<const T, 4>
{
    assert(i < 4);
    return std::span<const T, 4>(&data[i * 4], 4);
}

template <arithmetic T> auto matrix44<T>::column(size_t j) const -> std::array<T, 4>
{
    assert(j < 4);
    return {data[j], data[j + 4], data[j + 8], data[j + 12]};
}

template <arithmetic T> auto matrix44<T>::identity() noexcept -> matrix44<T>
{
    matrix44<T> m(0);
    m(0, 0) = m(1, 1) = m(2, 2) = m(3, 3) = 1;
    return m;
}

template <arithmetic T> auto matrix44<T>::zero() noexcept -> matrix44<T>
{
    return matrix44<T>(0);
}

template <arithmetic T> auto matrix44<T>::scale(T sx, T sy, T sz) noexcept -> matrix44<T>
{
    auto m = matrix44<T>::identity();
    m(0, 0) = sx;
    m(1, 1) = sy;
    m(2, 2) = sz;
    return m;
}
template <arithmetic T> auto matrix44<T>::translation(T tx, T ty, T tz) noexcept -> matrix44<T>
{
    auto m = matrix44<T>::identity();
    m(3, 0) = tx;
    m(3, 1) = ty;
    m(3, 2) = tz;
    return m;
}
template <arithmetic T> auto matrix44<T>::rotation_z(T angle) noexcept -> matrix44<T>
{
    T s = std::sin(angle);
    T c = std::cos(angle);
    auto m = matrix44<T>::identity();
    m(0, 0) = c;
    m(0, 1) = s;
    m(1, 0) = -s;
    m(1, 1) = c;
    return m;
}

template <arithmetic T> auto matrix44<T>::transpose() const -> matrix44<T>
{
    matrix44<T> result;
    for (size_t i = 0; i < 4; ++i)
        for (size_t j = 0; j < 4; ++j)
            result(j, i) = (*this)(i, j);
    return result;
}

template <arithmetic T> auto matrix44<T>::operator+=(const matrix44<T> &rhs) -> matrix44<T> &
{
    for (size_t i = 0; i < 16; ++i)
        data[i] += rhs.data[i];
    return *this;
}

template <arithmetic T> auto matrix44<T>::operator-=(const matrix44<T> &rhs) -> matrix44<T> &
{
    for (size_t i = 0; i < 16; ++i)
        data[i] -= rhs.data[i];
    return *this;
}

template <arithmetic T> auto matrix44<T>::operator*=(T scalar) -> matrix44<T> &
{
    for (auto &val : data)
        val *= scalar;
    return *this;
}

template <arithmetic T> auto matrix44<T>::operator/=(T scalar) -> matrix44<T> &
{
    for (auto &val : data)
        val /= scalar;
    return *this;
}

} // namespace openktg::inline core

template struct openktg::matrix44<float>;
template struct openktg::matrix44<int>;