#pragma once

#include "vector.hpp"
#include "matrix.hpp"

namespace Maths {

namespace Impl {

template<Concepts::MatrixExpression ME, Concepts::VectorExpression VE> requires (ME::cols == VE::vectorSize)
struct MatVecProductExpr : public Impl::VectorExpr<typename ME::value_type, MatVecProductExpr<ME, VE>> {
    typedef typename ME::value_type value_type;
    static constexpr size_t vectorSize = ME::rows;

    explicit constexpr MatVecProductExpr(const ME &me, const VE &ve)
      : me(me), ve(ve) {}

    [[nodiscard]] constexpr auto operator[](size_t idx) const noexcept {
        value_type sum{0};
        for (size_t j = 0; j < VE::vectorSize; j++) sum += me.At(idx, j) * ve[j];
        return sum;
    }

  private:
    const ME me;
    const VE ve;
};

template<Concepts::VectorExpression VE>
struct VecToMatExpr : public Impl::MatrixExpr<typename VE::value_type, VecToMatExpr<VE>> {
    typedef typename VE::value_type value_type;
    static constexpr std::size_t rows = VE::vectorSize;
    static constexpr std::size_t cols = 1;

    explicit constexpr VecToMatExpr(const VE &ve)
      : ve(ve) {}

    constexpr value_type At(std::size_t i, std::size_t j) const noexcept { return ve[i]; }

  private:
    const VE ve;
};

template<Concepts::MatrixExpression ME> requires (ME::cols == 1)
struct MatToVecExpr : public Impl::VectorExpr<typename ME::value_type, MatToVecExpr<ME>> {
    typedef typename ME::value_type value_type;
    static constexpr size_t vectorSize = ME::rows;

    explicit constexpr MatToVecExpr(const ME &me)
      : me(me) {}

    constexpr value_type operator[](std::size_t i) { return me.At(i, 1); }

  private:
    const ME me;
};

}

template<Concepts::MatrixExpression ME, Concepts::VectorExpression VE>
constexpr auto operator*(const ME &me, const VE &ve) { return Impl::MatVecProductExpr(me, ve); }

template<Concepts::MatrixExpression ME, Concepts::VectorExpression VE>
constexpr auto operator*(const VE &ve, const ME &me) { return Impl::MatVecProductExpr(me, ve); }

template<Concepts::VectorExpression VE>
constexpr Matrix<typename VE::value_type, 3, 3> SSC(const VE &v) noexcept {
    return {
      0, -v[2], v[1],
      v[2], 0, -v[0],
      -v[1], v[0], 0,
    };
}

template<Concepts::VectorExpression VE>
constexpr auto VecToMat(const VE &ve) { return Impl::VecToMatExpr(ve); }

template<Concepts::MatrixExpression ME>
constexpr auto MatToVec(const ME &me) { return Impl::MatToVecExpr(me); }

}
