#pragma once

#include <gfx/color.hpp>
#include <maths/maths.hpp>

namespace Gfx::Image::Filters {

namespace Detail {

template<typename E>
struct FilterExpression {
    typedef ColorChannelType value_type;

    [[nodiscard]] constexpr auto operator()(ColorChannelType v) const noexcept { return static_cast<const E &>(*this)(v); }
};

template<typename Op>
struct UnaryOpExpression : public FilterExpression<UnaryOpExpression<Op>> {
    constexpr explicit UnaryOpExpression(const Op &op) noexcept(std::is_nothrow_copy_constructible_v<Op>)
      : op(op) {}

    constexpr explicit UnaryOpExpression() noexcept(std::is_nothrow_default_constructible_v<Op>)
      : op({}) {}

    constexpr auto operator()(ColorChannelType v) const noexcept { return op(v); }

  private:
    const Op op;
};

template<typename E0, typename E1>
struct UnarySequenceExpression : public FilterExpression<UnarySequenceExpression<E0, E1>> {
    constexpr explicit UnarySequenceExpression(const E0 &f0, const E1 &f1) noexcept
      : f0(f0), f1(f1) {}

    constexpr auto operator()(ColorChannelType v) const noexcept { return f1(f0(v)); }

  private:
    const E0 f0;
    const E1 f1;
};

}

namespace Unary {

template<typename Op>
constexpr auto Oper(const Op &op) noexcept { return Detail::UnaryOpExpression{op}; }

constexpr auto Lerp(ColorChannelType min, ColorChannelType max) noexcept { return Oper([min, max](ColorChannelType v) { return Maths::Lerp(min, max, v); }); }

constexpr auto InvLerp(ColorChannelType min, ColorChannelType max) noexcept { return Oper([min, max](ColorChannelType v) { return Maths::InvLerp(min, max, v); }); }

constexpr auto Clamp(ColorChannelType min, ColorChannelType max) noexcept { return Oper([min, max](ColorChannelType v) { return std::clamp(v, min, max); }); }

template<typename E0, typename ...Es>
constexpr auto Sequence(const E0 &f0, const Es &...fNext) noexcept {
    constexpr std::size_t nNext = sizeof...(Es);
    static_assert(nNext != 0);

    if constexpr (nNext == 1) return Detail::UnarySequenceExpression{f0, fNext...};
    else return Detail::UnarySequenceExpression{f0, Sequence<Es...>(fNext...)};
}

}

}
