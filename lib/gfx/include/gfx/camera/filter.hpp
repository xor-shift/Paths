#pragma once

#include <gfx/definitions.hpp>
#include <gfx/spectrum.hpp>
#include <gfx/image.hpp>

namespace Gfx {

namespace Concepts {

template<typename T>
concept BasicFilterUnary = requires(const T &f, RGBSpectrum s) {
    { f(s) } -> std::same_as<RGBSpectrum>;
};

template<typename T>
concept BasicFilterBinary = requires(const T &f, RGBSpectrum s0, RGBSpectrum s1) {
    { f(s0, s1) } -> std::same_as<RGBSpectrum>;
};

template<typename T, size_t N>
concept BasicFilterNary = requires(const T &f, std::array<RGBSpectrum, N> s) {
    { f<N>(s) } -> std::same_as<RGBSpectrum>;
};

}

namespace Filters::Basic {

struct Combine {
    RGBSpectrum operator()(RGBSpectrum s0, RGBSpectrum s1) {
        return (s0 + s1) / 2.;
    }
};

struct PassThrough {
    RGBSpectrum operator()(RGBSpectrum s) {
        return s;
    }
};

template<size_t N>
struct PickNth {
    template<size_t M>
    RGBSpectrum operator()(std::array<RGBSpectrum, M> s) {
        return s[N];
    }
};

}

}
