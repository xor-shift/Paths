#pragma once

#include <cmath>
#include <concepts>
#include <fmt/format.h>
#include <functional>

#include <utils/ptrIterator.hpp>

namespace Math {

namespace Concepts {

template<typename T>
concept VectorExpression = requires(const T &ve, size_t idx) {
    { T::vectorSize } -> std::convertible_to<size_t>;
    { ve[idx] } -> std::convertible_to<typename T::value_type>;
};

};

namespace Impl {

template<typename T, typename E>
struct VectorExpr {
    typedef T value_type;
    static constexpr size_t vectorSize = E::vectorSize;

    [[nodiscard]] constexpr auto operator[](size_t idx) const noexcept { return static_cast<const E &>(*this)[idx]; }
};

template<Concepts::VectorExpression E0, Concepts::VectorExpression E1> requires (E0::vectorSize == 3 && E1::vectorSize == 3)
struct VecCrossProduct : public VectorExpr<typename E0::value_type, VecCrossProduct<E0, E1>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t vectorSize = E0::vectorSize;

    constexpr VecCrossProduct(const E0 &e0, const E1 &e1)
      : e0(e0), e1(e1) {}

    [[nodiscard]] constexpr auto operator[](size_t idx) const noexcept {
        if (idx == 0) return e0[1] * e1[2] - e0[2] * e1[1];
        else if (idx == 1) return e0[2] * e1[0] - e0[0] * e1[2];
        else return e0[0] * e1[1] - e0[1] * e1[0];
    }

  private:
    const E0 e0;
    const E1 e1;
};

template<Concepts::VectorExpression E0, Concepts::VectorExpression E1, typename Op> requires (E0::vectorSize == E1::vectorSize)
struct VecBinaryExpr : public VectorExpr<typename E0::value_type, VecBinaryExpr<E0, E1, Op>> {
    using base_type = VectorExpr<typename E0::value_type, VecBinaryExpr<E0, E1, Op>>;
    typedef typename E0::value_type value_type;
    static constexpr size_t vectorSize = E0::vectorSize;

    constexpr explicit VecBinaryExpr(const E0 &e0, const E1 &e1, const Op &op) noexcept requires std::is_nothrow_copy_constructible_v<Op>
      : e0(e0), e1(e1), op(op) {}

    constexpr explicit VecBinaryExpr(const E0 &e0, const E1 &e1) noexcept requires std::is_nothrow_default_constructible_v<Op>
      : e0(e0), e1(e1), op({}) {}

    constexpr auto operator[](size_t idx) const noexcept { return op(e0[idx], e1[idx]); }

  private:
    const E0 e0;
    const E1 e1;
    const Op op;
};

template<Concepts::VectorExpression E0, typename Op>
struct VecUnaryExpr : public VectorExpr<typename E0::value_type, VecUnaryExpr<E0, Op>> {
    using base_type = VectorExpr<typename E0::value_type, VecUnaryExpr<E0, Op>>;
    typedef typename E0::value_type value_type;
    static constexpr size_t vectorSize = E0::vectorSize;

    constexpr explicit VecUnaryExpr(const E0 &e0, const Op &op) noexcept requires std::is_nothrow_copy_constructible_v<Op>
      : e0(e0), op(op) {}

    constexpr explicit VecUnaryExpr(const E0 &e0) noexcept requires std::is_nothrow_default_constructible_v<Op>
      : e0(e0), op({}) {}

    constexpr auto operator[](size_t idx) const noexcept { return op(e0[idx]); }

  private:
    const E0 e0;
    const Op op;
};

}

template<typename T, size_t N>
struct Vector : public Impl::VectorExpr<T, Vector<T, N>> {
    typedef T value_type;
    static constexpr size_t vectorSize = N;
    typedef std::array<T, N> array_type;

    array_type impl;

    [[nodiscard]] auto begin() noexcept { return Utils::PtrIterator<T, N>(data()); }

    [[nodiscard]] auto end() noexcept { return Utils::PtrIterator<T, N>(data(), N); }

    [[nodiscard]] auto cbegin() const noexcept { return Utils::PtrIterator<const T, N>(data()); }

    [[nodiscard]] auto cend() const noexcept { return Utils::PtrIterator<const T, N>(data(), N); }

    constexpr Vector() noexcept
      : impl({}) {}

    template<Concepts::VectorExpression E>
    requires (E::vectorSize == vectorSize)
    constexpr Vector(const E &expr) noexcept {
        for (size_t i = 0; i < N; i++)
            impl[i] = static_cast<T>(expr[i]);
    }

    constexpr Vector(array_type &&v) noexcept {
        std::copy(v.data(), v.data() + N, data());
    }

    constexpr Vector(const array_type &v) {
        std::copy(v.data(), v.data() + N, data());
    }

    [[nodiscard]] constexpr size_t size() const noexcept { return N; }

    [[nodiscard]] constexpr T operator[](size_t idx) const noexcept { return impl[idx]; }

    [[nodiscard]] constexpr T &operator[](size_t idx) noexcept { return impl[idx]; }

    [[nodiscard]] constexpr T *data() noexcept { return impl.data(); }

    [[nodiscard]] constexpr const T *data() const noexcept { return impl.data(); }

    [[nodiscard]] constexpr auto &implData() noexcept { return impl; }

    [[nodiscard]] constexpr const auto &implData() const noexcept { return impl; }

#define v2vAssignEqualsFactory(oper) \
template<typename E0> \
requires (E0::vectorSize == vectorSize) \
constexpr Vector &operator oper##= (const E0 &e0) { for (size_t i = 0; i < vectorSize; i++) impl[i] oper##= e0[i]; return *this; }

