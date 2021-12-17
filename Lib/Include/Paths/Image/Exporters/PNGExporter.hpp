#pragma once

#include <type_traits>

#include "Paths/Image/Image.hpp"

namespace Paths::Image {

using PNGExporter = std::integral_constant<int, 0>;

template<> struct Exporter<PNGExporter> {
    [[maybe_unused]] static bool export_to(const std::string &filename, ImageView image);
};

}
