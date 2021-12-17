#pragma once

#include <type_traits>

namespace Utils {

template<typename T, size_t N> struct PtrIterator {
    typedef std::random_access_iterator_tag iterator_category;
    typedef T value_type;
    typedef T &reference;
    typedef std::ptrdiff_t difference_type;

    constexpr explicit PtrIterator(T *ptr, ptrdiff_t i = 0)
        : m_ptr(ptr)
        , m_i(i) { }

    /*template<typename U>
    friend constexpr auto operator<=>(const PtrIterator &lhs, const PtrIterator<U, N> &rhs) noexcept requires
    std::is_same_v<std::decay_t<T>, std::decay_t<U>> { return lhs.i <=> rhs.i; }*/

    template<typename U>
    friend constexpr bool operator==(const PtrIterator &lhs,
        const PtrIterator<U, N> &rhs) noexcept requires std::is_same_v<std::decay_t<T>, std::decay_t<U>> {
        return lhs.m_i == rhs.m_i;
    }

    template<typename U>
    friend constexpr bool operator>(const PtrIterator &lhs,
        const PtrIterator<U, N> &rhs) noexcept requires std::is_same_v<std::decay_t<T>, std::decay_t<U>> {
        return lhs.m_i > rhs.m_i;
    }

    template<typename U>
    friend constexpr bool operator<(const PtrIterator &lhs,
        const PtrIterator<U, N> &rhs) noexcept requires std::is_same_v<std::decay_t<T>, std::decay_t<U>> {
        return lhs.m_i < rhs.m_i;
    }

    constexpr PtrIterator &operator++() noexcept {
        ++m_i;
        return *this;
    }

    constexpr PtrIterator &operator--() noexcept {
        --m_i;
        return *this;
    }

    constexpr const PtrIterator operator++(int) noexcept {
        auto tmp = *this;
        ++m_i;
        return tmp;
    }

    constexpr const PtrIterator operator--(int) noexcept {
        auto tmp = *this;
        --m_i;
        return tmp;
    }

    constexpr T &operator*() noexcept { return m_ptr[m_i]; }

    constexpr T *operator->() noexcept { return &m_ptr[m_i]; }

    constexpr T &operator[](size_t v) noexcept { return m_ptr[m_i + v]; }

    constexpr PtrIterator &operator+=(difference_type n) noexcept {
        m_i += n;
        return *this;
    }

    constexpr PtrIterator &operator-=(difference_type n) noexcept { return *this += -n; }

    constexpr friend PtrIterator operator+(const PtrIterator &it, difference_type n) noexcept {
        return PtrIterator(it.m_ptr, it.m_i + n);
    }

    constexpr friend PtrIterator operator+(difference_type n, const PtrIterator &it) noexcept {
        return PtrIterator(it.m_ptr, it.m_i + n);
    }

    constexpr friend PtrIterator operator-(const PtrIterator &lhs, difference_type n) noexcept {
        PtrIterator t = lhs;
        t.m_i -= n;
        return t;
    }

    constexpr friend difference_type operator-(const PtrIterator &lhs, const PtrIterator &rhs) noexcept {
        return lhs.m_i - rhs.m_i;
    }

private:
    T *m_ptr;
    ptrdiff_t m_i;
};

}
