#pragma once

#include <cmath>
#include <fmt/format.h>

namespace Maths {

namespace Concepts {

template<typename T>
concept MatrixExpression = requires(const T &me, size_t i, size_t j) {
    { T::rows } -> std::convertible_to<size_t>;
    { T::cols } -> std::convertible_to<size_t>;
    { me.At(i, j) } -> std::convertible_to<typename T::value_type>;
};

}

namespace Detail::Matrix {



}

namespace Impl {

template<typename T, typename E>
struct MatrixExpr {
    typedef T value_type;
    static constexpr size_t
      rows = E::rows,
      cols = E::cols;

    [[nodiscard]] constexpr auto At(size_t i, size_t j) const noexcept {
        return static_cast<const E &>(*this).At(i, j);
    }
};

template<Concepts::MatrixExpression E0, Concepts::MatrixExpression E1, typename Op> requires (E0::rows == E1::rows && E0::cols == E1::cols)
struct MatrixBinaryExpr : public MatrixExpr<typename E0::value_type, MatrixBinaryExpr<E0, E1, Op>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t
      rows = E0::rows,
      cols = E0::cols;

    constexpr explicit MatrixBinaryExpr(const E0 &e0, const E1 &e1) noexcept(std::is_nothrow_default_constructible_v<Op>)
      : e0(e0), e1(e1), op({}) {}

    constexpr explicit MatrixBinaryExpr(const E0 &e0, const E1 &e1, const Op &op) noexcept(std::is_nothrow_copy_constructible_v<Op>)
      : e0(e0), e1(e1), op(op) {}

    [[nodiscard]] constexpr value_type At(size_t i, size_t j) const noexcept {
        return op(e0.At(i, j), e1.At(i, j));
    }

  private:
    const E0 e0;
    const E1 e1;
    const Op op;
};

template<Concepts::MatrixExpression E0, typename Op>
struct MatrixUnaryExpr : public MatrixExpr<typename E0::value_type, MatrixUnaryExpr<E0, Op>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t
      rows = E0::rows,
      cols = E0::cols;

    constexpr explicit MatrixUnaryExpr(const E0 &e0) noexcept(std::is_nothrow_default_constructible_v<Op>)
      : e0(e0), op({}) {}

    constexpr explicit MatrixUnaryExpr(const E0 &e0, const Op &op) noexcept(std::is_nothrow_copy_constructible_v<Op>)
      : e0(e0), op(op) {}

    [[nodiscard]] constexpr value_type At(size_t i, size_t j) const noexcept {
        return op(e0.At(i, j));
    }

  private:
    const E0 e0;
    const Op op;
};

template<Concepts::MatrixExpression E0, Concepts::MatrixExpression E1> requires (E0::cols == E1::rows)
struct MatrixMultExpr : public MatrixExpr<typename E0::value_type, MatrixMultExpr<E0, E1>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t
      rows = E0::rows,
      cols = E1::cols;

    constexpr explicit MatrixMultExpr(const E0 &e0, const E1 &e1) noexcept
      : e0(e0), e1(e1) {}

    [[nodiscard]] constexpr value_type At(size_t i, size_t j) const noexcept {
        value_type sum(0);

        for (size_t k = 0; k < E0::cols; k++) {
            sum += e0.At(i, k) * e1.At(k, j);
        }

        return sum;
    }

  private:
    const E0 e0;
    const E1 &e1;
};

template<Concepts::MatrixExpression E0>
struct MatrixTransposeExpr : public MatrixExpr<typename E0::value_type, MatrixTransposeExpr<E0>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t
      rows = E0::cols,
      cols = E0::rows;

    constexpr explicit MatrixTransposeExpr(const E0 &e0)
      : e0(e0) {}

    [[nodiscard]] constexpr value_type At(size_t i, size_t j) const noexcept { return e0.At(j, i); }

  private:
    const E0 e0;
};

template<Concepts::MatrixExpression E0, std::size_t si, std::size_t sj>
struct MatrixRowSwitchExpr : public MatrixExpr<typename E0::value_type, MatrixRowSwitchExpr<E0, si, sj>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t
      rows = E0::cols,
      cols = E0::rows;

    constexpr explicit MatrixRowSwitchExpr(const E0 &e0)
      : e0(e0) {}

    [[nodiscard]] constexpr value_type At(size_t i, size_t j) const noexcept {
        if (i == si) return e0.At(sj, j);
        else if (i == sj) return e0.At(si, j);
        else return e0.At(i, j);
    }

  private:
    const E0 e0;
};

template<Concepts::MatrixExpression E0, std::size_t mi, typename E0::value_type m>
struct MatrixRowScalarMultExpr : public MatrixExpr<typename E0::value_type, MatrixRowScalarMultExpr<E0, mi, m>>{
    typedef typename E0::value_type value_type;
    static constexpr size_t
      rows = E0::cols,
      cols = E0::rows;

    constexpr explicit MatrixRowScalarMultExpr(const E0 &e0)
      : e0(e0) {}

    [[nodiscard]] constexpr value_type At(size_t i, size_t j) const noexcept {
        if (i == mi) return e0.At(i, j) * m;
        else return e0.At(i, j);
    }

  private:
    const E0 e0;
};

