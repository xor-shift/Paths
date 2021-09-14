#pragma once

#include "gfx/spectrum.hpp"

namespace Gfx {

class Image {
  public:
    Image() = default;

    explicit Image(size_t width, size_t height, RGBSpectrum fill = {{0, 0, 0}})
      : width(width)
        , height(height) {
        data = std::vector<RGBSpectrum>(width * height, fill);
    }

    ~Image() = default;

    [[nodiscard]] RGBSpectrum &At(size_t x, size_t y) & noexcept { return data[y * width + x]; }

    [[nodiscard]] const RGBSpectrum &At(size_t x, size_t y) const & noexcept { return data[y * width + x]; }

    [[nodiscard]] size_t Width() const noexcept { return width; }

    [[nodiscard]] size_t Height() const noexcept { return height; }

    void Fill(RGBSpectrum color) { for (auto &c: data) c = color; }

  private:
    size_t width;
    size_t height;
    std::vector<RGBSpectrum> data{};
};

}
