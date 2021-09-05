#pragma once

#include <cmath>

namespace Math {

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
          constexpr friend bool operator==(VectorIterator &lhs, VectorIterator<isOtherConst> &rhs) noexcept {
            return lhs.idx == rhs.idx;
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

}
