#pragma once

#include "sampler.hpp"

namespace Gfx::Sampler {

class Whitted {
  public:
    Whitted() = default;

    ~Whitted() = default;

    [[nodiscard]] constexpr RGBSpectrum Sample(const Scene &scene, const Ray &ray, size_t depth = 0) const {
        if (depth >= nReflections) return {};

        const auto isectionOpt = scene.Intersect(ray);
        if (!isectionOpt) return {};
        const auto &isection = *isectionOpt;

#ifdef LIBGFX_WHITTED_DISPLAY_TCOUNT
        return RGBSpectrum{{static_cast<double>(std::clamp(isection.stats.traversals, 0ul, 255ul)) / 255., 0, 0}};
#endif

        const auto &mat = scene.GetMaterial(isection.matIndex);
        const bool goingIn = Math::Dot(isection.normal, ray.direction) < 0;
        const auto orientedNormal = isection.normal * (goingIn ? 1 : -1);
        const auto offsetIPoint = isection.intersectionPoint + orientedNormal * sensibleEps;

        RGBSpectrum omega{{.2, .2, .2}};
        for (const auto &light : pointLights) {
            const Point l = Math::Normalized(light.location - isection.intersectionPoint);
            const Ray lv{offsetIPoint, l};

            if (scene.Intersect(lv)) continue;

            omega += light.emittance * std::max<Real>(0., Math::Dot(l, orientedNormal));
        }

        omega *= mat.GetAlbedo(isection.uv);

        if (mat.metallic > 0) {
            omega *= (1. - mat.metallic);
            const auto reflected = ray.direction - orientedNormal * 2 * Math::Dot(ray.direction, orientedNormal);
            omega *= Sample(scene, Ray(offsetIPoint, reflected), depth + 1) * mat.metallic;
        }

        return omega;
    }

    static const size_t nReflections = 10;

    struct PointLight {
        Point location;
        RGBSpectrum emittance;
    };

    inline static const std::array pointLights{
      PointLight{
        .location{{0, 7, 0}},
        .emittance{{1, 1, 1}},
      },
    };

  private:
};

}
