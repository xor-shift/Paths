#pragma once

#include <functional>
#include "fmt/format.h"

namespace Math {

template<typename T, typename U>
concept Addable = requires(T x, U y) { x + y; x += y; };

template<typename T, typename U>
concept Subtractable = requires(T x, U y) { x - y; x -= y; };

template<typename T, typename U>
concept Multipliable = requires(T x, U y) { x * y; x *= y; };

template<typename T, typename U>
concept Divisible = requires(T x, U y) { x / y; x /= y; };

template<typename T, size_t N>
class Vector {
  public:
    T data[N];

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
    [[nodiscard]] friend Vector operator oper (const Vector &lhs, const Vector<U, N> &rhs) noexcept { auto temp = lhs; temp oper##= rhs; return temp; }

    v2vOperatorFactory(+)

    v2vOperatorFactory(-)

    v2vOperatorFactory(*)

    v2vOperatorFactory(/)

#undef v2vOperatorFactory

    template<bool isConst = false>
    class VectorIterator {
      public:
        constexpr VectorIterator() noexcept
          : idx(N) {}

        constexpr ~VectorIterator() noexcept = default;

        explicit constexpr VectorIterator(Vector<T, N> &vec) noexcept
          : data(&vec.data[0]), cData(&vec.data[0]) {}

        explicit constexpr VectorIterator(const Vector<T, N> &vec) noexcept
          : cData(&vec.data[0]) {}

        template<bool isOtherConst = false>
        constexpr bool operator==(VectorIterator<isOtherConst> &other) const noexcept {
            return idx == other.idx;
        }

        constexpr VectorIterator &operator++() noexcept {
            ++idx;
            return *this;
        }

        constexpr const VectorIterator operator++(int) noexcept {
            auto a = *this;
            ++idx;
            return a;
        }

        constexpr VectorIterator &operator--() noexcept {
            --idx;
            return *this;
        }

        constexpr const VectorIterator operator--(int) noexcept {
            auto a = *this;
            --idx;
            return a;
        }

        constexpr const T &operator*() const & noexcept { return cData[idx]; }

        constexpr T &operator*() & noexcept requires (!isConst) { return data[idx]; }

        constexpr T &&operator*() && noexcept requires (!isConst) { return std::move(data[idx]); }

      private:
        T *data{nullptr};
        const T *cData{nullptr};
        std::size_t idx{0};
    };

    VectorIterator(const Vector<T, N> &vec) -> VectorIterator<true>;
    VectorIterator(Vector<T, N> &vec) -> VectorIterator<false>;

    VectorIterator<false> begin() { return VectorIterator(*this); }

    VectorIterator<true> cbegin() const { return VectorIterator(*this); }

    VectorIterator<false> end() { return VectorIterator<false>(); }

    VectorIterator<true> cend() const { return VectorIterator<true>(); }

    template<typename U>
    constexpr T Dot(const Vector<U, N> &other) const {
        T sum = 0.;
        for (std::size_t i = 0; i < N; i++) sum += data[i] * other.data[i];
        return sum;
    }

    template<typename U>
    constexpr Vector &Cross(const Vector<U, 3> &other) noexcept requires (N == 3) {
        T tempData[N]{
          data[0] * other[2] - data[2] * other[1],
          data[2] * other[0] - data[0] * other[2],
          data[0] * other[1] - data[1] * other[0],
        };

        data[0] = tempData[0];
        data[1] = tempData[1];
        data[2] = tempData[2];

        return *this;
    }

    constexpr T Magnitude() const noexcept {
        return std::sqrt(Dot(*this));
    }

    constexpr Vector<T, N> &Normalize() noexcept { return *this /= Magnitude(); }

    [[nodiscard]] bool IsNormalized() const noexcept {
        constexpr T errorMargin = 0.001;
        auto amount = std::abs(Magnitude() - 1.);
        return amount <= errorMargin;
    }

};

template<typename T, size_t M, size_t N>
struct Matrix {
    std::array<std::array<T, N>, M> data{{0}};

    static Matrix Rotation(double theta) requires (M == 2 && N == 2) {
        T sinTheta = std::sin(theta);
        Matrix<T, 2, 2> mat{
            {},
            {}
        };
        return mat;
    }
};

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