template<Concepts::MatrixExpression E0, std::size_t mi, std::size_t mj, typename E0::value_type m>
struct MatrixRowAdditionExpr : public MatrixExpr<typename E0::value_type, MatrixRowAdditionExpr<E0, mi, mj, m>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t
      rows = E0::cols,
      cols = E0::rows;

    constexpr explicit MatrixRowAdditionExpr(const E0 &e0)
      : e0(e0) {}

    [[nodiscard]] constexpr value_type At(size_t i, size_t j) const noexcept {
        if (i == mi) return e0.At(i, j) + m * e0.At(mj, j);
        else return e0.At(i, j);
    }

  private:
    const E0 e0;
};

}

template<typename T, size_t M, size_t N>
struct Matrix : public Impl::MatrixExpr<T, Matrix<T, M, N>> {
    typedef T value_type;
    typedef value_type array_type[M][N];
    //typedef value_type flat_type[M * N];
    typedef std::array<value_type, M * N> flat_type;

    static constexpr size_t
      rows = M,
      cols = N;

    flat_type impl;

    Matrix()
      : impl({}) {}

    ~Matrix() = default;

    template<Concepts::MatrixExpression E>
    requires (E::cols == cols, E::rows == rows)
    constexpr Matrix(const E &expr) noexcept {
        for (size_t j = 0; j < M; j++)
            for (size_t i = 0; i < N; i++)
                impl[i + j * N] = expr.At(j, i);
    }

    template<typename... Ts>
    constexpr Matrix(Ts ...vs) noexcept : impl({static_cast<T>(vs)...}) {}

    constexpr value_type At(size_t i, size_t j) const noexcept { return impl[j + i * N]; }

    constexpr value_type &At(size_t i, size_t j) noexcept { return impl[j + i * N]; }

    static Matrix Rotation(double theta) requires (M == 2 && N == 2) {
        T sinTheta = std::sin(theta),
          cosTheta = std::cos(theta);

        Matrix<T, 2, 2> mat{
          {cosTheta, -sinTheta},
          {sinTheta, cosTheta}
        };

        return mat;
    }

    static Matrix Rotation(double yaw, double pitch, double roll) requires (M == 3 && N == 3) {
        T
          sa = std::sin(roll),
          ca = std::cos(roll),
          sb = std::sin(yaw),
          cb = std::cos(yaw),
          sg = std::sin(pitch),
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

template<typename T, size_t n>
constexpr Matrix<T, n, n> Identity() noexcept {
    Matrix<T, n, n> mat{};

    for (size_t i = 0; i < n; i++) mat.At(i, i) = 1;

    return mat;
}

//////////////////////////////////////
//// Element-wise unary operators ////
//////////////////////////////////////

template<Concepts::MatrixExpression E0, typename U>
requires std::is_convertible_v<U, typename E0::value_type>
constexpr auto operator*(const E0 &e0, const U s) noexcept {
    return Impl::MatrixUnaryExpr(e0, [s](auto v) {
        return v * s;
    });
}

template<Concepts::MatrixExpression E0, typename U>
requires std::is_convertible_v<U, typename E0::value_type>
constexpr auto operator/(const E0 &e0, const U s) noexcept {
    return Impl::MatrixUnaryExpr(e0, [s](auto v) {
        return v / s;
    });
}

template<Concepts::MatrixExpression E0>
constexpr auto operator-(const E0 &e0) noexcept { return Impl::MatrixUnaryExpr(e0, [](auto v) { return -v; }); }

template<Concepts::MatrixExpression E0>
constexpr auto Reciprocal(const E0 &e0) noexcept {
    return Impl::MatrixUnaryExpr(e0, [](auto v) { return 1. / v; });
}

/*template<Concepts::MatrixExpression E0, typename U>
requires std::is_convertible_v<U, typename E0::value_type>
constexpr auto Pow(const E0 &e0, const U s) noexcept { return Impl::MatrixUnaryExpr(e0, [s](auto v) { return std::pow(v, s); }); }*/

template<Concepts::MatrixExpression E0>
constexpr auto Transpose(const E0 &e0) noexcept { return Impl::MatrixTransposeExpr(e0); }

///////////////////////////////////////
//// Element-wise binary operators ////
///////////////////////////////////////

template<Concepts::MatrixExpression E0, Concepts::MatrixExpression E1>
constexpr auto operator+(const E0 &e0, const E1 &e1) noexcept {
    return Impl::MatrixBinaryExpr(e0, e1, std::plus{});
}

template<Concepts::MatrixExpression E0, Concepts::MatrixExpression E1>
constexpr auto operator-(const E0 &e0, const E1 &e1) noexcept {
    return Impl::MatrixBinaryExpr(e0, e1, std::minus{});
}

//// other /////
template<Concepts::MatrixExpression E0, Concepts::MatrixExpression E1>
constexpr auto operator*(const E0 &e0, const E1 &e1) noexcept { return Impl::MatrixMultExpr(e0, e1); }

}

template<typename ME>
requires Maths::Concepts::MatrixExpression<ME>
struct fmt::formatter<ME, char, std::enable_if_t<Maths::Concepts::MatrixExpression<ME>, void>> {
    constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<typename FormatContext>
    constexpr auto format(const ME &vec, FormatContext &ctx) -> decltype(ctx.out()) {
        auto o = ctx.out();

        o = fmt::format_to(o, "[");
        for (size_t j = 0; j < ME::rows; j++) {
            o = fmt::format_to(o, "[");
            for (size_t i = 0; i < ME::cols; i++) {
                o = fmt::format_to(o, "{}", vec.At(j, i));
                if (i != ME::cols - 1) o = fmt::format_to(o, ",");
            }
            o = fmt::format_to(o, "]");
            if (j != ME::rows - 1) o = fmt::format_to(o, ",");
        }
        o = fmt::format_to(o, "]");

        return o;
    }
};
