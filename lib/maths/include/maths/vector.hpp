#pragma once

#include <cmath>
#include <fmt/format.h>

#include <utils/ptrIterator.hpp>

namespace Math {

template<typename T, size_t N>
class Vector {
  public:
    T data[N];

    [[nodiscard]] auto begin() & noexcept { return PtrIterator<T, N>(data); }

    [[nodiscard]] auto end() & noexcept { return PtrIterator<T, N>(data, N); }

    [[nodiscard]] auto cbegin() const & noexcept { return PtrIterator<const T, N>(data); }

    [[nodiscard]] auto cend() const & noexcept { return PtrIterator<const T, N>(data, N); }

    template<typename U, U ...Vs>
    constexpr static Vector FromISequence(std::integer_sequence<U, Vs...> seq) requires (seq.size() == N) { return {static_cast<T>(Vs)...}; }

    template<typename U>
    [[nodiscard]] constexpr explicit operator Vector<U, N>() const noexcept requires std::is_convertible_v<T, U> {
        Vector<U, N> v{};
        for (std::size_t i = 0; i < N; i++) v[i] = static_cast<U>(data[i]);
        return v;
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept { return N; }

    [[nodiscard]] constexpr T operator[](std::size_t i) const & noexcept { return data[i]; }

    [[nodiscard]] constexpr T &operator[](std::size_t i) & noexcept { return data[i]; }

    [[nodiscard]] constexpr T &&operator[](std::size_t i) && noexcept { return std::move(data[i]); }

    template<typename U>
    [[nodiscard]] constexpr friend bool operator==(const Vector<T, N> &lhs, const Vector<U, N> &rhs) noexcept {
        for (std::size_t i = 0; i < N; i++) if (lhs[i] != rhs[i]) return false;
        return true;
    }

#define v2vAssignEqualsFactory(oper) \
template<typename U> \
constexpr Vector &operator oper##= (const Vector<U, N> &other) noexcept { \
for (std::size_t i = 0; i < N; i++) data[i] oper##= other[i]; \
return *this; \
}

    v2vAssignEqualsFactory(+)

    v2vAssignEqualsFactory(-)

    v2vAssignEqualsFactory(*)

    v2vAssignEqualsFactory(/)

#undef v2vAssignEqualsFactory

    template<typename U>
    constexpr Vector<T, N> &operator*=(U scalar) noexcept requires Multipliable<T, U> {
        std::transform(data, data + N, data, [scalar](T v) { return v * scalar; });
        return *this;
    }

    template<typename U>
    constexpr Vector<T, N> &operator/=(U scalar) noexcept requires Divisible<T, U> {
        std::transform(data, data + N, data, [scalar](T v) { return v / scalar; });
        return *this;
    }

#define v2sOperatorFactory(oper) \
template<typename U>         \
[[nodiscard]] friend Vector operator oper (const Vector &lhs, U scalar) { auto temp = lhs; temp oper##= scalar; return temp; }

    v2sOperatorFactory(*)

    v2sOperatorFactory(/)

#undef v2sOperatorFactory

#define v2vOperatorFactory(oper) \
template<typename U> \
[[nodiscard]] constexpr friend Vector operator oper (const Vector &lhs, const Vector<U, N> &rhs) noexcept { auto temp = lhs; temp oper##= rhs; return temp; }

    v2vOperatorFactory(+)

    v2vOperatorFactory(-)

    v2vOperatorFactory(*)

    v2vOperatorFactory(/)

#undef v2vOperatorFactory

    Vector operator-() const noexcept {
        Vector v{};
        for (size_t i = 0; i < N; i++) v[i] = -data[i];
        return v;
    }
};

template<typename T, typename U, size_t N>
static inline constexpr T Dot(const Vector<T, N> &lhs, const Vector<U, N> &rhs) noexcept {
    T sum = 0.;
    for (std::size_t i = 0; i < N; i++) sum += lhs[i] * rhs[i];
    return sum;
}

template<typename T, size_t N>
static inline constexpr T Magnitude(const Vector<T, N> &vec) noexcept { return std::sqrt(Dot(vec, vec)); }

template<typename T, size_t N>
static inline constexpr Vector<T, N> Normalized(const Vector<T, N> &vec) noexcept { return vec / Magnitude(vec); }

template<typename T, size_t N, T errorMargin = 0.00001L>
static inline constexpr Vector<T, N> IsNormalized(const Vector<T, N> &vec) noexcept { return std::abs(Magnitude(vec) - T(1)) <= errorMargin; }

template<typename T, typename U>
static inline constexpr Vector<T, 3> Cross(const Vector<T, 3> &lhs, const Vector<U, 3> &rhs) noexcept {
    return {
      lhs[1] * rhs[2] - lhs[2] * rhs[1],
      lhs[2] * rhs[0] - lhs[0] * rhs[2],
      lhs[0] * rhs[1] - lhs[1] * rhs[0],
    };
}

template<typename T, std::size_t N>
constexpr T MinElem(const Vector<T, N> &vec) noexcept {
    T v = std::numeric_limits<T>::max();
    for (std::size_t i = 0; i < N; i++) v = std::min(v, vec[i]);
    return v;
}

template<typename T, std::size_t N>
constexpr T MaxElem(const Vector<T, N> &vec) noexcept {
    T v = std::numeric_limits<T>::min();
    for (std::size_t i = 0; i < N; i++) v = std::max(v, vec[i]);
    return v;
}

namespace Impl {
template<typename T, typename U, std::size_t N, typename BinaryCallback>
constexpr Vector<T, N> BinaryConvolve(const Vector<T, N> &lhs, const Vector<U, N> &rhs, BinaryCallback &&fn) noexcept {
    Vector<T, N> out{};
    for (std::size_t i = 0; i < N; i++) out[i] = std::invoke(fn, lhs[i], rhs[i]);
    return out;
}
}

template<typename T, typename U, std::size_t N>
constexpr Vector<T, N> MinPerElem(const Vector<T, N> &lhs, const Vector<U, N> &rhs) noexcept { return Impl::BinaryConvolve(lhs, rhs, [](auto l, auto r) { return std::min(l, r); }); }

template<typename T, typename U, std::size_t N>
constexpr Vector<T, N> MaxPerElem(const Vector<T, N> &lhs, const Vector<U, N> &rhs) noexcept { return Impl::BinaryConvolve(lhs, rhs, [](auto l, auto r) { return std::max(l, r); }); }

}

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
