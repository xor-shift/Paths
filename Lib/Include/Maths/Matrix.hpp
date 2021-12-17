#pragma once

#include <cmath>
#include <fmt/format.h>

namespace Maths {

namespace Concepts {

template<typename T>
concept MatrixExpression = requires(const T &me, size_t i, size_t j) {
    { T::m_rows } -> std::convertible_to<size_t>;
    { T::m_cols } -> std::convertible_to<size_t>;
    { me.at(i, j) } -> std::convertible_to<typename T::value_type>;
};

}

namespace Impl {

template<typename T, typename E> struct MatrixExpr {
    typedef T value_type;
    static constexpr size_t m_rows = E::m_rows, m_cols = E::m_cols;

    [[nodiscard]] constexpr auto at(size_t i, size_t j) const noexcept {
        return static_cast<const E &>(*this).at(i, j);
    }
};

template<Concepts::MatrixExpression E0, Concepts::MatrixExpression E1, typename Op>
requires(E0::m_rows == E1::m_rows && E0::m_cols == E1::m_cols) struct MatrixBinaryExpr
    : public MatrixExpr<typename E0::value_type, MatrixBinaryExpr<E0, E1, Op>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t m_rows = E0::m_rows, m_cols = E0::m_cols;

    constexpr explicit MatrixBinaryExpr(const E0 &e_0, const E1 &e_1) noexcept(
        std::is_nothrow_default_constructible_v<Op>)
        : m_e_0(e_0)
        , m_e_1(e_1)
        , m_op({}) { }

    constexpr explicit MatrixBinaryExpr(const E0 &e_0, const E1 &e_1, const Op &op) noexcept(
        std::is_nothrow_copy_constructible_v<Op>)
        : m_e_0(e_0)
        , m_e_1(e_1)
        , m_op(op) { }

    [[nodiscard]] constexpr value_type at(size_t i, size_t j) const noexcept {
        return m_op(m_e_0.at(i, j), m_e_1.at(i, j));
    }

private:
    const E0 m_e_0;
    const E1 m_e_1;
    const Op m_op;
};

template<Concepts::MatrixExpression E0, typename Op>
struct MatrixUnaryExpr : public MatrixExpr<typename E0::value_type, MatrixUnaryExpr<E0, Op>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t m_rows = E0::m_rows, m_cols = E0::m_cols;

    constexpr explicit MatrixUnaryExpr(const E0 &e_0) noexcept(std::is_nothrow_default_constructible_v<Op>)
        : m_e_0(e_0)
        , m_op({}) { }

    constexpr explicit MatrixUnaryExpr(const E0 &e_0, const Op &op) noexcept(std::is_nothrow_copy_constructible_v<Op>)
        : m_e_0(e_0)
        , m_op(op) { }

    [[nodiscard]] constexpr value_type at(size_t i, size_t j) const noexcept { return m_op(m_e_0.at(i, j)); }

private:
    const E0 m_e_0;
    const Op m_op;
};

template<Concepts::MatrixExpression E0, Concepts::MatrixExpression E1>
requires(E0::m_cols == E1::m_rows) struct MatrixMultExpr
    : public MatrixExpr<typename E0::value_type, MatrixMultExpr<E0, E1>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t m_rows = E0::m_rows, m_cols = E1::m_cols;

    constexpr explicit MatrixMultExpr(const E0 &e_0, const E1 &e_1) noexcept
        : m_e_0(e_0)
        , m_e_1(e_1) { }

    [[nodiscard]] constexpr value_type at(size_t i, size_t j) const noexcept {
        value_type sum(0);

        for (size_t k = 0; k < E0::m_cols; k++) {
            sum += m_e_0.at(i, k) * m_e_1.at(k, j);
        }

        return sum;
    }

private:
    const E0 m_e_0;
    const E1 &m_e_1;
};

template<Concepts::MatrixExpression E0>
struct MatrixTransposeExpr : public MatrixExpr<typename E0::value_type, MatrixTransposeExpr<E0>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t m_rows = E0::m_cols, m_cols = E0::m_rows;

    constexpr explicit MatrixTransposeExpr(const E0 &e_0)
        : m_e_0(e_0) { }

    [[nodiscard]] constexpr value_type at(size_t i, size_t j) const noexcept { return m_e_0.at(j, i); }

