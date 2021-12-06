#pragma once

#include "bvh.hpp"

namespace Gfx::BVH::Detail {

struct ThinBVHNode {
    std::array<std::size_t, 2> shapeExtents; //diff. of 0 means empty node
    std::pair<Point, Point> extents{};
    std::array<std::size_t, 2> children;
};

template<typename ShapeT = void>
struct ThinBVHTree final : public ShapeStore {
    typedef typename FatBVHNode<ShapeT>::shape_t shape_t;

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

            if constexpr (Gfx::ProgramConfig::EmbedRayStats) ++boundChecks;
            if (!Shape::AABox::EIntersects(node.extents, ray)) continue;

            if (const auto[seMin, seMax] = node.shapeExtents; seMax - seMin) {
                if constexpr (Gfx::ProgramConfig::EmbedRayStats) shapeChecks += seMax - seMin;
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

namespace Gfx::BVH {

namespace Detail {

template<typename ShapeT>
std::shared_ptr<ThinBVHTree<ShapeT>> FatToThin(const TraversableBVHNode<ShapeT> &root) {
    std::vector<typename TraversableBVHNode<ShapeT>::shape_t> shapes;
    shapes.reserve(root.GetShapes().size());
    std::vector<Detail::ThinBVHNode> nodes;

    root.template Traverse<Detail::TraversalOrder::BreadthFirst>([&nodes, &shapes](const auto &n, std::size_t stackDepth) {
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

    auto p = std::make_shared<Detail::ThinBVHTree<ShapeT>>();
    p->shapes = std::move(shapes);
    p->nodes = std::move(nodes);

    return p;
}

}

template<typename ShapeT = void>
std::shared_ptr<Detail::ThinBVHTree<ShapeT>> FatToThin(const TraversableBVHTree<ShapeT> &tree) {
    return Detail::FatToThin<ShapeT>(dynamic_cast<const TraversableBVHNode<ShapeT> &>(tree.Root()));
}

}