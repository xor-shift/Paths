#pragma once

#include "Maths/MatVec.hpp"

namespace Equals {

static constexpr auto error_margin = 0.0001L;

template<typename T, typename U>
requires(std::is_floating_point_v<T> &&std::is_floating_point_v<U> &&std::is_convertible_v<U, T>)
    [[nodiscard]] inline bool float_eq(T v_0, U v_1) noexcept {
    return std::abs(v_0 - v_1) <= error_margin;
}

template<Maths::Concepts::VectorExpression VE0, Maths::Concepts::VectorExpression VE1>
requires(VE0::m_vector_size == VE1::m_vector_size)
    [[nodiscard]] inline bool vector_eq(const VE0 &v_0, const VE1 &v_1) noexcept {
    for (size_t i = 0; i < VE0::m_vector_size; i++)
        if (!float_eq(v_0[i], v_1[i]))
            return false;
    return true;
}

template<Maths::Concepts::MatrixExpression ME0, Maths::Concepts::MatrixExpression ME1>
requires(ME0::m_cols == ME1::m_cols && ME0::m_rows == ME1::m_rows)
    [[nodiscard]] inline bool matrix_eq(const ME0 &m_0, const ME1 &m_1) noexcept {
    for (size_t i = 0; i < ME0::m_rows; i++)
        for (size_t j = 0; j < ME0::m_cols; j++)
            if (!float_eq(m_0.at(i, j), m_1.at(i, j)))
                return false;
    return true;
}

}
