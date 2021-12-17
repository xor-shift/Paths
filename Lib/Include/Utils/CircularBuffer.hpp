#pragma once

#include "Utils.hpp"

namespace Utils {

/// A (very) basic circular buffer
/// \tparam T Some type to store. Not all are suitable, if the copy or move
/// constructor (and in the case of emplace_back, any used constructor at the
/// time of calling) throws, it will result in UB.
/// \tparam Alloc
template<typename T, typename Alloc = std::allocator<T>>
class CircularBuffer { /// TODO: add support for throwing constructors
    typedef Alloc AllocType;
    typedef std::allocator_traits<AllocType> AllocTraits;

    const std::size_t m_buffer_size;

    AllocType m_allocator;

    T *m_buffer;
    std::size_t m_front_index = 0;
    std::size_t m_used_size = 0;

    constexpr void create_buffer() { m_buffer = m_allocator.allocate(m_buffer_size); }

public:
    typedef T *pointer;
    typedef const T *const_pointer;
    typedef T &reference;
    typedef const T &const_reference;

    constexpr explicit CircularBuffer(std::size_t size) noexcept(std::is_nothrow_default_constructible_v<AllocType>)
        : m_buffer_size(size)
        , m_allocator(AllocType())
        , m_buffer(m_allocator.allocate(m_buffer_size)) { }

    constexpr explicit CircularBuffer(std::size_t size, const AllocType &allocator) noexcept(
        std::is_nothrow_default_constructible_v<AllocType>)
        : m_buffer_size(size)
        , m_allocator(allocator)
        , m_buffer(allocator.allocate(m_buffer_size)) { }

    constexpr ~CircularBuffer() noexcept {
        if constexpr (!std::is_trivially_destructible_v<T>)
            while (!empty())
                pop_front();

        m_allocator.deallocate(m_buffer, m_buffer_size);
    }

    constexpr const_reference front() const noexcept { return m_buffer[m_front_index]; }

    constexpr const_reference back() const noexcept {
        return m_buffer[(m_front_index + m_used_size - 1) % m_used_size];
    }

    constexpr reference front() noexcept { return CONST_CAST_CALL_NO_ARG(reference, front); }

    constexpr reference back() noexcept { return CONST_CAST_CALL_NO_ARG(reference, back); }

    [[nodiscard]] constexpr bool empty() const noexcept { return m_used_size == 0; }

    [[nodiscard]] constexpr bool full() const noexcept { return m_buffer_size - m_used_size == 0; }

    constexpr void pop_back() noexcept {
        if (empty())
            return;

        destruct_index((m_front_index + m_used_size - 1) % m_buffer_size);
        --m_used_size;
    }

    constexpr void pop_front() noexcept {
        if (empty())
            return;

        destruct_index(m_front_index);
        ++m_front_index %= m_buffer_size;
        --m_used_size;
    }

    template<typename... Us> constexpr void emplace_back(Us &&...args) noexcept {
        if (full())
            return;
        construct_at((m_front_index + m_used_size++) % m_buffer_size, std::forward<Us &&>(args)...);
    }

    constexpr void push_back(const T &v) noexcept {
        if constexpr (std::is_trivially_copy_assignable_v<T>) {
            m_buffer[(m_front_index + m_used_size++) % m_buffer_size] = v;
        } else
            emplace_back(v);
    }

    constexpr void push_back(T &&v) noexcept {
        if constexpr (std::is_trivially_move_assignable_v<T>) {
            m_buffer[(m_front_index + m_used_size++) % m_buffer_size] = v;
        } else
            emplace_back(v);
    }

    template<typename... Us> constexpr void emplace_front(Us &&...args) noexcept {
        if (full())
            return;
        construct_at(m_front_index, std::forward<Us &&>(args)...);
        m_front_index = (m_front_index + m_buffer_size - 1) % m_buffer_size;
    }

    constexpr void push_front(const T &v) noexcept {
        if constexpr (std::is_trivially_copy_assignable_v<T>) {
            m_buffer[m_front_index] = v;
            m_front_index = dec(m_front_index);
        } else
            emplace_front(v);
    }

    constexpr void push_front(T &&v) noexcept {
        if constexpr (std::is_trivially_move_assignable_v<T>) {
            m_buffer[m_front_index] = v;
            m_front_index = dec(m_front_index);
        } else
            emplace_front(v);
    }

private:
    template<typename... Us> constexpr void construct_at(std::size_t index, Us &&...args) noexcept {
        new (&m_buffer[index]) T(std::forward<Us &&>(args)...);
        // allocator.construct(&buffer[index], std::forward<Us &&>(args)...);
    }

    constexpr void destruct_index(std::size_t index) noexcept requires std::is_trivially_destructible_v<T> { }

    constexpr void destruct_index(std::size_t index) noexcept requires(!std::is_trivially_destructible_v<T>) {
        // allocator.destruct(&buffer[index]);
        delete &m_buffer[index];
    }

    constexpr std::size_t inc(std::size_t v) { return (v + 1) % m_buffer_size; }

    constexpr std::size_t dec(std::size_t v) { return (v + m_buffer_size - 1) % m_buffer_size; }
};

}
