#include "Paths/Integrator/Sampler/Statistics.hpp"

namespace Paths {

[[nodiscard]] Color StatVisualiserIntegrator::sample(Ray ray, Scene &scene) const noexcept {
    std::size_t bound_checks = 0, shape_checks = 0;

    [[maybe_unused]] auto isection = scene.intersect_ray(ray, bound_checks, shape_checks);

    return { bound_checks, shape_checks, 0 };
}

}
