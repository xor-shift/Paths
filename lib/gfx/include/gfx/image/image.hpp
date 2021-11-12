#pragma once

#include <cmath>
#include <cstring>
#include <string>
#include <type_traits>

#include <utils/utils.hpp>

#include "color.hpp"

namespace Gfx::Image {

class Image {
  public:
    typedef Color value_type;
    typedef value_type &reference;
    typedef const value_type &const_reference;

    typedef value_type *iterator;
    typedef const value_type *const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef const std::reverse_iterator<iterator> const_reverse_iterator;

    constexpr Image() noexcept = delete;

    constexpr Image(std::size_t width, std::size_t height) noexcept
        : width(width), height(height), impl(new value_type[width * height]) {}

    constexpr ~Image() noexcept {
        delete[] impl;
    }

    constexpr Image(const Image &other) noexcept
        : width(other.width), height(other.height), impl(new value_type[width * height]) {
        std::copy(other.impl, other.impl + size() * 3, impl);
    }

    constexpr Image(Image &&other) noexcept
        : width(other.width), height(other.height), impl(other.impl) {}

    [[nodiscard]] reference At(std::size_t x, std::size_t y) noexcept { return ConstCastCall(reference, At, x, y); }

    [[nodiscard]] constexpr const_reference At(std::size_t x, std::size_t y) const noexcept { return impl[y * width + x]; }

    [[nodiscard]] constexpr std::size_t size() const noexcept { return width * height; }

    [[nodiscard]] constexpr iterator begin() noexcept { return impl; }

    [[nodiscard]] constexpr iterator end() noexcept { return impl + size(); }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return impl; }

    [[nodiscard]] constexpr const_iterator cend() const noexcept { return impl + size(); }

    [[nodiscard]] constexpr value_type *data() noexcept { return impl; }

    [[nodiscard]] constexpr const value_type *data() const noexcept { return impl; }

    //helper functions

    constexpr void Fill(Color c) noexcept { std::fill(begin(), end(), c); }

    constexpr void Fill(ChannelType v) noexcept { Fill(Color{{v, v, v}}); }

    constexpr void FilledRect(std::pair<std::size_t, std::size_t> pos, std::pair<std::size_t, std::size_t> dims, Color c) noexcept {
        if (pos.first + dims.first > width) dims.first = width - pos.first;
        if (pos.second + dims.second > height) dims.first = width - pos.first;

        for (std::size_t y = 0; y < dims.second; y++)
            for (std::size_t x = 0; x < dims.first; x++)
                At(x + dims.first, y + dims.second) = c;
    }

    const std::size_t width{0}, height{0};

  private:
    value_type *const impl{nullptr};
};

template<typename E>
struct Exporter {
    [[maybe_unused]] static bool Export(const std::string &filename, const Image &image) { static_assert(!std::is_same_v<E, E>, "unsupported image exporter"); }
};

}