#include "Paths/Integrator/Sampler/Albedo.hpp"

namespace Paths {

[[nodiscard]] Color AlbedoIntegrator::sample(Ray ray, Scene &scene) const noexcept {
    std::size_t bound_checks = 0;
    std::size_t shape_checks = 0;

    auto isection = scene.intersect_ray(ray, bound_checks, shape_checks);

    return isection ? scene.get_material(isection->m_mat_index).m_albedo : Point { 0, 0, 0 };
}

}
