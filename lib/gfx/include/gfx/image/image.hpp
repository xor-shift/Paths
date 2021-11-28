#pragma once

#include <cmath>
#include <cstring>
#include <string>
#include <type_traits>

#include <gfx/color.hpp>
#include <utils/utils.hpp>

namespace Gfx::Image {

struct ImageView {
    typedef Color value_type;
    typedef value_type &reference;
    typedef const value_type &const_reference;

    typedef value_type *iterator;
    typedef const value_type *const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    [[nodiscard]] reference At(std::size_t x, std::size_t y) noexcept { return ConstCastCall(reference, At, x, y); }

    [[nodiscard]] constexpr const_reference At(std::size_t x, std::size_t y) const noexcept { return impl[y * width + x]; }

    [[nodiscard]] constexpr std::size_t size() const noexcept { return width * height; }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return impl; }

    [[nodiscard]] constexpr const_iterator cend() const noexcept { return impl + size(); }

    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator{cend()}; }

    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator{cbegin()}; }

    [[nodiscard]] constexpr const value_type *data() const noexcept { return impl; }

    const std::size_t width{0}, height{0};
    const value_type *const impl{nullptr};
};

template<typename Alloc = std::allocator<Color>>
class Image {
    Alloc allocator;

  public:
    typedef Color value_type;
    typedef value_type &reference;
    typedef const value_type &const_reference;

    typedef value_type *iterator;
    typedef const value_type *const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    //empty constructors

    constexpr Image() noexcept(noexcept(Alloc())) requires std::is_default_constructible_v<Alloc>
      : allocator({}), width(0), height(0), impl(nullptr) {}

    constexpr explicit Image(const Alloc &alloc) noexcept(noexcept(Alloc(allocator)))
      : allocator(allocator), width(0), height(0), impl(nullptr) {}

    //copy and move

    constexpr Image(const Image &other) noexcept(noexcept(Alloc())) requires std::is_default_constructible_v<Alloc>
      : allocator({}), width(other.width), height(other.height), impl(allocator.allocate(size())) { std::copy(other.impl, other.impl + size() * 3, impl); }

    constexpr Image(const Image &other, const Alloc &alloc) noexcept(noexcept(Alloc(alloc))) requires std::is_default_constructible_v<Alloc>
      : allocator(alloc), width(other.width), height(other.height), impl(allocator.allocate(size())) { std::copy(other.impl, other.impl + size() * 3, impl); }

    constexpr Image(Image &&other) noexcept(noexcept(Alloc())) requires std::is_default_constructible_v<Alloc>
      : allocator({}), width(other.width), height(other.height), impl(other.impl) {
        other.width = 0;
        other.height = 0;
        other.impl = nullptr;
    }

    constexpr Image(Image &&other, const Alloc &alloc) noexcept(noexcept(Alloc(alloc))) requires std::is_default_constructible_v<Alloc>
      : allocator(alloc), width(other.width), height(other.height), impl(other.impl) {
        other.width = 0;
        other.height = 0;
        other.impl = nullptr;
    }

    //size constructors

    constexpr Image(std::size_t width, std::size_t height) noexcept(noexcept(Alloc())) requires std::is_default_constructible_v<Alloc>
      : allocator({}), width(width), height(height), impl(allocator.allocate(size())) { std::fill(impl, impl + size(), {}); }

    constexpr Image(std::size_t width, std::size_t height, const Alloc &alloc) noexcept(noexcept(Alloc(alloc)))
      : allocator(alloc), width(width), height(height), impl(allocator.allocate(size())) { std::fill(impl, impl + size(), {}); }

    //ImageView constructors

    constexpr explicit Image(ImageView other) noexcept(noexcept(Alloc())) requires std::is_default_constructible_v<Alloc>
      : allocator({}), width(width), height(height), impl(allocator.allocate(size())) { std::copy(other.impl, other.impl + size() * 3, impl); }

    constexpr explicit Image(ImageView other, const Alloc &alloc) noexcept(noexcept(Alloc(alloc)))
      : allocator(alloc), width(width), height(height), impl(allocator.allocate(size())) { std::copy(other.impl, other.impl + size() * 3, impl); }

    constexpr ~Image() noexcept {
        allocator.deallocate(impl, size());
    }

    [[nodiscard]] constexpr reference At(std::size_t x, std::size_t y) noexcept { return ConstCastCall(reference, At, x, y); }

    [[nodiscard]] constexpr const_reference At(std::size_t x, std::size_t y) const noexcept { return impl[y * width + x]; }

    [[nodiscard]] constexpr std::size_t size() const noexcept { return width * height; }

    [[nodiscard]] constexpr iterator begin() noexcept { return impl; }

    [[nodiscard]] constexpr iterator end() noexcept { return impl + size(); }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return impl; }

    [[nodiscard]] constexpr const_iterator cend() const noexcept { return impl + size(); }

    [[nodiscard]] constexpr const_reverse_iterator crbegin() noexcept { return const_reverse_iterator{end()}; }

    [[nodiscard]] constexpr const_reverse_iterator crend() noexcept { return const_reverse_iterator{begin()}; }

    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator{cend()}; }

    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator{cbegin()}; }

    [[nodiscard]] constexpr value_type *data() noexcept { return impl; }

    [[nodiscard]] constexpr const value_type *data() const noexcept { return impl; }

    //helper functions

    constexpr void Resize(std::size_t w, std::size_t h) noexcept {
        value_type *implNew = allocator.allocate(w * h);
        std::fill(implNew, implNew + w * h, value_type{});

        const std::size_t mw = std::min(w, width);

        for (std::size_t y = 0; y < std::min(h, height); y++)
            std::copy(impl + (y * width), impl + (y * width + mw), implNew);

        if (impl) allocator.deallocate(impl, size());

        impl = implNew;
        width = w;
        height = h;
    }

    constexpr void Fill(Color c) noexcept { std::fill(begin(), end(), c); }

    constexpr void Fill(ColorChannelType v) noexcept { Fill({v, v, v}); }

    constexpr void FilledRect(std::pair<std::size_t, std::size_t> pos, std::pair<std::size_t, std::size_t> dims, Color c) noexcept {
        if (pos.first + dims.first > width) dims.first = width - pos.first;
        if (pos.second + dims.second > height) dims.first = width - pos.first;

        for (std::size_t y = 0; y < dims.second; y++)
            for (std::size_t x = 0; x < dims.first; x++)
                At(x + dims.first, y + dims.second) = c;
    }

    std::size_t width{0}, height{0};
    value_type *impl{nullptr};

    constexpr explicit operator ImageView() noexcept {
        return ImageView{
          .width = width,
          .height = height,
          .impl = impl,
        };
    }
};

template<typename E>
struct Exporter {
    [[maybe_unused]] static bool Export(const std::string &filename, ImageView image) { static_assert(!std::is_same_v<E, E>, "unsupported image exporter"); return false;}
};

}
