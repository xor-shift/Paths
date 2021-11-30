#include <gfx/integrator/mc.hpp>

namespace Gfx {

[[nodiscard]] Color MCIntegrator::Sample(Ray ray, Scene &scene) const noexcept {
    Color wO{0, 0, 0};
    Color curA{1, 1, 1};
    Real previousCosine = 1;

    std::size_t boundChecks = 0, shapeChecks = 0;

    Ray currentRay = ray;

    for (size_t depth = 0;; depth++) {
        if (depth > 7) {
            if (Math::RandomDouble() > .8) break;
        }

        auto isection = scene.Intersect(currentRay, boundChecks, shapeChecks);
        if (!isection) break;

        const auto material = scene.GetMaterial(isection->matIndex);
        const Point safeReflectionSpot = isection->intersectionPoint + isection->orientedNormal * sensibleEps;

        /*Color lambertian{};
        for (const auto &light: dotLights) {
            Point l = light.p - isection->intersectionPoint;
            const auto lDist = Maths::Magnitude(l);
            l = l / lDist;

            if (auto itemp = scene.Intersect(Ray(safeReflectionSpot, l), boundChecks, shapeChecks); itemp && itemp->distance < lDist)
                continue;

            lambertian = lambertian + light.color * std::max<Real>(Maths::Dot(l, isection->orientedNormal), 0);
        }*/

        wO = wO + (isection->goingIn ? material.emittance : Color{}) * curA * previousCosine;
        curA = curA * material.albedo;

        if (Math::RandomDouble() > material.reflectance)
            currentRay = Ray(safeReflectionSpot, Math::D3RandomUnitVector());
        else
            currentRay = Ray(safeReflectionSpot, Detail::Reflect(currentRay.direction, isection->orientedNormal));

        previousCosine = Maths::Dot(currentRay.direction, isection->orientedNormal);
    }

    if constexpr (Gfx::ProgramConfig::VisualiseRayStats) return Color{boundChecks, shapeChecks, 0};
    else return wO;
}

}
