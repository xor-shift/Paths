#pragma once

#include <vector>

#include <maths/vector.hpp>

namespace Gfx::OBJ {

struct Vertex {
    Math::Vector<double, 3> coordinate;
    double weight{1.};
};

struct Group {
    std::string material{};
    bool smooth{false};
    std::vector<Vertex> vertices{};
};

struct File {
    std::vector<Math::Vector<double, 3>> textureCoords{};
    std::vector<Math::Vector<double, 3>> vertexNormals{};
    std::vector<Math::Vector<double, 3>> pSpaceVertices{};
    std::unordered_map<std::string, Group> groups;
};

}