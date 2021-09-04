#pragma once

#include <variant>

#include <gfx/spectrum.hpp>

namespace Gfx {

struct Material {
    struct AlbedoDirect {
        RGBSpectrum albedo;
    };

    struct AlbedoUVFunc {
        std::function<RGBSpectrum(Math::Vector<Real, 2>)> uvFunc;
    };

    typedef std::variant<AlbedoDirect, AlbedoUVFunc> AlbedoType;

    AlbedoType albedo;
};



}