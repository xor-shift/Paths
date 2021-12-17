#pragma once

#include "SamplerWrapper.hpp"

namespace Paths {

struct MonteCarloIntegrator : public SamplerWrapperIntegrator {
public:
    ~MonteCarloIntegrator() noexcept override = default;

    void add_dot_light(Point p, Point color) noexcept { m_dot_lights.push_back({ p, color }); }

protected:
    [[nodiscard]] Color sample(Ray ray, Scene &scene) const noexcept override;

private:
    struct DotLight {
        Point m_position;
        Point m_color;
    };

    std::vector<DotLight> m_dot_lights {
        { { -10, 10, -2.5 }, { 1, 0, 0 } },
        { { 10, 10, -2.5 }, { 0, 0, 1 } },
    };
};

}
