#pragma once

#include <type_traits>

#include <gfx/image/image.hpp>

namespace Gfx::Image {

typedef std::integral_constant<int, 1> EXRExporterF16;
typedef std::integral_constant<int, 2> EXRExporterF32;
typedef std::integral_constant<int, 3> EXRExporterU32;

template<>
struct Exporter<EXRExporterF16> {
    [[maybe_unused]] static bool Export(const std::string &filename, const Image &image);
};

template<>
struct Exporter<EXRExporterF32> {
    [[maybe_unused]] static bool Export(const std::string &filename, const Image &image);
};

template<>
struct Exporter<EXRExporterU32> {
    [[maybe_unused]] static bool Export(const std::string &filename, const Image &image);
};

}