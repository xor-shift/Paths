#pragma once

#include <type_traits>

namespace Utils {

template<typename T, size_t N>
struct PtrIterator {
    typedef std::random_access_iterator_tag iterator_category;
    typedef T value_type;
    typedef T &reference;
    typedef std::ptrdiff_t difference_type;

    constexpr explicit PtrIterator(T *ptr, ptrdiff_t i = 0)
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

    constexpr PtrIterator &operator++() noexcept {
        ++i;
        return *this;
    }

    constexpr PtrIterator &operator--() noexcept {
        --i;
        return *this;
    }

    constexpr const PtrIterator operator++(int) noexcept {
        auto tmp = *this;
        ++i;
        return tmp;
    }

    constexpr const PtrIterator operator--(int) noexcept {
        auto tmp = *this;
        --i;
        return tmp;
    }

    constexpr T &operator*() noexcept { return ptr[i]; }

    constexpr T *operator->() noexcept { return &ptr[i]; }

    constexpr T &operator[](size_t v) noexcept { return ptr[i + v]; }

    constexpr PtrIterator &operator+=(difference_type n) noexcept {
        i += n;
        return *this;
    }

    constexpr PtrIterator &operator-=(difference_type n) noexcept {
        return *this += -n;
    }

    constexpr friend PtrIterator operator+(const PtrIterator &it, difference_type n) noexcept {
        return PtrIterator(it.ptr, it.i + n);
    }

    constexpr friend PtrIterator operator+(difference_type n, const PtrIterator &it) noexcept {
        return PtrIterator(it.ptr, it.i + n);
    }

    constexpr friend PtrIterator operator-(const PtrIterator &lhs, difference_type n) noexcept {
        PtrIterator t = lhs;
        t.i -= n;
        return t;
    }

    constexpr friend difference_type operator-(const PtrIterator &lhs, const PtrIterator &rhs) noexcept {
        return lhs.i - rhs.i;
    }

  private:
    T *ptr;
    ptrdiff_t i;
};

}
