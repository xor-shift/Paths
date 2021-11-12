#pragma once

#include <maths/matvec.hpp>

namespace Equals {

static constexpr auto errorMargin = 0.0001L;

template<typename T, typename U>
requires (
  std::is_floating_point_v<T> &&
  std::is_floating_point_v<U> &&
  std::is_convertible_v<U, T>
)
[[nodiscard]] inline bool Float(T v0, U v1) noexcept {
    return std::abs(v0 - v1) <= errorMargin;
}

template<Maths::Concepts::VectorExpression VE0, Maths::Concepts::VectorExpression VE1>
requires (VE0::vectorSize == VE1::vectorSize)
[[nodiscard]] inline bool Vector(const VE0 &v0, const VE1 &v1) noexcept {
    for (size_t i = 0; i < VE0::vectorSize; i++)
        if (!Float(v0[i], v1[i])) return false;
    return true;
}

template<Maths::Concepts::MatrixExpression ME0, Maths::Concepts::MatrixExpression ME1>
requires (
  ME0::cols == ME1::cols &&
  ME0::rows == ME1::rows
)
[[nodiscard]] inline bool Matrix(const ME0 &m0, const ME1 &m1) noexcept {
    for (size_t i = 0; i < ME0::rows; i++)
        for (size_t j = 0; j < ME0::cols; j++)
            if (!Float(m0.At(i, j), m1.At(i, j))) return false;
    return true;
}

}
