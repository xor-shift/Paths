#pragma once

#include "sampler_wrapper.hpp"

namespace Gfx {

class StatVisualiserIntegrator : public SamplerWrapperIntegrator {
  public:
    ~StatVisualiserIntegrator() override = default;

  protected:
    [[nodiscard]] Color Sample(Ray ray, Scene &scene) const noexcept override;
};

}
