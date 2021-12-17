#pragma once

#include <cmath>
#include <cstring>
#include <string>
#include <type_traits>

#include "Paths/Color.hpp"
#include "Utils/Utils.hpp"

namespace Paths::Image {

struct ImageView {
    using value_type = Color;
    using reference = value_type &;
    using const_reference = const value_type &;

    using iterator = value_type *;
    using const_iterator = const value_type *;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    reference at(std::size_t x, std::size_t y) noexcept { return CONST_CAST_CALL(reference, at, x, y); }

    [[nodiscard]] constexpr const_reference at(std::size_t x, std::size_t y) const noexcept {
        return m_impl[y * m_width + x];
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept { return m_width * m_height; }

    [[nodiscard]] constexpr std::size_t max_size() const noexcept { return m_width * m_height; }

    [[nodiscard]] constexpr iterator begin() const noexcept { return m_impl; }

    [[nodiscard]] constexpr iterator end() const noexcept { return m_impl + size(); }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return m_impl; }

    [[nodiscard]] constexpr const_iterator cend() const noexcept { return m_impl + size(); }

    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator { cend() };
    }

    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator { cbegin() };
    }

    [[nodiscard]] constexpr const value_type *data() const noexcept { return m_impl; }

    [[nodiscard]] constexpr value_type &operator[](std::size_t i) const noexcept { return m_impl[i]; }

    const std::size_t m_width { 0 }, m_height { 0 };
    value_type *const m_impl { nullptr };
};

template<typename Alloc = std::allocator<Color>> class Image {
    Alloc m_allocator;

public:
    using value_type = Color;
    typedef value_type &reference;
    typedef const value_type &const_reference;

    using iterator = value_type *;
    using const_iterator = const value_type *;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // empty constructors

    constexpr Image() noexcept(noexcept(Alloc())) requires std::is_default_constructible_v<Alloc>;

    constexpr explicit Image(const Alloc &alloc) noexcept(noexcept(Alloc(alloc)));

    // copy and move

    constexpr Image(const Image &other) noexcept(noexcept(Alloc())) requires std::is_default_constructible_v<Alloc>;

    constexpr Image(const Image &other, const Alloc &alloc) noexcept(
        noexcept(Alloc(alloc))) requires std::is_default_constructible_v<Alloc>;

    constexpr Image(Image &&other) noexcept(noexcept(Alloc())) requires std::is_default_constructible_v<Alloc>;

    constexpr Image(Image &&other, const Alloc &alloc) noexcept(
        noexcept(Alloc(alloc))) requires std::is_default_constructible_v<Alloc>;

    // size constructors

    constexpr Image(std::size_t width, std::size_t height) noexcept(
        noexcept(Alloc())) requires std::is_default_constructible_v<Alloc>;

    constexpr Image(std::size_t width, std::size_t height, const Alloc &alloc) noexcept(noexcept(Alloc(alloc)));

    // ImageView constructors

    constexpr explicit Image(ImageView other) noexcept(
        noexcept(Alloc())) requires std::is_default_constructible_v<Alloc>;

    constexpr explicit Image(ImageView other, const Alloc &alloc) noexcept(noexcept(Alloc(alloc)));

    constexpr ~Image() noexcept { m_allocator.deallocate(m_impl, size()); }

    constexpr reference at(std::size_t x, std::size_t y) noexcept { return CONST_CAST_CALL(reference, at, x, y); }

    [[nodiscard]] constexpr const_reference at(std::size_t x, std::size_t y) const noexcept {
        return m_impl[y * m_width + x];
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept { return m_width * m_height; }

    [[nodiscard]] constexpr std::size_t max_size() const noexcept { return size(); }

    constexpr iterator begin() noexcept { return m_impl; }

    constexpr iterator end() noexcept { return m_impl + size(); }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return m_impl; }

    [[nodiscard]] constexpr const_iterator cend() const noexcept { return m_impl + size(); }

    constexpr const_reverse_iterator rbegin() noexcept { return const_reverse_iterator { end() }; }

    constexpr const_reverse_iterator rend() noexcept { return const_reverse_iterator { begin() }; }

    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator { cend() };
    }

    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator { cbegin() };
    }

    constexpr value_type *data() noexcept { return m_impl; }

    [[nodiscard]] constexpr const value_type *data() const noexcept { return m_impl; }

    // helper functions

    constexpr void resize(std::size_t w, std::size_t h) noexcept {
        value_type *impl_new = m_allocator.allocate(w * h);
        std::fill(impl_new, impl_new + w * h, value_type {});

        const std::size_t mw = std::min(w, m_width);

        for (std::size_t y = 0; y < std::min(h, m_height); y++)
            std::copy(m_impl + (y * m_width), m_impl + (y * m_width + mw), impl_new);

        if (m_impl)
            m_allocator.deallocate(m_impl, size());

        m_impl = impl_new;
        m_width = w;
        m_height = h;
    }

    constexpr void fill(value_type c) noexcept { std::fill(begin(), end(), c); }

    constexpr void fill(ColorChannelType v) noexcept { fill({ v, v, v }); }

    constexpr void draw_filled_rect(
        std::pair<std::size_t, std::size_t> pos, std::pair<std::size_t, std::size_t> dims, value_type c) noexcept {
        if (pos.first + dims.first > m_width)
            dims.first = m_width - pos.first;
        if (pos.second + dims.second > m_height)
            dims.first = m_width - pos.first;

        for (std::size_t y = 0; y < dims.second; y++)
            for (std::size_t x = 0; x < dims.first; x++)
                at(x + dims.first, y + dims.second) = c;
    }

    std::size_t m_width { 0 }, m_height { 0 };
    value_type *m_impl { nullptr };

    constexpr explicit operator ImageView() const noexcept {
        return ImageView {
            .m_width = m_width,
            .m_height = m_height,
            .m_impl = m_impl,
        };
    }
};

template<typename E> struct Exporter {
    [[maybe_unused]] static bool export_to(const std::string &, ImageView) {
        static_assert(!std::is_same_v<E, E>, "unsupported image exporter");
        return false;
    }
};

} // namespace Paths::Image

#include "Image.ipp"
