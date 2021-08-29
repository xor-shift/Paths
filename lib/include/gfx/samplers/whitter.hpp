#pragma once

#include <gfx/concepts/integrator.hpp>

namespace Gfx::Sampler {

class Whitter {
  public:
    Whitter() = default;

    ~Whitter() = default;

    static constexpr bool monteCarlo = false;

    [[nodiscard]] RGBSpectrum Sample(const Scene &scene, const Ray &ray) const {
        auto isection = scene.Intersect(ray);
        if (!isection) return RGBSpectrum{0, 0, 0};
        else
            return std::visit([&isection]<typename A>(const A &a) -> RGBSpectrum {
                if constexpr(std::is_same_v<A, Material::AlbedoDirect>) {
                    return a.albedo;
                } else if constexpr (std::is_same_v<A, Material::AlbedoUVFunc>) {
                    return a.uvFunc(isection->uv);
                } else {
                    return RGBSpectrum{0, 0, 0};
                }
            }, scene.GetMaterial(isection->matIndex).albedo);
    }

  private:
};

}
