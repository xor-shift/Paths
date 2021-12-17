#include "Paths/Integrator/Sampler/MonteCarlo.hpp"

namespace Paths {

[[nodiscard]] Color MonteCarloIntegrator::sample(Ray ray, Scene &scene) const noexcept {
    Color w_o { 0, 0, 0 };
    Color cur_a { 1, 1, 1 };
    Real previous_cosine = 1;

    std::size_t bound_checks = 0;
    std::size_t shape_checks = 0;

    Ray current_ray = ray;

    for (size_t depth = 0;; depth++) {
        if (depth > 7) {
            if (Maths::Random::uniform_normalised() > .8)
                break;
        }

        auto isection = scene.intersect_ray(current_ray, bound_checks, shape_checks);
        if (!isection)
            break;

        const auto material = scene.get_material(isection->m_mat_index);
        const Point safe_reflection_spot = isection->m_intersection_point + isection->m_oriented_normal * sensible_eps;

        /*Color lambertian{};
        for (const auto &light: dotLights) {
            Point l = light.p - isection->intersectionPoint;
            const auto lDist = Maths::Magnitude(l);
            l = l / lDist;

            if (auto itemp = scene.Intersect(Ray(safeReflectionSpot, l), boundChecks, shapeChecks); itemp &&
        itemp->distance < lDist) continue;

            lambertian = lambertian + light.color * std::max<Real>(Maths::Dot(l, isection->orientedNormal), 0);
        }*/

        w_o = w_o + (isection->m_going_in ? material.m_emittance : Color {}) * cur_a * previous_cosine;
        cur_a = cur_a * material.m_albedo;

        if (Maths::Random::uniform_normalised() > material.m_reflectance)
            current_ray = Ray(safe_reflection_spot, Maths::Random::unit_vector());
        else
            current_ray = Ray(
                safe_reflection_spot, Detail::reflect_vector(current_ray.m_direction, isection->m_oriented_normal));

        previous_cosine = Maths::dot(current_ray.m_direction, isection->m_oriented_normal);
    }

    return w_o;
}

}
