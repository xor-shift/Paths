#pragma once

#include "Traversal.hpp"

namespace Paths::BVH::Detail {

struct ThinBVHNode {
    std::array<std::size_t, 2> shapeExtents; // diff. of 0 means empty node
    std::pair<Point, Point> extents {};
    std::array<std::size_t, 2> children;
};

template<typename ShapeT = void> struct ThinBVHTree final : public ShapeStore {
    typedef typename Shape::BoundableShapeT<ShapeT> shape_t;

    explicit ThinBVHTree(const TraversableBVHTree<ShapeT> &tree) {
        const auto &root = dynamic_cast<const TraversableBVHNode<ShapeT> &>(tree.root());

        shapes.reserve(root.get_shapes().size());

        root.template traverse<Detail::ETraversalOrder::BreadthFirst>([this](const auto &n, std::size_t stackDepth) {
            const auto &node = dynamic_cast<const TraversableBVHNode<ShapeT> &>(n);

            const std::size_t start = shapes.size();
            auto nodeShapes = node.get_shapes();
            std::copy(nodeShapes.begin(), nodeShapes.end(), std::back_inserter(shapes));
            const std::size_t end = shapes.size();

            const std::size_t lhsChild = nodes.size() + stackDepth + 1;
            nodes.push_back({ .shapeExtents = { start, end },
                .extents = node.get_extents(),
                .children = { lhsChild, lhsChild + 1 } });
        });

        /// TODO: use memoization here, for the (per-node current) depth calculations
        root.template traverse<Detail::ETraversalOrder::PreOrder>([this](const auto &n) {
            std::size_t currentDepth = 0;
            const auto *current = std::addressof(n);
            while (current->parent()) {
                ++currentDepth;
                current = current->parent();
            }

            maxDepth = std::max(maxDepth, currentDepth);
        });
    }

    std::vector<shape_t> shapes {};
    std::vector<ThinBVHNode> nodes {};

protected:
    [[nodiscard]] std::optional<Intersection> intersect_impl(
        Ray ray, std::size_t &boundChecks, std::size_t &shapeChecks) const noexcept override {
        std::vector<std::size_t> callStack(maxDepth + 1, 0);
        auto stackPointer = callStack.begin();
        *stackPointer++ = 0;

        std::optional<Intersection> best {};

        while (stackPointer != callStack.begin()) {
            const auto current = *(stackPointer - 1);
            --stackPointer;
            const auto &node = nodes[current];

            if constexpr (Paths::ProgramConfig::embed_ray_stats)
                ++boundChecks;
            if (!Shape::AxisAlignedBox::ray_intersects(node.extents, ray))
                continue;

            const auto [shapesStart, shapesEnd] = node.shapeExtents;
            const auto shapesCount = shapesEnd - shapesStart;

            if (!shapesCount) {
                *stackPointer++ = node.children[1];
                *stackPointer++ = node.children[0];
                continue;
            }

            if constexpr (Paths::ProgramConfig::embed_ray_stats)
                shapeChecks += shapesCount;
            auto newIsection = Shape::intersect_linear(ray, shapes.cbegin() + shapesStart, shapes.cbegin() + shapesEnd);
            Intersection::replace(best, std::move(newIsection));
        }

        return best;
    }

private:
    std::size_t maxDepth = 0;
};

}
