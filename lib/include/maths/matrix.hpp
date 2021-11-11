#pragma once

#include <cmath>

namespace Math {

namespace Concepts {

template<typename T>
concept MatrixExpression = requires(const T &me, size_t j, size_t i) {
    { T::rows } -> std::convertible_to<size_t>;
    { T::cols } -> std::convertible_to<size_t>;
    { me.At(j, i) } -> std::convertible_to<typename T::value_type>;
};

}

namespace Impl {

template<typename T, typename E>
struct MatrixExpr {
    typedef T value_type;
    static constexpr size_t
      rows = E::rows,
      cols = E::cols;

    [[nodiscard]] constexpr auto At(size_t m, size_t n) const noexcept { return static_cast<const E &>(*this).At(m, n); }
};

template<Concepts::MatrixExpression E0, Concepts::MatrixExpression E1, typename Op> requires (E0::rows == E1::rows && E0::cols == E1::cols)
struct MatrixBinaryExpr : public MatrixExpr<typename E0::value_type, MatrixBinaryExpr<E0, E1, Op>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t
      rows = E0::rows,
      cols = E0::cols;

    explicit constexpr MatrixBinaryExpr(const E0 &e0, const E1 &e1) requires std::is_nothrow_default_constructible_v<Op>
      : e0(e0), e1(e1), op({}) {}

    explicit constexpr MatrixBinaryExpr(const E0 &e0, const E1 &e1, const Op &op) requires std::is_nothrow_copy_constructible_v<Op>
      : e0(e0), e1(e1), op(op) {}

    [[nodiscard]] constexpr value_type At(size_t m, size_t n) const noexcept {
        return op(e0.At(m, n), e1.At(m, n));
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

    explicit constexpr MatrixUnaryExpr(const E0 &e0) requires std::is_nothrow_default_constructible_v<Op>
      : e0(e0), op({}) {}

    explicit constexpr MatrixUnaryExpr(const E0 &e0, const Op &op) requires std::is_nothrow_copy_constructible_v<Op>
      : e0(e0), op(op) {}

    [[nodiscard]] constexpr value_type At(size_t m, size_t n) const noexcept {
        return op(e0.At(m, n));
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

    explicit constexpr MatrixMultExpr(const E0 &e0, const E1 &e1)
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
struct MatrixTransposeExpr {
    typedef typename E0::value_type value_type;
    static constexpr size_t
      rows = E0::cols,
      cols = E0::rows;

    explicit constexpr MatrixTransposeExpr(const E0 &e0)
      : e0(e0) {}

    [[nodiscard]] constexpr value_type At(size_t i, size_t j) const noexcept { return e0.At(j, i); }

  private:
    const E0 e0;
};

}

template<typename T, size_t M, size_t N>
struct Matrix : public Impl::MatrixExpr<T, Matrix<T, M, N>> {
    typedef T value_type;
    typedef value_type array_type[M][N];
    typedef value_type flat_type[M * N];
    static constexpr size_t
      rows = M,
      cols = N;

    flat_type impl;

    Matrix()
      : impl({{0}}) {}

    ~Matrix() = default;

    template<Concepts::MatrixExpression E>
    requires (E::cols == cols, E::rows == rows)
    constexpr Matrix(const E &expr) noexcept {
        for (size_t j = 0; j < M; j++)
            for (size_t i = 0; i < N; i++)
                impl[i + j * N] = expr.At(j, i);
    }

    constexpr Matrix(flat_type &&data) noexcept { std::copy(data, data + M * N, impl); }

    constexpr Matrix(const flat_type &data) noexcept { std::copy(data, data + M * N, impl); }

    constexpr value_type At(size_t j, size_t i) const noexcept { return impl[i + j * N]; }

    constexpr value_type &At(size_t j, size_t i) noexcept { return impl[i + j * N]; }

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
          Matrix<T, 3, 3>{{
                            ca, -sa, 0,
                            sa, ca, 0,
                            0, 0, 1,
                          }} *
          Matrix<T, 3, 3>{{
                            cb, 0, sb,
                            0, 1, 0,
                            -sb, 0, cb,
                          }} *
          Matrix<T, 3, 3>{{
                            1, 0, 0,
                            0, cg, -sg,
                            0, sg, cg,
                          }};
    }
};

template<typename T, size_t n>
constexpr inline Matrix<T, n, n> Identity() noexcept {
    Matrix<T, n, n> mat{{0}};

    for (size_t i = 0; i < n; i++) mat.At(i, i) = 1;

    return mat;
}

//////////////////////////////////////
//// Element-wise unary operators ////
//////////////////////////////////////

template<Concepts::MatrixExpression E0, typename U>
requires std::is_convertible_v<U, typename E0::value_type>
constexpr inline auto operator*(const E0 &e0, const U s) noexcept { return Impl::MatrixUnaryExpr(e0, [s](auto v) { return v * s; }); }

template<Concepts::MatrixExpression E0, typename U>
requires std::is_convertible_v<U, typename E0::value_type>
constexpr inline auto operator/(const E0 &e0, const U s) noexcept { return Impl::MatrixUnaryExpr(e0, [s](auto v) { return v / s; }); }

template<Concepts::MatrixExpression E0>
constexpr inline auto operator-(const E0 &e0) noexcept { return Impl::MatrixUnaryExpr(e0, [](auto v) { return -v; }); }

template<Concepts::MatrixExpression E0>
constexpr inline auto Reciprocal(const E0 &e0) noexcept { return Impl::MatrixUnaryExpr(e0, [](auto v) { return 1. / v; }); }

/*template<Concepts::MatrixExpression E0, typename U>
requires std::is_convertible_v<U, typename E0::value_type>
constexpr inline auto Pow(const E0 &e0, const U s) noexcept { return Impl::MatrixUnaryExpr(e0, [s](auto v) { return std::pow(v, s); }); }*/

template<Concepts::MatrixExpression E0>
constexpr inline auto Transpose(const E0 &e0) noexcept { return Impl::MatrixTransposeExpr(e0); }

///////////////////////////////////////
//// Element-wise binary operators ////
///////////////////////////////////////

template<Concepts::MatrixExpression E0, Concepts::MatrixExpression E1>
constexpr inline auto operator+(const E0 &e0, const E1 &e1) noexcept { return Impl::MatrixBinaryExpr(e0, e1, std::plus{}); }

template<Concepts::MatrixExpression E0, Concepts::MatrixExpression E1>
constexpr inline auto operator-(const E0 &e0, const E1 &e1) noexcept { return Impl::MatrixBinaryExpr(e0, e1, std::minus{}); }

//// other /////
template<Concepts::MatrixExpression E0, Concepts::MatrixExpression E1>
constexpr inline auto operator*(const E0 &e0, const E1 &e1) noexcept { return Impl::MatrixMultExpr(e0, e1); }

}

template<Math::Concepts::MatrixExpression E>
struct fmt::formatter<E> {
    constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<typename FormatContext>
    constexpr auto format(const E &vec, FormatContext &ctx) -> decltype(ctx.out()) {
        auto o = ctx.out();

        o = fmt::format_to(o, "[");

        for (size_t j = 0; j < E::rows; j++) {
            o = fmt::format_to(o, "[");
            for (size_t i = 0; i < E::cols; i++) {
                o = fmt::format_to(o, "{}", vec.At(j, i));
                if (i != E::cols - 1) o = fmt::format_to(o, ",");
            }
            o = fmt::format_to(o, "]");
            if (j != E::rows - 1) o = fmt::format_to(o, ",");
        }

        return o;
    }
};
