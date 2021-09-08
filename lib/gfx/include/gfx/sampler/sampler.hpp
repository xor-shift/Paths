#pragma once

#include <gfx/ray.hpp>
#include <gfx/scene/scene.hpp>
#include <gfx/spectrum.hpp>

namespace Gfx::Concepts {

template<typename T>
concept Sampler = requires(const T &s, const Ray &ray, const Scene &scene) {
    { s.Sample(scene, ray) } -> std::same_as<RGBSpectrum>;
};

}
