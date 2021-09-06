#pragma once

#include <type_traits>

template<typename T, size_t N>
struct PtrIterator {
    explicit PtrIterator(T *ptr, ptrdiff_t i = 0)
    : ptr(ptr)
    , i(i) {}

    /*template<typename U>
    friend constexpr auto operator<=>(const PtrIterator &lhs, const PtrIterator<U, N> &rhs) noexcept requires std::is_same_v<std::decay_t<T>, std::decay_t<U>> { return lhs.i <=> rhs.i; }*/

    template<typename U>
    friend constexpr bool operator==(const PtrIterator &lhs, const PtrIterator<U, N> &rhs) noexcept requires std::is_same_v<std::decay_t<T>, std::decay_t<U>> { return lhs.i == rhs.i; }

    template<typename U>
    friend constexpr bool operator>(const PtrIterator &lhs, const PtrIterator<U, N> &rhs) noexcept requires std::is_same_v<std::decay_t<T>, std::decay_t<U>> { return lhs.i > rhs.i; }

    template<typename U>
    friend constexpr bool operator<(const PtrIterator &lhs, const PtrIterator<U, N> &rhs) noexcept requires std::is_same_v<std::decay_t<T>, std::decay_t<U>> { return lhs.i < rhs.i; }

    constexpr PtrIterator &operator++() {
        ++i;
        return *this;
    }

    constexpr PtrIterator &operator--() {
        --i;
        return *this;
    }

    constexpr const PtrIterator operator++(int) {
        auto tmp = *this;
        ++i;
        return tmp;
    }

    constexpr const PtrIterator operator--(int) {
        auto tmp = *this;
        --i;
        return tmp;
    }

    constexpr T &operator*() { return ptr[i]; }

    constexpr T *operator->() { return &ptr[i]; }

    constexpr T &operator[](size_t v) { return ptr[i + v]; }

  private:
    T *ptr;
    ptrdiff_t i;
};
