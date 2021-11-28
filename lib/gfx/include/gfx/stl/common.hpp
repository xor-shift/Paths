#pragma once

#include <gfx/common.hpp>
#include <gfx/shape/triangle.hpp>

namespace Gfx::STL {

struct Triangle {
    Point normal{}, v0{}, v1{}, v2{};

    [[nodiscard]] constexpr Shape::Triangle ToTriangle(size_t matIndex, Point offset, Maths::Matrix<Real, 3, 3> transform) const noexcept {
        std::array<Point, 3> vertices{v0, v1, v2};

        for (auto &v : vertices) v = v * transform;

        for (auto &v : vertices) v = v + offset;

        return Shape::Triangle(matIndex, vertices);
    }
};

}
