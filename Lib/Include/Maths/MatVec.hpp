#pragma once

#include "Matrix.hpp"
#include "Vector.hpp"

namespace Maths {

namespace Impl {

template<Concepts::MatrixExpression ME, Concepts::VectorExpression VE>
requires(ME::m_cols == VE::m_vector_size) struct MatVecProductExpr
    : public Impl::VectorExpr<typename ME::value_type, MatVecProductExpr<ME, VE>> {
    typedef typename ME::value_type value_type;
    static constexpr size_t m_vector_size = ME::m_rows;

    explicit constexpr MatVecProductExpr(const ME &me, const VE &ve)
        : m_me(me)
        , m_ve(ve) { }

    [[nodiscard]] constexpr auto operator[](size_t idx) const noexcept {
        value_type sum { 0 };
        for (size_t j = 0; j < VE::m_vector_size; j++)
            sum += m_me.at(idx, j) * m_ve[j];
        return sum;
    }

private:
    const ME m_me;
    const VE m_ve;
};

template<Concepts::VectorExpression VE>
struct VecToMatExpr : public Impl::MatrixExpr<typename VE::value_type, VecToMatExpr<VE>> {
    typedef typename VE::value_type value_type;
    static constexpr std::size_t m_rows = VE::m_vector_size;
    static constexpr std::size_t m_cols = 1;

    explicit constexpr VecToMatExpr(const VE &ve)
        : m_ve(ve) { }

    constexpr value_type at(std::size_t i, std::size_t j) const noexcept { return m_ve[i]; }

private:
    const VE m_ve;
};

template<Concepts::MatrixExpression ME>
requires(ME::cols == 1) struct MatToVecExpr : public Impl::VectorExpr<typename ME::value_type, MatToVecExpr<ME>> {
    typedef typename ME::value_type value_type;
    static constexpr size_t m_vector_size = ME::m_rows;

    explicit constexpr MatToVecExpr(const ME &me)
        : m_me(me) { }

    constexpr value_type operator[](std::size_t i) { return m_me.at(i, 1); }

private:
    const ME m_me;
};

}

template<Concepts::MatrixExpression ME, Concepts::VectorExpression VE>
constexpr auto operator*(const ME &me, const VE &ve) {
    return Impl::MatVecProductExpr(me, ve);
}

template<Concepts::MatrixExpression ME, Concepts::VectorExpression VE>
constexpr auto operator*(const VE &ve, const ME &me) {
    return Impl::MatVecProductExpr(me, ve);
}

template<Concepts::VectorExpression VE> constexpr Matrix<typename VE::value_type, 3, 3> ssc(const VE &v) noexcept {
    return {
        0,
        -v[2],
        v[1],
        v[2],
        0,
        -v[0],
        -v[1],
        v[0],
        0,
    };
}

template<Concepts::VectorExpression VE> constexpr auto vec_to_mat(const VE &ve) { return Impl::VecToMatExpr(ve); }

template<Concepts::MatrixExpression ME> constexpr auto mat_to_vec(const ME &me) { return Impl::MatToVecExpr(me); }

}
