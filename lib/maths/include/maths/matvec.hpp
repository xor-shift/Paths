#pragma once

#include "vector.hpp"
#include "matrix.hpp"

namespace Math {

namespace Impl {

template<typename E0, typename U, size_t M>
struct MatVecProductExpr : public Impl::VectorExpr<typename E0::value_type, MatVecProductExpr<E0, U, M>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t arraySize = M;

    const E0 &e0;
    const Matrix <U, M, E0::arraySize> &mat;

    constexpr MatVecProductExpr(const E0 &e0, const Matrix <U, M, E0::arraySize> &mat)
      : e0(e0)
        , mat(mat) {}

    [[nodiscard]] constexpr size_t size() const noexcept { return arraySize; }

    [[nodiscard]] constexpr value_type operator[](size_t idx) const noexcept {
        value_type sum{0};
        for (size_t j = 0; j < E0::arraySize; j++) sum += mat.data[idx][j] * e0[j];
        return sum;
    }
};

}

template<typename E0, typename U, size_t M>
constexpr inline Impl::MatVecProductExpr<E0, U, M> operator*(const E0 &e0, const Matrix <U, M, E0::arraySize> &mat) { return {e0, mat}; }

template<typename E0, typename U, size_t M>
constexpr inline Impl::MatVecProductExpr<E0, U, M> operator*(const Matrix <U, M, E0::arraySize> &mat, const E0 &e0) { return {e0, mat}; }

}
