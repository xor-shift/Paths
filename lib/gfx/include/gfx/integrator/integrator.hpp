#pragma once

#include <concepts>

#include <gfx/ray.hpp>
#include <gfx/scene/scene.hpp>
#include <gfx/spectrum.hpp>

namespace Gfx {
struct RenderOptions {
    Real fovWidth = M_PI / 2; //in radians
    Point position{{0, 0, 0}};
    Math::Matrix<Real, 3, 3> rotation{{{1, 0, 0},
                                        {0, 1, 0},
                                        {0, 0, 1}}};
    //Point rotation{0, 0, 0};
};
}

namespace Gfx::Concepts {

template<typename T>
concept Integrator = requires(const T &i) {
    true;
};

}
