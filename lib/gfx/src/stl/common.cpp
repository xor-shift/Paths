#include <gfx/stl/common.hpp>

namespace Gfx::STL {

void InsertIntoScene(Scene &scene, const std::vector<Triangle> &triangles, size_t matIndex, const Point &offset, const Math::Matrix<Real, 3, 3> &transform) {
    for (const auto &t : triangles) {
        scene << t.ToTriangle(matIndex, offset, transform);
    }
}

}
