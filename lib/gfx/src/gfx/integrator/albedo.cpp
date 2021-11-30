#include <gfx/integrator/albedo.hpp>

namespace Gfx {

[[nodiscard]] Color AlbedoIntegrator::Sample(Ray ray, Scene &scene) const noexcept {
    std::size_t boundChecks = 0, shapeChecks = 0;

    auto isection = scene.Intersect(ray, boundChecks, shapeChecks);

    return isection ? scene.GetMaterial(isection->matIndex).albedo : Point{0, 0, 0};
}

}
