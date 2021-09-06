#pragma once

#include <cmath>
#include <fmt/format.h>

#include <utils/ptrIterator.hpp>

namespace Math {

namespace Impl {

template<typename T, size_t N>
struct Array {
    typedef T data_type[N];
    T data[N];

    [[nodiscard]] constexpr T operator[](size_t idx) const noexcept { return data[idx]; }

    [[nodiscard]] constexpr T &operator[](size_t idx) noexcept { return data[idx]; }
};

template<typename T, typename E>
struct VectorExpr {
    typedef T value_type;
    static constexpr size_t arraySize = E::arraySize;

    [[nodiscard]] constexpr size_t size() const noexcept { return static_cast<const E &>(*this).size(); }

    [[nodiscard]] constexpr value_type operator[](size_t idx) const noexcept { return static_cast<const E &>(*this)[idx]; }
};

template<typename E0, typename E1>
struct VecSumExpr : public Impl::VectorExpr<typename E0::value_type, VecSumExpr<E0, E1>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t arraySize = E0::arraySize;

    const E0 &e0;
    const E1 &e1;

    constexpr VecSumExpr(const E0 &e0, const E1 &e1)
      : e0(e0), e1(e1) {}

    [[nodiscard]] constexpr size_t size() const noexcept { return e0.size(); }

    [[nodiscard]] constexpr value_type operator[](size_t idx) const noexcept { return e0[idx] + e1[idx]; }
};

template<typename E0, typename E1>
struct VecDiffExpr : public Impl::VectorExpr<typename E0::value_type, VecDiffExpr<E0, E1>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t arraySize = E0::arraySize;

    const E0 &e0;
    const E1 &e1;

    constexpr VecDiffExpr(const E0 &e0, const E1 &e1)
      : e0(e0), e1(e1) {}

    [[nodiscard]] constexpr size_t size() const noexcept { return e0.size(); }

    [[nodiscard]] constexpr value_type operator[](size_t idx) const noexcept { return e0[idx] - e1[idx]; }
};

template<typename E0, typename E1>
struct VecEWiseProductExpr : public Impl::VectorExpr<typename E0::value_type, VecEWiseProductExpr<E0, E1>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t arraySize = E0::arraySize;

    const E0 &e0;
    const E1 &e1;

    constexpr VecEWiseProductExpr(const E0 &e0, const E1 &e1)
      : e0(e0), e1(e1) {}

    [[nodiscard]] constexpr size_t size() const noexcept { return e0.size(); }

    [[nodiscard]] constexpr value_type operator[](size_t idx) const noexcept { return e0[idx] * e1[idx]; }
};

template<typename E0>
struct VecScalarProductExpr : public Impl::VectorExpr<typename E0::value_type, VecScalarProductExpr<E0>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t arraySize = E0::arraySize;

    const E0 &e0;
    const value_type s;

    constexpr VecScalarProductExpr(const E0 &e0, const value_type s)
      : e0(e0), s(s) {}

    [[nodiscard]] constexpr size_t size() const noexcept { return e0.size(); }

    [[nodiscard]] constexpr value_type operator[](size_t idx) const noexcept { return e0[idx] * s; }
};

template<typename E0, typename E1>
struct VecEWiseDivisionExpr : public Impl::VectorExpr<typename E0::value_type, VecEWiseDivisionExpr<E0, E1>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t arraySize = E0::arraySize;

    const E0 &e0;
    const E1 &e1;

    constexpr VecEWiseDivisionExpr(const E0 &e0, const E1 &e1)
      : e0(e0), e1(e1) {}

    [[nodiscard]] constexpr size_t size() const noexcept { return e0.size(); }

    [[nodiscard]] constexpr value_type operator[](size_t idx) const noexcept { return e0[idx] / e1[idx]; }
};

template<typename E0>
struct VecScalarDivisionExpr : public Impl::VectorExpr<typename E0::value_type, VecScalarDivisionExpr<E0>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t arraySize = E0::arraySize;

    const E0 &e0;
    const value_type s;

    constexpr VecScalarDivisionExpr(const E0 &e0, const value_type s)
      : e0(e0), s(s) {}

    [[nodiscard]] constexpr size_t size() const noexcept { return e0.size(); }

    [[nodiscard]] constexpr value_type operator[](size_t idx) const noexcept { return e0[idx] / s; }
};

