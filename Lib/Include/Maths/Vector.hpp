#pragma once

#include <cmath>
#include <concepts>
#include <fmt/format.h>
#include <functional>

#include "Utils/PointerIterator.hpp"

namespace Maths {

namespace Concepts {

template<typename T>
concept VectorExpression = requires(const T &ve, size_t idx) {
    { T::m_vector_size } -> std::convertible_to<size_t>;
    { ve[idx] } -> std::convertible_to<typename T::value_type>;
};

};

namespace Impl {

template<typename T, typename E> struct VectorExpr {
    typedef T value_type;
    static constexpr size_t m_vector_size = E::m_vector_size;

    [[nodiscard]] constexpr auto operator[](size_t idx) const noexcept { return static_cast<const E &>(*this)[idx]; }
};

template<Concepts::VectorExpression E0, Concepts::VectorExpression E1>
requires(E0::m_vector_size == 3 && E1::m_vector_size == 3) struct VecCrossProduct
    : public VectorExpr<typename E0::value_type, VecCrossProduct<E0, E1>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t m_vector_size = E0::m_vector_size;

    constexpr VecCrossProduct(const E0 &e_0, const E1 &e_1)
        : m_e_0(e_0)
        , m_e_1(e_1) { }

    [[nodiscard]] constexpr auto operator[](size_t idx) const noexcept {
        // return e0[(idx + 1) % 3] * e1[(idx + 2) % 3] - e0[(idx + 2) % 3] * e1[(idx + 1) % 3];
        if (idx == 0)
            return m_e_0[1] * m_e_1[2] - m_e_0[2] * m_e_1[1];
        if (idx == 1)
            return m_e_0[2] * m_e_1[0] - m_e_0[0] * m_e_1[2];
        return m_e_0[0] * m_e_1[1] - m_e_0[1] * m_e_1[0];
    }

private:
    const E0 m_e_0;
    const E1 m_e_1;
};

template<Concepts::VectorExpression E0, Concepts::VectorExpression E1, typename Op>
requires(E0::m_vector_size == E1::m_vector_size) struct VecBinaryExpr
    : public VectorExpr<typename E0::value_type, VecBinaryExpr<E0, E1, Op>> {
    using base_type = VectorExpr<typename E0::value_type, VecBinaryExpr<E0, E1, Op>>;
    typedef typename E0::value_type value_type;
    static constexpr size_t m_vector_size = E0::m_vector_size;

    constexpr explicit VecBinaryExpr(const E0 &e_0, const E1 &e_1, const Op &op) noexcept(
        std::is_nothrow_copy_constructible_v<Op>)
        : m_e_0(e_0)
        , m_e_1(e_1)
        , m_op(op) { }

    constexpr explicit VecBinaryExpr(const E0 &e_0, const E1 &e_1) noexcept(std::is_nothrow_default_constructible_v<Op>)
        : m_e_0(e_0)
        , m_e_1(e_1)
        , m_op({}) { }

    constexpr auto operator[](size_t idx) const noexcept { return m_op(m_e_0[idx], m_e_1[idx]); }

private:
    const E0 m_e_0;
    const E1 m_e_1;
    const Op m_op;
};

template<Concepts::VectorExpression E0, typename Op>
struct VecUnaryExpr : public VectorExpr<typename E0::value_type, VecUnaryExpr<E0, Op>> {
    using base_type = VectorExpr<typename E0::value_type, VecUnaryExpr<E0, Op>>;
    typedef typename E0::value_type value_type;
    static constexpr size_t m_vector_size = E0::m_vector_size;

    constexpr explicit VecUnaryExpr(const E0 &e_0, const Op &op) noexcept(std::is_nothrow_copy_constructible_v<Op>)
        : m_e_0(e_0)
        , m_op(op) { }

    constexpr explicit VecUnaryExpr(const E0 &e_0) noexcept(std::is_nothrow_default_constructible_v<Op>)
        : m_e_0(e_0)
        , m_op({}) { }

    constexpr auto operator[](size_t idx) const noexcept { return m_op(m_e_0[idx]); }

private:
    const E0 m_e_0;
    const Op m_op;
};

}

