#pragma once

#include "sampler_wrapper.hpp"

namespace Gfx {

struct MCIntegrator : public SamplerWrapperIntegrator {
  public:
    ~MCIntegrator() noexcept override = default;

    void AddDotLight(Point p, Point color) noexcept { dotLights.push_back({p, color}); }

  protected:
    [[nodiscard]] Color Sample(Ray ray, Scene &scene) const noexcept override;

  private:
    struct DotLight {
        Point p;
        Point color;
    };

    std::vector<DotLight> dotLights{
      {{-10, 10, -2.5}, {1, 0, 0}},
      {{10,  10, -2.5}, {0, 0, 1}},
    };
};

}
