#pragma once

#include <maths/rand.hpp>
#include "sampler.hpp"

namespace Gfx::Sampler {

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

            auto isectionOpt = scene.Intersect(currentRay);
            if (!isectionOpt) break;
            const auto isection = *isectionOpt;
            const auto &mat = scene.GetMaterial(isection.matIndex);
            const bool goingIn = Math::Dot(isection.normal, currentRay.direction) < 0;
            const auto orientedNormal = isection.normal * (goingIn ? 1 : -1);

            wO += mat.emittance * curA;
            curA *= mat.GetAlbedo(isection.uv);

            auto dir = Math::D3RandomUnitVector();
            if (Math::Dot(dir, orientedNormal) < 0) dir = -dir;

            currentRay = Ray(isection.intersectionPoint + orientedNormal * EpsilonVector, dir);
        }

        return wO;
    }

    static const size_t maxDepth = 6;

  private:

    static RGBSpectrum SampleOne(const Scene &scene, const Ray &ray) { return {{}}; }
};

}
