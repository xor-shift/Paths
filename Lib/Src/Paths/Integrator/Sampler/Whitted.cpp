#include "Paths/Integrator/Sampler/Whitted.hpp"

namespace Paths {

[[nodiscard]] Color WhittedIntegrator::sample(Ray ray, Scene &scene) const noexcept {
    std::size_t bound_checks = 0, shape_checks = 0;
    const Color res = sample_impl(ray, scene, 0, bound_checks, shape_checks);
    return res;
}

[[nodiscard]] Color WhittedIntegrator::sample_impl(
    Ray ray, Scene &scene, std::size_t depth, std::size_t &bound_checks, std::size_t &shape_checks) const noexcept {
    if (depth >= 8)
        return {};

    auto isection = scene.intersect_ray(ray, bound_checks, shape_checks);

    if (!isection)
        return {};

    enum class EAction {
        Diffuse,
        Transparent,
        Mirror,
    } action
        = EAction::Diffuse;

    const auto material = scene.get_material(isection->m_mat_index);
    const Point safe_reflection_spot = isection->m_intersection_point + isection->m_oriented_normal * sensible_eps;

    if (material.m_reflectance >= 0.95)
        action = EAction::Mirror;

    if (action == EAction::Diffuse) {
        Point lambertian = 0;
        Point specular = 0;

        for (const auto &light : m_dot_lights) {
            const Point l = light.m_position - isection->m_intersection_point;
            const auto l_dist = Maths::Magnitude(l);

            if (auto itemp = scene.intersect_ray(Ray(safe_reflection_spot, l / l_dist), bound_checks, shape_checks);
                itemp && itemp->m_distance < l_dist)
                continue;

            const auto [c_lamb, c_spec] = Detail::blinn_phong_coefficients(
                light.m_position, isection->m_intersection_point, isection->m_oriented_normal, -ray.m_direction);

            lambertian = lambertian + light.m_emission * c_lamb;
            specular = specular + light.m_emission * c_spec;
        }

        // return isection->orientedNormal;
        return material.m_albedo * (lambertian + specular + m_ambient_light);
    } else if (action == EAction::Mirror) {
        return sample_impl(
            Ray(safe_reflection_spot, Detail::reflect_vector(ray.m_direction, isection->m_oriented_normal)), scene,
            depth + 1, bound_checks, shape_checks);
    } else {
        return {};
    }
}

}
