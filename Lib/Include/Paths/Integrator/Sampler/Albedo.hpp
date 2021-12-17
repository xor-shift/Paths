#pragma once

#include "SamplerWrapper.hpp"

namespace Paths {

class AlbedoIntegrator : public SamplerWrapperIntegrator {
public:
    ~AlbedoIntegrator() noexcept override = default;

protected:
    [[nodiscard]] Color sample(Ray ray, Scene &scene) const noexcept override;
};

}
