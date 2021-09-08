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

        for (size_t depth = 0; depth < maxDepth; depth++) {
            auto isectionOpt = scene.Intersect(ray);
            if (!isectionOpt) break;
            const auto isection = *isectionOpt;


        }

        return {};
    }

    static const size_t maxDepth = 10;

  private:

    static RGBSpectrum SampleOne(const Scene &scene, const Ray &ray) { return {{}}; }
};

}
