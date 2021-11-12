#pragma once

#include <type_traits>

#include <gfx/image/image.hpp>

namespace Gfx::Image {

typedef std::integral_constant<int, 0> PNGExporter;

template<>
struct Exporter<PNGExporter> {
    [[maybe_unused]] static bool Export(const std::string &filename, const Image &image);
};

}