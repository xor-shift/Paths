#pragma once

#include "sampler_wrapper.hpp"

namespace Gfx {

class WhittedIntegrator : public SamplerWrapperIntegrator {
  public:
    ~WhittedIntegrator() noexcept override = default;

    void AddDotLight(Point p, Point color) noexcept { dotLights.push_back({p, color}); }

  protected:
    [[nodiscard]] Color Sample(Ray ray, Scene &scene) const noexcept override;

  private:
    [[nodiscard]] Color SampleImpl(Ray ray, Scene &scene, std::size_t depth, std::size_t &boundChecks, std::size_t &shapeChecks) const noexcept;

    struct DotLight {
        Point p;
        Point color;
    };

    //Point ambientLight {0.1, 0.1, 0.1};
    Point ambientLight{};

    std::vector<DotLight> dotLights{
      {{-10, 10, -2.5}, {1, 1, 1}},
      {{10,  10, -2.5}, {1, 1, 1}},
    };
};

}
