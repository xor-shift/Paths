#pragma once

#include "common.hpp"

#include <string>
#include <vector>

namespace Gfx::STL {

struct BinarySTL {
    char header[80]{0};
    std::vector<STL::Triangle> triangles{};

    [[nodiscard]] std::vector<Shape::Triangle> Convert(size_t matIndex, Point offset= {}, Maths::Matrix<Real, 3, 3> transform = Maths::Identity<Real, 3>()) const noexcept;
};

extern std::optional<BinarySTL> ReadSTL(const std::string &str) noexcept;

}
