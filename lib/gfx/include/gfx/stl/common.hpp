#pragma once

#include <gfx/definitions.hpp>
#include <gfx/scene/scene.hpp>
#include <gfx/shape/triangle.hpp>

namespace Gfx::STL {

struct Triangle {
    Point normal, v0, v1, v2;
    uint16_t attribLen{0};

    [[nodiscard]] constexpr ::Gfx::Shape::Triangle ToTriangle(size_t matIndex, const Point &offset, const Math::Matrix<Real, 3, 3> &transform) const noexcept {
        std::array<Point, 3> vertices{v0, v1, v2};

        for (auto &v : vertices) v = v * transform;

        for (auto &v : vertices) v += offset;

        return ::Gfx::Shape::Triangle(matIndex, vertices);
    }
};

inline void InsertIntoGeneric(auto &generic, const std::vector<Triangle> &triangles, size_t matIndex, const Point &offset = {{0, 0, 0}}, const Math::Matrix<Real, 3, 3> &transform = {{1, 0, 0, 0, 1, 0, 0, 0, 1}}) {
    for (const auto &t : triangles) {
        generic << t.ToTriangle(matIndex, offset, transform);
    }
}

}
