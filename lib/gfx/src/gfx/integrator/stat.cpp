#include <gfx/integrator/stat.hpp>

namespace Gfx {

[[nodiscard]] Color StatVisualiserIntegrator::Sample(Ray ray, Scene &scene) const noexcept {
    std::size_t boundChecks = 0, shapeChecks = 0;

    auto isection = scene.Intersect(ray, boundChecks, shapeChecks);

    return Color{boundChecks, 0, 0};
}

}
