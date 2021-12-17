#pragma once

#include "Maths/MatVec.hpp"
#include "Maths/Matrix.hpp"
#include "Paths/Common.hpp"
#include "Paths/Shape/Triangle.hpp"

namespace Paths::STL {

struct Triangle {
    Point m_normal {}, m_v_0 {}, m_v_1 {}, m_v_2 {};

    [[nodiscard]] constexpr Shape::Triangle to_triangle(
        size_t mat_index, Point offset, Maths::Matrix<Real, 3, 3> transform) const noexcept {
        std::array<Point, 3> vertices { m_v_0, m_v_1, m_v_2 };

        for (auto &v : vertices)
            v = v * transform;

        for (auto &v : vertices)
            v = v + offset;

        return Shape::Triangle(mat_index, vertices);
    }
};

}
