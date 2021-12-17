#pragma once

#include "Common.hpp"

#include <string>
#include <vector>

namespace Paths::STL {

struct BinarySTL {
    char m_header[80] { 0 };
    std::vector<STL::Triangle> m_triangles {};

    [[nodiscard]] std::vector<Shape::Triangle> convert(size_t mat_index, Point offset = {},
        Maths::Matrix<Real, 3, 3> transform = Maths::identity_matrix<Real, 3>()) const noexcept;
};

extern std::optional<BinarySTL> read_stl(const std::string &str) noexcept;

}
