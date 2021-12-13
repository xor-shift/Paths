#pragma once

#include "traversal.hpp"

namespace Gfx::BVH::Detail {

struct ThinBVHNode {
    std::array<std::size_t, 2> shapeExtents; //diff. of 0 means empty node
    std::pair<Point, Point> extents{};
    std::array<std::size_t, 2> children;
};

template<typename ShapeT = void>
struct ThinBVHTree final : public ShapeStore {
    typedef typename Shape::boundable_shape_t<ShapeT> shape_t;

    explicit ThinBVHTree(const TraversableBVHTree <ShapeT> &tree) {
        const auto &root = dynamic_cast<const TraversableBVHNode<ShapeT> &>(tree.Root());

        shapes.reserve(root.GetShapes().size());

        root.template Traverse<Detail::TraversalOrder::BreadthFirst>([this](const auto &n, std::size_t stackDepth) {
            const auto &node = dynamic_cast<const TraversableBVHNode<ShapeT> &>(n);

            const std::size_t start = shapes.size();
            auto nodeShapes = node.GetShapes();
            std::copy(nodeShapes.begin(), nodeShapes.end(), std::back_inserter(shapes));
            const std::size_t end = shapes.size();

            const std::size_t lhsChild = nodes.size() + stackDepth + 1;
            nodes.push_back({
                              .shapeExtents = {start, end},
                              .extents = node.GetExtents(),
                              .children = {lhsChild, lhsChild + 1}
                            });
        });
    }

    std::vector<shape_t> shapes{};
    std::vector<ThinBVHNode> nodes{};

  protected:
    [[nodiscard]] std::optional<Intersection> IntersectImpl(Ray ray, std::size_t &boundChecks, std::size_t &shapeChecks) const noexcept override {
        std::stack<std::size_t> callStack{};
        callStack.push(0);

        std::optional<Intersection> best{};

        while (!callStack.empty()) {
            const auto current = callStack.top();
            callStack.pop();
            const auto &node = nodes[current];

            if constexpr (Gfx::ProgramConfig::embedRayStats) ++boundChecks;
            if (!Shape::AABox::EIntersects(node.extents, ray)) continue;

            if (const auto[seMin, seMax] = node.shapeExtents; seMax - seMin) {
                if constexpr (Gfx::ProgramConfig::embedRayStats) shapeChecks += seMax - seMin;
                Intersection::Replace(
                  best,
                  Shape::IntersectLinear(ray, shapes.cbegin() + seMin, shapes.cbegin() + seMax)
                );
            } else {
                callStack.push(node.children[1]);
                callStack.push(node.children[0]);
            }
        }

        return best;
    }

};

}