    v2vAssignEqualsFactory(+)

    v2vAssignEqualsFactory(-)

    v2vAssignEqualsFactory(*)

    v2vAssignEqualsFactory(/)

#undef v2vAssignEqualsFactory

#define v2sAssignEqualsFactory(oper) \
template<typename U> \
constexpr Vector &operator oper##= (U s) { for (size_t i = 0; i < vectorSize; i++) impl[i] oper##= s; return *this; }

    v2sAssignEqualsFactory(+)

    v2sAssignEqualsFactory(-)

    v2sAssignEqualsFactory(*)

    v2sAssignEqualsFactory(/)

#undef v2sAssignEqualsFactory

};

//////////////////////////////////////
//// Element-wise unary operators ////
//////////////////////////////////////

template<Concepts::VectorExpression E0, typename U>
requires std::is_convertible_v<U, typename E0::value_type>
constexpr inline auto operator*(const E0 &e0, const U s) { return Impl::VecUnaryExpr(e0, [s](auto v) { return v * s; }); }

template<Concepts::VectorExpression E0, typename U>
requires std::is_convertible_v<U, typename E0::value_type>
constexpr inline auto operator/(const E0 &e0, const U s) { return Impl::VecUnaryExpr(e0, [s](auto v) { return v / s; }); }

template<Concepts::VectorExpression E0>
constexpr inline auto operator-(const E0 &e0) { return Impl::VecUnaryExpr(e0, [](auto v) { return -v; }); }

template<Concepts::VectorExpression E0>
constexpr inline auto Reciprocal(const E0 &e0) { return Impl::VecUnaryExpr(e0, [](auto v) { return 1. / v; }); }

template<Concepts::VectorExpression E0>
constexpr inline auto Abs(const E0 &e0) { return Impl::VecUnaryExpr(e0, [](auto v) { return std::abs(v); }); }

///////////////////////////////////////
//// Element-wise binary operators ////
///////////////////////////////////////

template<Concepts::VectorExpression E0, Concepts::VectorExpression E1>
requires (E0::vectorSize == E1::vectorSize)
constexpr inline auto operator+(const E0 &e0, const E1 &e1) { return Impl::VecBinaryExpr(e0, e1, std::plus{}); }

template<Concepts::VectorExpression E0, Concepts::VectorExpression E1>
requires (E0::vectorSize == E1::vectorSize)
constexpr inline auto operator-(const E0 &e0, const E1 &e1) { return Impl::VecBinaryExpr(e0, e1, std::minus{}); }

template<Concepts::VectorExpression E0, Concepts::VectorExpression E1>
requires (E0::vectorSize == E1::vectorSize)
constexpr inline auto operator*(const E0 &e0, const E1 &e1) { return Impl::VecBinaryExpr(e0, e1, std::multiplies{}); }

template<Concepts::VectorExpression E0, Concepts::VectorExpression E1>
requires (E0::vectorSize == E1::vectorSize)
constexpr inline auto operator/(const E0 &e0, const E1 &e1) { return Impl::VecBinaryExpr(e0, e1, std::divides{}); }

template<Concepts::VectorExpression E0, Concepts::VectorExpression E1>
constexpr inline auto Max(const E0 &e0, const E1 &e1) { return Impl::VecBinaryExpr(e0, e1, [](auto v0, auto v1) { return std::max(v0, v1); }); }

template<Concepts::VectorExpression E0, Concepts::VectorExpression E1>
constexpr inline auto Min(const E0 &e0, const E1 &e1) { return Impl::VecBinaryExpr(e0, e1, [](auto v0, auto v1) { return std::min(v0, v1); }); }

////////////////////////
//// Other function ////
////////////////////////

template<Concepts::VectorExpression E0, Concepts::VectorExpression E1>
constexpr inline Impl::VecCrossProduct<E0, E1> Cross(const E0 &e0, const E1 &e1) { return Impl::VecCrossProduct<E0, E1>{e0, e1}; }

template<Concepts::VectorExpression E0, Concepts::VectorExpression E1>
constexpr inline typename E0::value_type Dot(const E0 &e0, const E1 &e1) {
    typename E0::value_type sum = 0;
    for (size_t i = 0; i < E0::vectorSize; i++) sum += e0[i] * e1[i];
    return sum;
}

template<Concepts::VectorExpression E0>
constexpr inline typename E0::value_type Magnitude(const E0 &e0) { return std::sqrt(Dot(e0, e0)); }

template<Concepts::VectorExpression E0>
constexpr inline auto Normalized(const E0 &e0) { return e0 / Magnitude(e0); }

template<typename E0>
constexpr inline bool IsNormalized(const E0 &e0) {
    constexpr typename E0::value_type errorMargin = 0.00001L;
    return std::abs(Magnitude(e0) - typename E0::value_type(1)) <= errorMargin;
}

}

template<Math::Concepts::VectorExpression E>
struct fmt::formatter<E> {
    constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<typename FormatContext>
    constexpr auto format(const E &vec, FormatContext &ctx) -> decltype(ctx.out()) {
        auto o = ctx.out();

        o = fmt::format_to(o, "(");
        for (size_t i = 0; i < E::vectorSize; i++) {
            o = fmt::format_to(o, "{}", vec[i]);
            if (i != E::vectorSize - 1) o = fmt::format_to(o, ",");
        }
        o = fmt::format_to(o, ")");

        return o;
    }
};
