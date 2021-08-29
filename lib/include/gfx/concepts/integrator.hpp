#pragma once

#include <concepts>

#include <gfx/ray.hpp>
#include <gfx/scene.hpp>
#include <gfx/spectrum.hpp>

namespace Gfx {
struct RenderOptions {
    Math::Vector<size_t, 2> dimensions{0, 0};
    Real fovWidth = M_PI/2; //in radians
    Point position{0, 0, 0};
    //Point rotation{0, 0, 0};
};
}

namespace Gfx::Concepts {

template<typename T>
concept Sampler = requires(const T &s, const Ray &ray, const Scene &scene) {
    { s.monteCarlo } -> std::convertible_to<bool>;
    { s.Sample(scene, ray) } -> std::same_as<RGBSpectrum>;
};

template<typename T>
concept Integrator = requires(const T &i) {
    true;
};

}
