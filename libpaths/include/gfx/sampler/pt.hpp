#pragma once

#include <maths/rand.hpp>
#include "sampler.hpp"

namespace Gfx::Sampler {

[[nodiscard]] inline Point OffsetIntersectionPoint(const Intersection &isection) {
    return isection.intersectionPoint + isection.orientedNormal * EpsilonVector;
}

/**
 * Scatters a ray randomly in the hemisphere of the face normal. Takes in no ray argument, heh
 * @param isection
 * @return
 */
[[nodiscard]] inline Ray RandScatter(const Intersection &isection) noexcept {
    const auto dir = Math::D3RandomUnitVector();

    return {
      OffsetIntersectionPoint(isection),
      Math::Dot(dir, isection.orientedNormal) >= 0 ? dir : -dir,
    };
}

[[nodiscard]] inline Ray Reflect(const Ray &ray, const Intersection &isection) noexcept {
    return {
      OffsetIntersectionPoint(isection),
      ray.direction - isection.orientedNormal * Math::Dot(ray.direction, isection.orientedNormal) * 2.,
    };
}

[[nodiscard]] inline Ray Refract(const Ray &ray, const Intersection &isection, const Material &material) {}

class PT {
  public:
    PT() = default;

    ~PT() = default;

    [[nodiscard]] RGBSpectrum Sample(const Scene &scene, const Ray &ray) const {
        RGBSpectrum wO{{0, 0, 0}};
        RGBSpectrum curA{{1, 1, 1}};

        Ray currentRay = ray;

        for (size_t depth = 0;; depth++) {
            if (depth > maxDepth) {
                if (Math::RandomDouble() > .8) break;
            }

            const auto isectionOpt = scene.Intersect(currentRay);
            if (!isectionOpt) break;
            const auto &isection = *isectionOpt;
            const auto &mat = scene.GetMaterial(isection.matIndex);

            wO += mat.emittance * curA;
            curA *= mat.GetAlbedo(isection.uv);

            if (true) {
                currentRay = RandScatter(isection);
            } else {
                if (Math::RandomDouble() > 0.8)
                    currentRay = RandScatter(isection);
                else
                    currentRay = Reflect(currentRay, isection);
            }
        }

        return wO;
    }

    static const size_t maxDepth = 6;

  private:

    static RGBSpectrum SampleOne(const Scene &scene, const Ray &ray) { return {{}}; }
};

}
