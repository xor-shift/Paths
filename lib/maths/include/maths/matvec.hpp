#pragma once

#include "vector.hpp"
#include "matrix.hpp"

namespace Math {

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
    const ME &me;
    const VE &ve;
};

}

template<Concepts::MatrixExpression ME, Concepts::VectorExpression VE>
constexpr inline auto operator*(const ME &me, const VE &ve) { return Impl::MatVecProductExpr(me, ve); }

template<Concepts::MatrixExpression ME, Concepts::VectorExpression VE>
constexpr inline auto operator*(const VE &ve, const ME &me) { return Impl::MatVecProductExpr(me, ve); }

template<Concepts::VectorExpression VE>
constexpr inline Matrix<typename VE::value_type, 3, 3> SSC(const VE &v) noexcept {
    return {{
        0, -v[2], v[1],
        v[2], 0, -v[0],
        -v[1], v[0], 0,
    }};
}

}