template<typename E0>
struct VecNegationExpr : public Impl::VectorExpr<typename E0::value_type, VecNegationExpr<E0>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t arraySize = E0::arraySize;

    const E0 &e0;

    constexpr VecNegationExpr(const E0 &e0)
      : e0(e0) {}

    [[nodiscard]] constexpr size_t size() const noexcept { return e0.size(); }

    [[nodiscard]] constexpr value_type operator[](size_t idx) const noexcept { return -e0[idx]; }
};

template<typename E0, typename E1> requires (E0::arraySize == 3 && E1::arraySize == 3)
struct VecCrossProduct : public Impl::VectorExpr<typename E0::value_type, VecCrossProduct<E0, E1>> {
    typedef typename E0::value_type value_type;
    static constexpr size_t arraySize = E0::arraySize;

    const E0 &e0;
    const E1 &e1;

    constexpr VecCrossProduct(const E0 &e0, const E1 &e1)
      : e0(e0), e1(e1) {}

    [[nodiscard]] constexpr size_t size() const noexcept { return e0.size(); }

    [[nodiscard]] constexpr value_type operator[](size_t idx) const noexcept {
        if (idx == 0) return e0[1] * e1[2] - e0[2] * e1[1];
        else if (idx == 1) return e0[2] * e1[0] - e0[0] * e1[2];
        else return e0[0] * e1[1] - e0[1] * e1[0];
    }
};

}

template<typename T, size_t N>
struct Vector : public Impl::VectorExpr<T, Vector<T, N>> {
    typedef T value_type;
    static constexpr size_t arraySize = N;
    typedef Impl::Array<T, N> array_type;

    array_type impl{0};

    [[nodiscard]] auto begin() noexcept { return PtrIterator<T, N>(data()); }

    [[nodiscard]] auto end() noexcept { return PtrIterator<T, N>(data(), N); }

    [[nodiscard]] auto cbegin() const noexcept { return PtrIterator<const T, N>(data()); }

    [[nodiscard]] auto cend() const noexcept { return PtrIterator<const T, N>(data(), N); }

    operator const Impl::VectorExpr<T, Vector<T, N>> &() { return *this; }

    constexpr Vector() noexcept = default;

    template<typename E>
    constexpr Vector(const Impl::VectorExpr<T, E> &expr) noexcept {
        static_assert(Impl::VectorExpr<T, E>::arraySize == N);
        for (size_t i = 0; i < N; i++)
            impl[i] = expr[i];
    }

    constexpr Vector(array_type &&data) noexcept
      : impl(std::forward<array_type>(data)) {}

    constexpr Vector(const array_type &data) noexcept
      : impl(data) {}

    [[nodiscard]] constexpr size_t size() const noexcept { return N; }

    [[nodiscard]] constexpr T operator[](size_t idx) const noexcept { return impl[idx]; }

    [[nodiscard]] constexpr T &operator[](size_t idx) noexcept { return impl[idx]; }

    [[nodiscard]] constexpr T *data() noexcept { return impl.data; }

    [[nodiscard]] constexpr const T *data() const noexcept { return impl.data; }

#define v2vAssignEqualsFactory(oper) \
    template<typename E0> \
    requires (E0::arraySize == arraySize) \
    constexpr Vector &operator oper##= (const E0 &e0) { for (size_t i = 0; i < arraySize; i++) impl[i] oper##= e0[i]; return *this; }

    v2vAssignEqualsFactory(+)

    v2vAssignEqualsFactory(-)

    v2vAssignEqualsFactory(*)

    v2vAssignEqualsFactory(/)

#undef v2vAssignEqualsFactory

#define v2sAssignEqualsFactory(oper) \
    template<typename U> \
    constexpr Vector &operator oper##= (U s) { for (size_t i = 0; i < arraySize; i++) impl[i] oper##= s; return *this; }

    v2sAssignEqualsFactory(+)

    v2sAssignEqualsFactory(-)

