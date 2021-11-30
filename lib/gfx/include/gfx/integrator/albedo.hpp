#pragma once

#include "sampler_wrapper.hpp"

namespace Gfx {

class AlbedoIntegrator : public SamplerWrapperIntegrator {
  public:
    ~AlbedoIntegrator() noexcept override = default;

  protected:
    [[nodiscard]] Color Sample(Ray ray, Scene &scene) const noexcept override;
};

}
