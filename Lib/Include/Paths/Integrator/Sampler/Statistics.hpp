#pragma once

#include "SamplerWrapper.hpp"

namespace Paths {

class StatVisualiserIntegrator : public SamplerWrapperIntegrator {
public:
    ~StatVisualiserIntegrator() override = default;

protected:
    [[nodiscard]] Color sample(Ray ray, Scene &scene) const noexcept override;
};

}