    v2sAssignEqualsFactory(*)

    v2sAssignEqualsFactory(/)

#undef v2sAssignEqualsFactory

};

template<typename E0, typename E1>
constexpr inline Impl::VecSumExpr<E0, E1> operator+(const E0 &e0, const E1 &e1) { return {e0, e1}; }

template<typename E0, typename E1>
constexpr inline Impl::VecDiffExpr<E0, E1> operator-(const E0 &e0, const E1 &e1) { return {e0, e1}; }

template<typename E0, typename E1>
constexpr inline Impl::VecEWiseProductExpr<E0, E1> operator*(const E0 &e0, const E1 &e1) { return {e0, e1}; }

template<typename E0, typename U>
constexpr inline Impl::VecScalarProductExpr<E0> operator*(const E0 &e0, const U s) requires std::is_convertible_v<U, typename E0::value_type> {
    return {e0, static_cast<typename E0::value_type>(s)};
}

template<typename E0, typename E1>
constexpr inline Impl::VecEWiseDivisionExpr<E0, E1> operator/(const E0 &e0, const E1 &e1) { return {e0, e1}; }

template<typename E0, typename U>
constexpr inline Impl::VecScalarDivisionExpr<E0> operator/(const E0 &e0, const U s) requires std::is_convertible_v<U, typename E0::value_type> {
    return {e0, static_cast<typename E0::value_type>(s)};
}

template<typename E0>
constexpr inline Impl::VecNegationExpr<E0> operator-(const E0 &e0) { return {e0}; }

template<typename E0, typename E1>
constexpr inline typename E0::value_type Dot(const E0 &e0, const E1 &e1) {
    typename E0::value_type sum = 0;
    for (size_t i = 0; i < E0::arraySize; i++) {
        sum += e0[i] * e1[i];
    }
    return sum;
}

template<typename E0>
constexpr inline typename E0::value_type Magnitude(const E0 &e0) { return std::sqrt(Dot(e0, e0)); }

template<typename E0>
constexpr inline Impl::VecScalarDivisionExpr<E0> Normalized(const E0 &e0) { return {e0, Magnitude(e0)}; }

template<typename E0>
constexpr inline bool IsNormalized(const E0 &e0) {
    constexpr typename E0::value_type errorMargin = 0.00001L;
    return std::abs(Magnitude(e0) - typename E0::value_type(1)) <= errorMargin;
}

template<typename E0, typename E1>
constexpr inline Impl::VecCrossProduct<E0, E1> Cross(const E0 &e0, const E1 &e1) { return {e0, e1}; }

}

template<typename T, typename E>
struct fmt::formatter<Math::Impl::VectorExpr<T, E>> {
    constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<typename FormatContext>
    constexpr auto format(const Math::Impl::VectorExpr<T, E> &vec, FormatContext &ctx) -> decltype(ctx.out()) {
        return FormatInserter<decltype(ctx.out()), 0>(vec, fmt::format_to(ctx.out(), "("));
    }

    template<typename U, typename FormatContext>
    constexpr auto format(const T &vec, FormatContext &ctx) -> decltype(ctx.out()) {
        const Math::Impl::VectorExpr<T, E> &ref = vec;
        return format(ref);
    }

  private:
    static constexpr size_t size = Math::Impl::VectorExpr<T, E>::arraySize;

    template<typename C, std::size_t i = 0>
    [[nodiscard]] constexpr C FormatInserter(const Math::Impl::VectorExpr<T, E> &vec, C o) {
        if constexpr (i >= size) return o;
        else return FormatInserter<C, i + 1>(vec, fmt::format_to(o, (i + 1 == size) ? "{})" : "{}, ", vec[i]));
    }
};

template<typename U> requires std::is_convertible_v<const U &, const Math::Impl::VectorExpr<typename U::value_type, U> &>
struct fmt::formatter<U> {
    constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<typename FormatContext>
    constexpr auto format(const U &vec, FormatContext &ctx) -> decltype(ctx.out()) {
        const Math::Impl::VectorExpr<typename U::value_type, U> &expr = vec;
        return fmt::format_to(ctx.out(), "{}", expr);
    }
};
