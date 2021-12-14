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

        ///TODO: use memoization here, for the (per-node current) depth calculations
        root.template Traverse<Detail::TraversalOrder::PreOrder>([this](const auto &n) {
            std::size_t currentDepth = 0;
            const auto *current = std::addressof(n);
            while (current->Parent()) {
                ++currentDepth;
                current = current->Parent();
            }

            maxDepth = std::max(maxDepth, currentDepth);
        });
    }

    std::vector<shape_t> shapes{};
    std::vector<ThinBVHNode> nodes{};

  protected:
    [[nodiscard]] std::optional<Intersection> IntersectImpl(Ray ray, std::size_t &boundChecks, std::size_t &shapeChecks) const noexcept override {
        std::vector<std::size_t> callStack(maxDepth + 1, 0);
        auto stackPointer = callStack.begin();
        *stackPointer++ = 0;

        std::optional<Intersection> best{};

        while (stackPointer != callStack.begin()) {
            const auto current = *(stackPointer - 1);
            --stackPointer;
            const auto &node = nodes[current];

            if constexpr (Gfx::ProgramConfig::embedRayStats) ++boundChecks;
            if (!Shape::AABox::EIntersects(node.extents, ray)) continue;

            const auto[shapesStart, shapesEnd] = node.shapeExtents;
            const auto shapesCount = shapesEnd - shapesStart;

            if (!shapesCount) {
                *stackPointer++ = node.children[1];
                *stackPointer++ = node.children[0];
                continue;
            }

            if constexpr (Gfx::ProgramConfig::embedRayStats) shapeChecks += shapesCount;
            const auto newIsection = Shape::IntersectLinear(ray, shapes.cbegin() + shapesStart, shapes.cbegin() + shapesEnd);
            Intersection::Replace(best, newIsection);
        }

        return best;
    }



  private:
    std::size_t maxDepth = 0;
};

}
