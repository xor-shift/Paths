#pragma once

#include "SamplerWrapper.hpp"

namespace Paths {

class WhittedIntegrator : public SamplerWrapperIntegrator {
public:
    ~WhittedIntegrator() noexcept override = default;

    void add_dot_light(Point p, Point color) noexcept { m_dot_lights.push_back({ p, color }); }

protected:
    [[nodiscard]] Color sample(Ray ray, Scene &scene) const noexcept override;

private:
    [[nodiscard]] Color sample_impl(
        Ray ray, Scene &scene, std::size_t depth, std::size_t &bound_checks, std::size_t &shape_checks) const noexcept;

    struct DotLight {
        Point m_position;
        Point m_emission;
    };

    // Point ambientLight {0.1, 0.1, 0.1};
    Point m_ambient_light {};

    std::vector<DotLight> m_dot_lights {
        { { -10, 10, -2.5 }, { 1, 1, 1 } },
        { { 10, 10, -2.5 }, { 1, 1, 1 } },
    };
};

}
