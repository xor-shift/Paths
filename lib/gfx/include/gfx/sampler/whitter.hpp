#pragma once

#include "sampler.hpp"

namespace Gfx::Sampler {

class Whitted {
  public:
    Whitted() = default;

    ~Whitted() = default;

    [[nodiscard]] constexpr RGBSpectrum Sample(const Scene &scene, const Ray &ray, size_t depth = 0) const {
        if (depth >= nReflections) return {};

        auto isection = scene.Intersect(ray);
        if (!isection) return {};
        isection->ComputeIntersectionPoint();

        const auto &mat = scene.GetMaterial(isection->matIndex);

        const bool goingIn = Math::Dot(isection->normal, ray.direction) < 0;

        const auto normal = isection->normal * (goingIn ? 1 : -1);

        RGBSpectrum omega{0};
        for (const auto &light : pointLights) {
            const auto l = Math::Normalized(light.location - isection->intersectionPoint);
            const auto cos = Math::Dot(l, normal);
            omega += light.emittance * std::max<Real>(0., cos);
        }

        omega *= mat.GetAlbedo(isection->uv);

        if (mat.metallic > 0) {
            omega *= (1. - mat.metallic);
            const auto reflected = ray.direction - normal * 2 * Math::Dot(ray.direction, normal);
            omega *= Sample(scene, Ray(isection->intersectionPoint, reflected), depth + 1) * mat.metallic;
        }

        return omega;
    }

    static const size_t nReflections = 10;

    struct PointLight {
        Point location;
        RGBSpectrum emittance;
    };

    inline static const std::vector<PointLight> pointLights{
      PointLight{
        .location{0, 7, 0},
        .emittance{2, 2, 2},
      },
    };

  private:
};

}
