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

    [[nodiscard]] constexpr RGBSpectrum GetAlbedo(const Math::Vector<Real, 2> &uv) const noexcept {
        return std::visit([&uv]<typename A>(const A &a) -> RGBSpectrum {
            if constexpr(std::is_same_v<A, Material::AlbedoDirect>) {
                return a.albedo;
            } else if constexpr (std::is_same_v<A, Material::AlbedoUVFunc>) {
                return a.uvFunc(uv);
            } else {
                return RGBSpectrum{{0, 0, 0}};
            }
        }, albedo);
    }

    const AlbedoType albedo;
    const RGBSpectrum emittance{{0, 0, 0}};
    const Real metallic{0};
    const Real roughness{0};
};


}