#pragma once

#include "utils.hpp"

namespace Utils {

/// A (very) basic circular buffer
/// \tparam T Some type to store. Not all are suitable, if the copy or move
/// constructor (and in the case of emplace_back, any used constructor at the
/// time of calling) throws, it will result in UB.
/// \tparam Alloc
template<typename T, typename Alloc = std::allocator<T>>
class CircularBuffer { ///TODO: add support for throwing constructors
    typedef Alloc AllocType;
    typedef std::allocator_traits<AllocType> AllocTraits;

    const std::size_t bufferSize;

    AllocType allocator;

    T *buffer;
    std::size_t frontIndex = 0;
    std::size_t usedSize = 0;

    constexpr void CreateBuffer() {
        buffer = allocator.allocate(bufferSize);
    }

  public:
    typedef T *pointer;
    typedef const T *const_pointer;
    typedef T &reference;
    typedef const T &const_reference;

    constexpr explicit CircularBuffer(std::size_t size) noexcept(std::is_nothrow_default_constructible_v<AllocType>)
        : bufferSize(size), allocator(AllocType()), buffer(allocator.allocate(bufferSize)) {}

    constexpr explicit CircularBuffer(std::size_t size, const AllocType &allocator) noexcept(std::is_nothrow_default_constructible_v<AllocType>)
        : bufferSize(size), allocator(allocator), buffer(allocator.allocate(bufferSize)) {}

    constexpr ~CircularBuffer() noexcept {
        if constexpr (!std::is_trivially_destructible_v<T>)
            while (!empty()) pop_front();

        allocator.deallocate(buffer, bufferSize);
    }

    constexpr const_reference front() const noexcept { return buffer[frontIndex]; }

    constexpr const_reference back() const noexcept { return buffer[(frontIndex + usedSize - 1) % usedSize]; }

    constexpr reference front() noexcept { return ConstCastCallNoArg(reference, front); }

    constexpr reference back() noexcept { return ConstCastCallNoArg(reference, back); }

    [[nodiscard]] constexpr bool empty() const noexcept { return !usedSize; }

    [[nodiscard]] constexpr bool full() const noexcept { return !(bufferSize - usedSize); }

    constexpr void pop_back() noexcept {
        if (!usedSize) return;

        DestructIndex((frontIndex + usedSize - 1) % bufferSize);
        --usedSize;
    }

    constexpr void pop_front() noexcept {
        if (!usedSize) return;

        DestructIndex(frontIndex);
        ++frontIndex %= bufferSize;
        --usedSize;
    }

    template<typename ...Us>
    constexpr void emplace_back(Us &&...args) noexcept {
        if (full()) return;
        ConstructAt((frontIndex + usedSize++) % bufferSize, std::forward<Us &&>(args)...);
    }

    constexpr void push_back(const T &v) noexcept {
        if constexpr (std::is_trivially_copy_assignable_v<T>) {
            buffer[(frontIndex + usedSize++) % bufferSize] = v;
        } else emplace_back(v);
    }

    constexpr void push_back(T &&v) noexcept {
        if constexpr (std::is_trivially_move_assignable_v<T>) {
            buffer[(frontIndex + usedSize++) % bufferSize] = v;
        } else emplace_back(v);
    }

    template<typename ...Us>
    constexpr void emplace_front(Us &&...args) noexcept {
        if (full()) return;
        ConstructAt(frontIndex, std::forward<Us &&>(args)...);
        frontIndex = (frontIndex + bufferSize - 1) % bufferSize;
    }

    constexpr void push_front(const T &v) noexcept {
        if constexpr (std::is_trivially_copy_assignable_v<T>) {
            buffer[frontIndex] = v;
            frontIndex = Dec(frontIndex);
        } else emplace_front(v);
    }

    constexpr void push_front(T &&v) noexcept {
        if constexpr (std::is_trivially_move_assignable_v<T>) {
            buffer[frontIndex] = v;
            frontIndex = Dec(frontIndex);
        } else emplace_front(v);
    }

  private:
    template<typename ...Us>
    constexpr void ConstructAt(std::size_t index, Us &&...args) noexcept {
        new (&buffer[index]) T(std::forward<Us &&>(args)...);
        //allocator.construct(&buffer[index], std::forward<Us &&>(args)...);
    }

    constexpr void DestructIndex(std::size_t index) noexcept requires std::is_trivially_destructible_v<T> {}

    constexpr void DestructIndex(std::size_t index) noexcept requires (!std::is_trivially_destructible_v<T>) {
        //allocator.destruct(&buffer[index]);
        delete &buffer[index];
    }

    constexpr std::size_t Inc(std::size_t v) { return (v + 1) % bufferSize; }

    constexpr std::size_t Dec(std::size_t v) { return (v + bufferSize - 1) % bufferSize; }
};

}
