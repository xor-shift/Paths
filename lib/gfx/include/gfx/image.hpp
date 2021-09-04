#pragma once

#include "gfx/spectrum.hpp"

namespace Gfx {

class Image {
  public:
    Image() = default;

    explicit Image(Math::Vector<std::size_t, 2> dimensions, RGBSpectrum fill = {0,0,0})
        : dimensions(dimensions) {
        data = std::vector<RGBSpectrum>(dimensions[0] * dimensions[1], fill);
    }

    ~Image() = default;

    [[nodiscard]] RGBSpectrum &At(Math::Vector<std::size_t, 2> loc) & noexcept { return data[loc[1] * dimensions[0] + loc[0]]; }

    [[nodiscard]] const RGBSpectrum &At(Math::Vector<std::size_t, 2> loc) const & noexcept { return data[loc[1] * dimensions[0] + loc[0]]; }

    [[nodiscard]] std::size_t Height() const noexcept { return dimensions[1]; }

    [[nodiscard]] std::size_t Width() const noexcept { return dimensions[0]; }

    void Fill(RGBSpectrum color) { for (auto &c : data) c = color; }

    Math::Vector<std::size_t, 2> dimensions{0, 0};
    std::vector<RGBSpectrum> data{};
};

}
