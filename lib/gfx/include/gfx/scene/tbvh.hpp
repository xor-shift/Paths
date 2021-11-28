#pragma once

namespace Gfx::Detail::BVH {

struct ThreadedThinBVHNode {
    std::array<std::size_t, 2> shapeExtents;
    std::pair<Point, Point> extents{};
    std::array<std::size_t, 2> links;
};

}