private:
    const E0 m_e_0;
};

template<Concepts::MatrixExpression E0, std::size_t si, std::size_t sj>
struct MatrixRowSwitchExpr : public MatrixExpr<typename E0::value_type, MatrixRowSwitchExpr<E0, si, sj>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t m_rows = E0::m_cols, m_cols = E0::m_rows;

    constexpr explicit MatrixRowSwitchExpr(const E0 &e_0)
        : m_e_0(e_0) { }

    [[nodiscard]] constexpr value_type at(size_t i, size_t j) const noexcept {
        if (i == si)
            return m_e_0.at(sj, j);
        else if (i == sj)
            return m_e_0.at(si, j);
        else
            return m_e_0.at(i, j);
    }

private:
    const E0 m_e_0;
};

template<Concepts::MatrixExpression E0, std::size_t mi, typename E0::value_type m>
struct MatrixRowScalarMultExpr : public MatrixExpr<typename E0::value_type, MatrixRowScalarMultExpr<E0, mi, m>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t m_rows = E0::m_cols, m_cols = E0::m_rows;

    constexpr explicit MatrixRowScalarMultExpr(const E0 &e_0)
        : m_e_0(e_0) { }

    [[nodiscard]] constexpr value_type at(size_t i, size_t j) const noexcept {
        if (i == mi)
            return m_e_0.at(i, j) * m;
        else
            return m_e_0.at(i, j);
    }

private:
    const E0 m_e_0;
};

template<Concepts::MatrixExpression E0, std::size_t mi, std::size_t mj, typename E0::value_type m>
struct MatrixRowAdditionExpr : public MatrixExpr<typename E0::value_type, MatrixRowAdditionExpr<E0, mi, mj, m>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t m_rows = E0::m_cols, m_cols = E0::m_rows;

    constexpr explicit MatrixRowAdditionExpr(const E0 &e_0)
        : m_e_0(e_0) { }

    [[nodiscard]] constexpr value_type at(size_t i, size_t j) const noexcept {
        if (i == mi)
            return m_e_0.at(i, j) + m * m_e_0.at(mj, j);
        else
            return m_e_0.at(i, j);
    }

private:
    const E0 m_e_0;
};

}

template<typename T, size_t M, size_t N> struct Matrix : public Impl::MatrixExpr<T, Matrix<T, M, N>> {
    typedef T value_type;
    typedef value_type array_type[M][N];
    // typedef value_type flat_type[M * N];
    typedef std::array<value_type, M * N> flat_type;

    static constexpr size_t m_rows = M, m_cols = N;

    flat_type m_impl;

    Matrix()
        : m_impl({}) { }

    ~Matrix() = default;

    template<Concepts::MatrixExpression E>
    requires(E::m_cols == m_cols, E::m_rows == m_rows) constexpr Matrix(const E &expr) noexcept {
        for (size_t j = 0; j < M; j++)
            for (size_t i = 0; i < N; i++)
                m_impl[i + j * N] = expr.at(j, i);
    }

    template<typename... Ts>
    constexpr Matrix(Ts... vs) noexcept
        : m_impl({ static_cast<T>(vs)... }) { }

    constexpr value_type at(size_t i, size_t j) const noexcept { return m_impl[j + i * N]; }

    constexpr value_type &at(size_t i, size_t j) noexcept { return m_impl[j + i * N]; }

    static Matrix rotation(double theta) requires(M == 2 && N == 2) {
        T sin_theta = std::sin(theta), cos_theta = std::cos(theta);

        Matrix<T, 2, 2> mat { { cos_theta, -sin_theta }, { sin_theta, cos_theta } };

        return mat;
    }

    static Matrix rotation(double yaw, double pitch, double roll) requires(M == 3 && N == 3) {
        T sa = std::sin(roll), ca = std::cos(roll), sb = std::sin(yaw), cb = std::cos(yaw), sg = std::sin(pitch),
          cg = std::cos(pitch);

        return
          Matrix<T, 3, 3>{
                            ca, -sa, 0,
                            sa, ca, 0,
                            0, 0, 1,
                          } *
          Matrix<T, 3, 3>{
                            cb, 0, sb,
                            0, 1, 0,
                            -sb, 0, cb,
                          } *
          Matrix<T, 3, 3>{
                            1, 0, 0,
                            0, cg, -sg,
                            0, sg, cg,
                          };
    }
};

