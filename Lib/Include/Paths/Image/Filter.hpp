#pragma once

#include "Maths/Maths.hpp"
#include "Paths/Color.hpp"

namespace Paths::Image::Filters {

namespace Detail {

template<typename E> struct FilterExpression {
    typedef ColorChannelType value_type;

    [[nodiscard]] constexpr auto operator()(ColorChannelType v) const noexcept {
        return static_cast<const E &>(*this)(v);
    }
};

template<typename Op> struct UnaryOpExpression : public FilterExpression<UnaryOpExpression<Op>> {
    constexpr explicit UnaryOpExpression(const Op &op) noexcept(std::is_nothrow_copy_constructible_v<Op>)
        : m_op(op) { }

    constexpr explicit UnaryOpExpression() noexcept(std::is_nothrow_default_constructible_v<Op>)
        : m_op({}) { }

    constexpr auto operator()(ColorChannelType v) const noexcept { return m_op(v); }

private:
    const Op m_op;
};

template<typename E0, typename E1>
struct UnarySequenceExpression : public FilterExpression<UnarySequenceExpression<E0, E1>> {
    constexpr explicit UnarySequenceExpression(const E0 &f_0, const E1 &f_1) noexcept
        : m_f_0(f_0)
        , m_f_1(f_1) { }

    constexpr auto operator()(ColorChannelType v) const noexcept { return m_f_1(m_f_0(v)); }

private:
    const E0 m_f_0;
    const E1 m_f_1;
};

}

namespace Unary {

/**
 * Performs Op on each ColorChannelType of a given Color.
 */
template<typename Op> constexpr auto oper(const Op &op) noexcept { return Detail::UnaryOpExpression { op }; }

/**
 * Performs v = lerp(min, max, t) where t is a ColorChannelType.
 */
constexpr auto lerp(ColorChannelType min, ColorChannelType max) noexcept {
    return oper([min, max](ColorChannelType v) { return Maths::lerp(min, max, v); });
}

/**
 * Performs the inverse function of lerp, t = lerp(min, max, v).
 */
constexpr auto inv_lerp(ColorChannelType min, ColorChannelType max) noexcept {
    return oper([min, max](ColorChannelType v) { return Maths::inv_lerp(min, max, v); });
}

/**
 * Clamps color channels within a range
 */
constexpr auto clamp(ColorChannelType min, ColorChannelType max) noexcept {
    return oper([min, max](ColorChannelType v) { return std::clamp(v, min, max); });
}

/**
 * Sequences two or more unary filter expressions. The sequenced filters will
 * be applied in the order they are passed in.
 * @return The sequenced filters
 */
template<typename E0, typename... Es> constexpr auto sequence(const E0 &f_0, const Es &...f_next) noexcept {
    constexpr std::size_t n_next = sizeof...(Es);
    static_assert(n_next != 0);

    if constexpr (n_next == 1)
        return Detail::UnarySequenceExpression { f_0, f_next... };
    else
        return Detail::UnarySequenceExpression { f_0, sequence<Es...>(f_next...) };
}

}

}