template<typename T, size_t N> struct Vector : public Impl::VectorExpr<T, Vector<T, N>> {
    typedef T value_type;
    static constexpr size_t m_vector_size = N;
    typedef std::array<T, N> array_type;

    typedef T *iterator;
    typedef const T *const_iterator;
    typedef std::reverse_iterator<T *> reverse_iterator;
    typedef std::reverse_iterator<const T *> const_reverse_iterator;

    array_type m_impl {};

    [[nodiscard]] constexpr iterator begin() noexcept { return data(); }

    [[nodiscard]] constexpr iterator end() noexcept { return data() + N; }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return data(); }

    [[nodiscard]] constexpr const_iterator cend() const noexcept { return data() + N; }

    constexpr Vector() noexcept = default;

    template<Concepts::VectorExpression E>
    requires(E::m_vector_size == m_vector_size) constexpr Vector(const E &expr) noexcept {
        for (size_t i = 0; i < N; i++)
            m_impl[i] = static_cast<T>(expr[i]);
    }

    template<typename... Ts>
    constexpr Vector(Ts... vs) noexcept
        : m_impl({ static_cast<T>(vs)... }) { }

    [[nodiscard]] constexpr size_t size() const noexcept { return N; }

    [[nodiscard]] constexpr size_t max_size() const noexcept { return N; }

    [[nodiscard]] constexpr T operator[](size_t idx) const noexcept { return m_impl[idx]; }

    [[nodiscard]] constexpr T &operator[](size_t idx) noexcept { return m_impl[idx]; }

    [[nodiscard]] constexpr T *data() noexcept { return m_impl.data(); }

    [[nodiscard]] constexpr const T *data() const noexcept { return m_impl.data(); }

    [[nodiscard]] constexpr auto &impl_data() noexcept { return m_impl; }

    [[nodiscard]] constexpr const auto &impl_data() const noexcept { return m_impl; }

#define V_2_V_ASSIGN_EQUALS_FACTORY(oper)                                                           \
    template<typename E0>                                                                           \
    requires(E0::m_vector_size == m_vector_size) constexpr Vector &operator oper##=(const E0 &e0) { \
        for (size_t i = 0; i < m_vector_size; i++)                                                  \
            m_impl[i] oper## = e0[i];                                                               \
        return *this;                                                                               \
    }

    V_2_V_ASSIGN_EQUALS_FACTORY(+)

    V_2_V_ASSIGN_EQUALS_FACTORY(-)

    V_2_V_ASSIGN_EQUALS_FACTORY(*)

    V_2_V_ASSIGN_EQUALS_FACTORY(/)

#undef V_2_V_ASSIGN_EQUALS_FACTORY

#define V_2_S_ASSIGN_EQUALS_FACTORY(oper)                          \
    template<typename U> constexpr Vector &operator oper##=(U s) { \
        for (size_t i = 0; i < m_vector_size; i++)                 \
            m_impl[i] oper## = s;                                  \
        return *this;                                              \
    }

    V_2_S_ASSIGN_EQUALS_FACTORY(+)

    V_2_S_ASSIGN_EQUALS_FACTORY(-)

    V_2_S_ASSIGN_EQUALS_FACTORY(*)

    V_2_S_ASSIGN_EQUALS_FACTORY(/)

#undef V_2_S_ASSIGN_EQUALS_FACTORY
};

//////////////////////////////////////
//// Element-wise unary operators ////
//////////////////////////////////////

template<Concepts::VectorExpression E0, typename U>
requires std::is_convertible_v<U, typename E0::value_type>
constexpr auto operator*(const E0 &e_0, const U s) {
    return Impl::VecUnaryExpr(e_0, [s](auto v) { return v * s; });
}

template<Concepts::VectorExpression E0, typename U>
requires std::is_convertible_v<U, typename E0::value_type>
constexpr auto operator/(const E0 &e_0, const U s) {
    return Impl::VecUnaryExpr(e_0, [s = s](auto v) { return v / s; });
}

template<Concepts::VectorExpression E0> constexpr auto operator-(const E0 &e_0) {
    return Impl::VecUnaryExpr(e_0, [](auto v) { return -v; });
}

template<Concepts::VectorExpression E0> constexpr auto reciprocal(const E0 &e_0) {
    return Impl::VecUnaryExpr(e_0, [](auto v) { return 1. / v; });
}

