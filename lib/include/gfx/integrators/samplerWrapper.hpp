#pragma once

#include <thread>

#include <gfx/concepts/integrator.hpp>
#include <gfx/image.hpp>
#include <gfx/scene.hpp>
#include <math/math.hpp>

namespace Gfx {

template<Concepts::Sampler Sampler>
class SamplerWrapperIntegrator {
  public:
    explicit SamplerWrapperIntegrator(Sampler &&sampler)
      : sampler(std::move(sampler)) {}

    void SetScene(std::shared_ptr<Scene> newScene) {
        scene = std::move(newScene);
    }

    void SetRenderOptions(RenderOptions opts) {
        renderOptions = opts;
    }

    Gfx::Image GetRender() {
        Gfx::Image image(renderOptions.dimensions);

        Real d = static_cast<Real>(renderOptions.dimensions[0]) / 2;
        d *= 1. / std::tan(renderOptions.fovWidth / 2.);

#pragma omp parallel for default(none) shared(image, d) schedule(dynamic, 8)
        for (std::size_t y = 0; y < renderOptions.dimensions[1]; y++) {
            for (std::size_t x = 0; x < renderOptions.dimensions[0]; x++) {
                Real rayX = static_cast<Real>(x);
                rayX -= static_cast<Real>(renderOptions.dimensions[0]) / 2.;

                Real rayY = static_cast<Real>(renderOptions.dimensions[1] - y);
                rayY -= static_cast<Real>(renderOptions.dimensions[1]) / 2.;

                Ray ray(renderOptions.position,
                        Math::Ops::Vector::Normalized(Point{rayX, rayY, d}));

                image.At({x, y}) = sampler.Sample(*scene, ray);
            }
        }

        return image;
    }

  private:
    Sampler &&sampler;

    RenderOptions renderOptions{};
    std::shared_ptr<Scene> scene{nullptr};

    std::vector<std::thread> workers{};
};

}
