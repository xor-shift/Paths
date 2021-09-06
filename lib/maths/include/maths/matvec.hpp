#pragma once

#include "vector.hpp"
#include "matrix.hpp"

namespace Math::Ops::MatVec {

template<typename T, typename U, size_t N, size_t M>
constexpr Vector<T, M> MatVecMult(const Matrix<T, M, N> &mat, const Vector<U, N> &vec) noexcept {
    Vector<T, M> v{0};

    for (size_t i = 0; i < M; i++) {
        T sum = 0.;
        for (size_t j = 0; j < N; j++) {
            sum += mat.data[i][j] * vec[j];
        }
        v[i] = sum;
    }

    return v;
}

template<typename T, typename U, size_t N, size_t M>
constexpr Vector<T, M> operator*(const Matrix<T, M, N> &mat, const Vector<U, N> &vec) noexcept {
    Vector<T, M> v{0};

    for (size_t i = 0; i < M; i++) {
        T sum = 0.;
        for (size_t j = 0; j < N; j++) {
            sum += mat.data[i][j] * vec[j];
        }
        v[i] = sum;
    }

    return v;
}

}