template<Concepts::VectorExpression E0> constexpr auto abs(const E0 &e_0) {
    return Impl::VecUnaryExpr(e_0, [](auto v) { return std::abs(v); });
}

///////////////////////////////////////
//// Element-wise binary operators ////
///////////////////////////////////////

template<Concepts::VectorExpression E0, Concepts::VectorExpression E1>
requires(E0::m_vector_size == E1::m_vector_size) constexpr auto operator+(const E0 &e_0, const E1 &e_1) {
    return Impl::VecBinaryExpr(e_0, e_1, std::plus {});
}

template<Concepts::VectorExpression E0, Concepts::VectorExpression E1>
requires(E0::m_vector_size == E1::m_vector_size) constexpr auto operator-(const E0 &e_0, const E1 &e_1) {
    return Impl::VecBinaryExpr(e_0, e_1, std::minus {});
}

template<Concepts::VectorExpression E0, Concepts::VectorExpression E1>
requires(E0::m_vector_size == E1::m_vector_size) constexpr auto operator*(const E0 &e_0, const E1 &e_1) {
    return Impl::VecBinaryExpr(e_0, e_1, std::multiplies {});
}

template<Concepts::VectorExpression E0, Concepts::VectorExpression E1>
requires(E0::m_vector_size == E1::m_vector_size) constexpr auto operator/(const E0 &e_0, const E1 &e_1) {
    return Impl::VecBinaryExpr(e_0, e_1, std::divides {});
}

template<Concepts::VectorExpression E0, Concepts::VectorExpression E1>
constexpr auto max(const E0 &e_0, const E1 &e_1) {
    return Impl::VecBinaryExpr(e_0, e_1, [](auto v_0, auto v_1) { return std::max(v_0, v_1); });
}

/// Calculates the elementwise minimum of two vector expressions
/// @example
/// Min({1, 2, 3}, {3, 2, 1}) -> {1, 2, 1}
/// \return The minimum expression
template<Concepts::VectorExpression E0, Concepts::VectorExpression E1>
constexpr auto min(const E0 &e_0, const E1 &e_1) {
    return Impl::VecBinaryExpr(e_0, e_1, [](auto v_0, auto v_1) { return std::min(v_0, v_1); });
}

////////////////////////
//// Other function ////
////////////////////////

template<Concepts::VectorExpression E0, Concepts::VectorExpression E1>
constexpr Impl::VecCrossProduct<E0, E1> cross(const E0 &e_0, const E1 &e_1) {
    return Impl::VecCrossProduct<E0, E1> { e_0, e_1 };
}

template<Concepts::VectorExpression E0, Concepts::VectorExpression E1>
constexpr typename E0::value_type dot(const E0 &e_0, const E1 &e_1) {
    typename E0::value_type sum = 0;
    for (size_t i = 0; i < E0::m_vector_size; i++)
        sum += e_0[i] * e_1[i];
    return sum;
}

template<Concepts::VectorExpression E0> constexpr typename E0::value_type Magnitude(const E0 &e_0) {
    return std::sqrt(dot(e_0, e_0));
}

template<Concepts::VectorExpression E0> constexpr auto normalized(const E0 &e_0) { return e_0 / Magnitude(e_0); }

template<typename E0> constexpr bool is_normalized(const E0 &e_0) {
    constexpr typename E0::value_type error_margin = 0.00001L;
    return std::abs(Magnitude(e_0) - typename E0::value_type(1)) <= error_margin;
}

}

template<typename VE>
requires Maths::Concepts::VectorExpression<VE>
struct fmt::formatter<VE, char, std::enable_if_t<Maths::Concepts::VectorExpression<VE>, void>> {
    constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<typename FormatContext> constexpr auto format(const VE &vec, FormatContext &ctx) -> decltype(ctx.out()) {
        auto o = ctx.out();

        o = fmt::format_to(o, "(");
        for (size_t i = 0; i < VE::m_vector_size; i++) {
            o = fmt::format_to(o, "{}", vec[i]);
            if (i != VE::m_vector_size - 1)
                o = fmt::format_to(o, ",");
        }
        o = fmt::format_to(o, ")");

        return o;
    }
};
