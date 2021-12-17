#pragma once

#include <type_traits>

#include "Paths/Image/Image.hpp"

namespace Paths::Image {

using EXRExporterF16 = std::integral_constant<int, 1>;
using EXRExporterF32 = std::integral_constant<int, 2>;
using EXRExporterU32 = std::integral_constant<int, 3>;

template<> struct Exporter<EXRExporterF16> {
    [[maybe_unused]] static bool export_to(const std::string &filename, ImageView image);
};

template<> struct Exporter<EXRExporterF32> {
    [[maybe_unused]] static bool export_to(const std::string &filename, ImageView image);
};

template<> struct Exporter<EXRExporterU32> {
    [[maybe_unused]] static bool export_to(const std::string &filename, ImageView image);
};

}
