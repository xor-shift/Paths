#include <gfx/integrator/whitted.hpp>

namespace Gfx {

[[nodiscard]] Color WhittedIntegrator::Sample(Ray ray, Scene &scene) const noexcept {
    std::size_t boundChecks = 0, shapeChecks = 0;
    const Color res = SampleImpl(ray, scene, 0, boundChecks, shapeChecks);
    if constexpr (Gfx::ProgramConfig::VisualiseRayStats) return Color{boundChecks, shapeChecks};
    else return res;
}

[[nodiscard]] Color WhittedIntegrator::SampleImpl(Ray ray, Scene &scene, std::size_t depth, std::size_t &boundChecks, std::size_t &shapeChecks) const noexcept {
    if (depth >= 8) return {};

    auto isection = scene.Intersect(ray, boundChecks, shapeChecks);

    if (!isection) return {};

    enum class Action {
        Diffuse,
        Transparent,
        Mirror,
    } action = Action::Diffuse;

    const auto material = scene.GetMaterial(isection->matIndex);
    const Point safeReflectionSpot = isection->intersectionPoint + isection->orientedNormal * sensibleEps;

    if (material.reflectance >= 0.95) action = Action::Mirror;

    if (action == Action::Diffuse) {
        Point
          lambertian = 0,
          specular = 0;

        for (const auto &light: dotLights) {
            const Point l = light.p - isection->intersectionPoint;
            const auto lDist = Maths::Magnitude(l);

            if (auto itemp = scene.Intersect(Ray(safeReflectionSpot, l / lDist), boundChecks, shapeChecks); itemp && itemp->distance < lDist)
                continue;

            const auto[cLamb, cSpec] = Detail::BlinnPhongCoefficients(
              light.p,
              isection->intersectionPoint,
              isection->orientedNormal,
              -ray.direction
            );

            lambertian = lambertian + light.color * cLamb;
            specular = specular + light.color * cSpec;
        }

        //return isection->orientedNormal;
        return material.albedo * (lambertian + specular + ambientLight);
    } else if (action == Action::Mirror) {
        return SampleImpl(Ray(safeReflectionSpot, Detail::Reflect(ray.direction, isection->orientedNormal)), scene, depth + 1, boundChecks, shapeChecks);
    } else {
        return {};
    }
}

}