template<typename T, size_t n> constexpr Matrix<T, n, n> identity_matrix() noexcept {
    Matrix<T, n, n> mat {};

    for (size_t i = 0; i < n; i++)
        mat.at(i, i) = 1;

    return mat;
}

//////////////////////////////////////
//// Element-wise unary operators ////
//////////////////////////////////////

template<Concepts::MatrixExpression E0, typename U>
requires std::is_convertible_v<U, typename E0::value_type>
constexpr auto operator*(const E0 &e_0, const U s) noexcept {
    return Impl::MatrixUnaryExpr(e_0, [s](auto v) { return v * s; });
}

template<Concepts::MatrixExpression E0, typename U>
requires std::is_convertible_v<U, typename E0::value_type>
constexpr auto operator/(const E0 &e_0, const U s) noexcept {
    return Impl::MatrixUnaryExpr(e_0, [s](auto v) { return v / s; });
}

template<Concepts::MatrixExpression E0> constexpr auto operator-(const E0 &e_0) noexcept {
    return Impl::MatrixUnaryExpr(e_0, [](auto v) { return -v; });
}

template<Concepts::MatrixExpression E0> constexpr auto reciprocal(const E0 &e_0) noexcept {
    return Impl::MatrixUnaryExpr(e_0, [](auto v) { return 1. / v; });
}

/*template<Concepts::MatrixExpression E0, typename U>
requires std::is_convertible_v<U, typename E0::value_type>
constexpr auto Pow(const E0 &e0, const U s) noexcept { return Impl::MatrixUnaryExpr(e0, [s](auto v) { return std::pow(v,
s); }); }*/

template<Concepts::MatrixExpression E0> constexpr auto transpose(const E0 &e_0) noexcept {
    return Impl::MatrixTransposeExpr(e_0);
}

///////////////////////////////////////
//// Element-wise binary operators ////
///////////////////////////////////////

template<Concepts::MatrixExpression E0, Concepts::MatrixExpression E1>
constexpr auto operator+(const E0 &e_0, const E1 &e_1) noexcept {
    return Impl::MatrixBinaryExpr(e_0, e_1, std::plus {});
}

template<Concepts::MatrixExpression E0, Concepts::MatrixExpression E1>
constexpr auto operator-(const E0 &e_0, const E1 &e_1) noexcept {
    return Impl::MatrixBinaryExpr(e_0, e_1, std::minus {});
}

//// other /////
template<Concepts::MatrixExpression E0, Concepts::MatrixExpression E1>
constexpr auto operator*(const E0 &e_0, const E1 &e_1) noexcept {
    return Impl::MatrixMultExpr(e_0, e_1);
}

}

template<typename ME>
requires Maths::Concepts::MatrixExpression<ME>
struct fmt::formatter<ME, char, std::enable_if_t<Maths::Concepts::MatrixExpression<ME>, void>> {
    constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<typename FormatContext> constexpr auto format(const ME &vec, FormatContext &ctx) -> decltype(ctx.out()) {
        auto o = ctx.out();

        o = fmt::format_to(o, "[");
        for (size_t j = 0; j < ME::m_rows; j++) {
            o = fmt::format_to(o, "[");
            for (size_t i = 0; i < ME::m_cols; i++) {
                o = fmt::format_to(o, "{}", vec.at(j, i));
                if (i != ME::m_cols - 1)
                    o = fmt::format_to(o, ",");
            }
            o = fmt::format_to(o, "]");
            if (j != ME::m_rows - 1)
                o = fmt::format_to(o, ",");
        }
        o = fmt::format_to(o, "]");

        return o;
    }
};
