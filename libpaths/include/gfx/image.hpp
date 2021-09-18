#pragma once

#include "gfx/spectrum.hpp"

namespace Gfx {

namespace Concepts {

template<typename T>
concept Kernel = requires (const T &k, RGBSpectrum c, size_t x, size_t y) {
    { std::invoke(k, c, x, y) } -> std::same_as<RGBSpectrum>;
};

}

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

    [[nodiscard]] const std::vector<RGBSpectrum> &Data() const noexcept { return data; }

    [[nodiscard]] std::vector<RGBSpectrum> &Data() noexcept { return data; }

    void ExportPNG(const std::string &filename, Real min = 0., Real max = 1.) const;

    /**
     * Exports the image as a linear EXR image. There are three supported types numbered from 0 to 2: half, float, uint
     * @param filename The filename to export to
     * @param type The type of EXR pixel format to use, invalid values will default to 0
     */
    void ExportEXR(const std::string &filename, size_t type = 0) const;

    template<Concepts::Kernel Kernel>
    void ApplyKernel(const Kernel &kern) {}

  private:
    size_t width{};
    size_t height{};
    std::vector<RGBSpectrum> data{};
};

}
