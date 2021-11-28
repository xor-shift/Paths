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
    [[nodiscard]] std::optional<Intersection> IntersectImpl(Ray ray) const noexcept override {
        //return Fn(ray, 0);

        std::stack<std::size_t> callStack{};
        callStack.push(0);

        std::optional<Intersection> best{};

        while (!callStack.empty()) {
            const auto current = callStack.top();
            callStack.pop();
            const auto &node = nodes[current];

            if (!Shape::AABox::EIntersects(node.extents, ray)) continue;

            if (const auto[seMin, seMax] = node.shapeExtents; seMax - seMin) {
                for (std::size_t i = seMin; i < seMax; i++) {
                    Shape::Apply(shapes[i], [ray, &best]<Concepts::Shape T>(const T &s) {
                        Intersection::Replace(best, std::move(s.Intersect(ray)));
                    });
                }
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

template<typename ShapeT = void>
std::shared_ptr<Detail::ThinBVHTree<ShapeT>> FatToThin(const Detail::FatBVHNode<ShapeT> &root) {
    using NodeType = Detail::FatBVHNode<ShapeT>;

    std::vector<typename NodeType::shape_t> shapes;
    std::vector<Detail::ThinBVHNode> nodes;

    root.BreadthFirstTraverse([&nodes, &shapes](const auto &node, std::size_t stackDepth) {
        const std::size_t start = shapes.size();
        std::copy(node.shapes.cbegin(), node.shapes.cend(), std::back_inserter(shapes));
        const std::size_t end = shapes.size();

        const std::size_t lhsChild = nodes.size() + stackDepth + 1;
        nodes.push_back({
                          .shapeExtents = {start, end},
                          .extents = node.extents,
                          .children = {lhsChild, lhsChild + 1}
                        });
    });

    auto p = std::make_shared<Detail::ThinBVHTree<ShapeT>>();
    p->shapes = std::move(shapes);
    p->nodes = std::move(nodes);

    return p;
}

}