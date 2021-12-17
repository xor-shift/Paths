#pragma once

namespace Paths::Image {

template<typename Alloc>
constexpr Image<Alloc>::Image() noexcept(noexcept(Alloc())) requires std::is_default_constructible_v<Alloc>
    : m_allocator({})
    , m_width(0)
    , m_height(0)
    , m_impl(nullptr) { }

template<typename Alloc>
constexpr Image<Alloc>::Image(const Alloc &alloc) noexcept(noexcept(Alloc(alloc)))
    : m_allocator(alloc)
    , m_width(0)
    , m_height(0)
    , m_impl(nullptr) { }

// copy and move

template<typename Alloc>
constexpr Image<Alloc>::Image(const Image &other) noexcept(
    noexcept(Alloc())) requires std::is_default_constructible_v<Alloc>
    : m_allocator({})
    , m_width(other.m_width)
    , m_height(other.m_height)
    , m_impl(m_allocator.allocate(size())) {
    std::copy(other.m_impl, other.m_impl + size() * 3, m_impl);
}

template<typename Alloc>
constexpr Image<Alloc>::Image(const Image &other, const Alloc &alloc) noexcept(
    noexcept(Alloc(alloc))) requires std::is_default_constructible_v<Alloc>
    : m_allocator(alloc)
    , m_width(other.m_width)
    , m_height(other.m_height)
    , m_impl(m_allocator.allocate(size())) {
    std::copy(other.m_impl, other.m_impl + size() * 3, m_impl);
}

template<typename Alloc>
constexpr Image<Alloc>::Image(Image &&other) noexcept(noexcept(Alloc())) requires std::is_default_constructible_v<Alloc>
    : m_allocator({})
    , m_width(other.m_width)
    , m_height(other.m_height)
    , m_impl(other.m_impl) {
    other.m_width = 0;
    other.m_height = 0;
    other.m_impl = nullptr;
}

template<typename Alloc>
constexpr Image<Alloc>::Image(Image &&other, const Alloc &alloc) noexcept(
    noexcept(Alloc(alloc))) requires std::is_default_constructible_v<Alloc>
    : m_allocator(alloc)
    , m_width(other.m_width)
    , m_height(other.m_height)
    , m_impl(other.m_impl) {
    other.m_width = 0;
    other.m_height = 0;
    other.m_impl = nullptr;
}

// size constructors

template<typename Alloc>
constexpr Image<Alloc>::Image(std::size_t width, std::size_t height) noexcept(
    noexcept(Alloc())) requires std::is_default_constructible_v<Alloc>
    : m_allocator({})
    , m_width(width)
    , m_height(height)
    , m_impl(m_allocator.allocate(size())) {
    std::fill(m_impl, m_impl + size(), value_type {});
}

template<typename Alloc>
constexpr Image<Alloc>::Image(std::size_t width, std::size_t height, const Alloc &alloc) noexcept(
    noexcept(Alloc(alloc)))
    : m_allocator(alloc)
    , m_width(width)
    , m_height(height)
    , m_impl(m_allocator.allocate(size())) {
    std::fill(m_impl, m_impl + size(), value_type {});
}

// ImageView constructors

template<typename Alloc>
constexpr Image<Alloc>::Image(ImageView other) noexcept(
    noexcept(Alloc())) requires std::is_default_constructible_v<Alloc>
    : m_allocator({})
    , m_width(other.m_width)
    , m_height(other.m_height)
    , m_impl(m_allocator.allocate(size())) {
    std::copy(other.m_impl, other.m_impl + size(), m_impl);
}

template<typename Alloc>
constexpr Image<Alloc>::Image(ImageView other, const Alloc &alloc) noexcept(noexcept(Alloc(alloc)))
    : m_allocator(alloc)
    , m_width(other.m_width)
    , m_height(other.m_height)
    , m_impl(m_allocator.allocate(size())) {
    std::copy(other.m_impl, other.m_impl + size(), m_impl);
}

}
