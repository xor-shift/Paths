#pragma once

#include "concepts.hpp"
#include "vector.hpp"
#include "matrix.hpp"
#include "matvec.hpp"

#include <functional>
#include <fmt/format.h>

namespace Math {

template<typename T = float>
struct Quaternion {
    //T a, b, c, d;
    T data[4];

#define q2qOperAssignEqualsFactory(oper) \
    template<typename U> \
    constexpr Quaternion &operator oper##= (const Quaternion<U> &other) noexcept { \
        for (std::size_t i = 0; i < 4; i++) data[i] oper##= other.data[i]; \
        return *this; \
    }

    q2qOperAssignEqualsFactory(+)

    q2qOperAssignEqualsFactory(-)

#undef q2qOperAssignEqualsFactory

#define q2qOperFactory(oper)
#undef q2qOperFactory

    template<typename U>
    friend constexpr Quaternion operator*(const Quaternion &lhs, const Quaternion<U> &rhs) {
        const auto &[a1, b1, c1, d1] = lhs.data;
        const auto &[a2, b2, c2, d2] = rhs.data;

        return {
          a1 * a2 - b1 * b2 - c1 * c2 - d1 * d2,
          a1 * b2 + b1 * a2 + c1 * d2 - d1 * c2,
          a1 * c2 - b1 * d2 + c1 * a2 + d1 * b2,
          a1 * d2 + b1 * c2 - c1 * b2 + d1 * a2
        };
    }
};

}

namespace Math::Ops {

namespace Vector {

namespace Impl {
template<typename T, typename U, std::size_t N, typename BinaryCallback>
constexpr ::Math::Vector<T, N> BinaryConvolve(const ::Math::Vector<T, N> &lhs, const ::Math::Vector<U, N> &rhs, BinaryCallback &&fn) noexcept {
    ::Math::Vector<T, N> out{};
    for (std::size_t i = 0; i < N; i++) out[i] = std::invoke(fn, lhs[i], rhs[i]);
    return out;
}
}

template<typename T, typename U, std::size_t N>
constexpr T Dot(const ::Math::Vector<T, N> &lhs, const ::Math::Vector<U, N> &rhs) noexcept { return lhs.Dot(rhs); }

template<typename T, typename U>
constexpr ::Math::Vector<T, 3> Cross(const ::Math::Vector<T, 3> &lhs, const ::Math::Vector<U, 3> &rhs) noexcept {
    auto v = lhs;
    v.Cross(rhs);
    return v;
}

template<typename T, typename U, std::size_t N>
constexpr ::Math::Vector<T, N> MinPerElem(const ::Math::Vector<T, N> &lhs, const ::Math::Vector<U, N> &rhs) noexcept { return Impl::BinaryConvolve(lhs, rhs, [](auto l, auto r) { return std::min(l, r); }); }

template<typename T, typename U, std::size_t N>
constexpr ::Math::Vector<T, N> MaxPerElem(const ::Math::Vector<T, N> &lhs, const ::Math::Vector<U, N> &rhs) noexcept { return Impl::BinaryConvolve(lhs, rhs, [](auto l, auto r) { return std::max(l, r); }); }

template<typename T, std::size_t N>
constexpr ::Math::Vector<T, N> Normalized(const ::Math::Vector<T, N> &vec) noexcept {
    auto v = vec;
    v.Normalize();
    return v;
}

template<typename T, std::size_t N>
constexpr T MinElem(const ::Math::Vector<T, N> &vec) noexcept {
    T v = std::numeric_limits<T>::max();
    for (std::size_t i = 0; i < N; i++) v = std::min(v, vec[i]);
    return v;
}

template<typename T, std::size_t N>
constexpr T MaxElem(const ::Math::Vector<T, N> &vec) noexcept {
    T v = std::numeric_limits<T>::min();
    for (std::size_t i = 0; i < N; i++) v = std::max(v, vec[i]);
    return v;
}
} //Math::Ops::Vector

namespace Matrix {

}

} //Math::Ops

template<typename T, std::size_t N>
struct fmt::formatter<Math::Vector<T, N>> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    constexpr auto format(const Math::Vector<T, N> &vec, FormatContext &ctx) -> decltype(ctx.out()) {
        return FormatInserter<decltype(ctx.out()), 0>(vec, fmt::format_to(ctx.out(), "("));
    }

  private:
    template<typename C, std::size_t i = 0>
    [[nodiscard]] constexpr C FormatInserter(const Math::Vector<T, N> &vec, C o) {
        if constexpr (i >= N) return o;
        else return FormatInserter<C, i + 1>(vec, fmt::format_to(o, (i + 1 == N) ? "{})" : "{}, ", vec[i]));
    }

};
