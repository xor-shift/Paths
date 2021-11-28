#pragma once

#include <concepts>

#include "sampler_wrapper.hpp"

namespace Gfx {

class AlbedoIntegrator : public SamplerWrapperIntegrator {
  public:
    ~AlbedoIntegrator() noexcept override = default;

  protected:
    [[nodiscard]] Color Sample(Ray ray, Scene &scene) const noexcept override {
        auto isection = scene.Intersect(ray);

        return isection ? scene.GetMaterial(isection->matIndex).albedo : Point{0, 0, 0};
    }
};

class WhittedIntegrator : public SamplerWrapperIntegrator {
  public:
    ~WhittedIntegrator() noexcept override = default;

    void AddDotLight(Point p, Point color) noexcept { dotLights.push_back({p, color}); }

  protected:
    [[nodiscard]] Color Sample(Ray ray, Scene &scene) const noexcept override { return SampleImpl(ray, scene, 0); }

  private:
    [[nodiscard]] Color SampleImpl(Ray ray, Scene &scene, std::size_t depth) const noexcept;

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