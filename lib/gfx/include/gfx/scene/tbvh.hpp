#pragma once

#include "tree.hpp"

namespace Gfx::BVH::Detail {

template<typename ShapeT = void, bool MT = true>
class ThreadedBVH final : public ShapeStore {
    struct Node {
        std::array<std::size_t, 2> shapeExtents{};
        std::pair<Point, Point> extents{};
    };

    typedef std::array<std::size_t, 2> link_t;

    static std::size_t GetMissLink(const ThreadableBVHNode<ShapeT> &node) noexcept {
        using const_node_pointer_t = const ThreadableBVHNode<ShapeT> *;

        const_node_pointer_t current = std::addressof(node);

        while (current->Parent()) {
            const auto &parent = dynamic_cast<const ThreadableBVHNode<ShapeT> &>(*current->Parent());
            bool currentIsLeft = parent.Left() == current;
            if (currentIsLeft) return dynamic_cast<const ThreadableBVHNode<ShapeT> &>(*parent.Right()).GetID();
            current = std::addressof(parent);
        }

        return npos;
    }

  public:
    typedef Shape::boundable_shape_t <ShapeT> shape_t;
    static constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();

    explicit ThreadedBVH(ThreadableBVHTree<ShapeT> &tree) noexcept {
        using node_t = ThreadableBVHNode<ShapeT>;
        auto &root = dynamic_cast<ThreadableBVHNode<ShapeT> &>(tree.Root());

        extents = root.GetExtents();
        std::size_t nodeCount = 0;
        root.template Traverse<TraversalOrder::PreOrder>([this, &nodeCount](auto &n) {
            auto &node = dynamic_cast<node_t &>(n);
            node.SetID(nodeCount);
            ++nodeCount;

            auto nodeShapes = node.GetShapes();

            const std::size_t start = shapes.size();
            std::copy(nodeShapes.begin(), nodeShapes.end(), std::back_inserter(shapes));
            const std::size_t end = shapes.size();

            nodes.push_back({{start, end}, node.GetExtents()});
        });

        auto GenerateLinks = [&root, nodeCount]() -> std::vector<link_t> {
            std::vector<link_t> tLinks(nodeCount);
            root.template Traverse<TraversalOrder::PreOrder>([&tLinks](auto &n) {
                auto &node = dynamic_cast<node_t &>(n);
                std::size_t miss = GetMissLink(node);
                std::size_t hit;
                if (bool isLeaf = !node.Left(); isLeaf) hit = miss;
                else hit = dynamic_cast<const node_t *>(node.Left())->GetID();
                tLinks[node.GetID()] = {hit, miss};
            });
            return tLinks;
        };

        if constexpr (MT) {
            for (std::size_t i = 0; i < 6; i++) {
                root.template Traverse<TraversalOrder::PreOrder>([axis = static_cast<MajorAxis>(i)](auto &n) {
                    auto &node = dynamic_cast<node_t &>(n);
                    node.ReorderChildren(axis);
                });
                linksLists[i] = GenerateLinks();
            }
        } else {
            linksLists[0] = GenerateLinks();
        }
    }

  private:
    std::vector<shape_t> shapes{};
    std::pair<Point, Point> extents{};
    std::vector<Node> nodes;
    std::array<std::vector<link_t>, 6> linksLists{};

  protected:
    [[nodiscard]] std::optional<Intersection> IntersectImpl(Ray ray, std::size_t &boundChecks, std::size_t &shapeChecks) const noexcept override {
        std::optional<Intersection> best = std::nullopt;

        const auto axis = static_cast<std::size_t>(ray.majorDirection);
        const auto &linksList = linksLists[MT ? axis : 0];

        for (std::size_t pos = 0; /*pos != npos*/ pos < linksList.size();) {
            const auto &node = nodes[pos];
            const auto &links = linksList[pos];
            if constexpr (Gfx::ProgramConfig::embedRayStats) ++boundChecks;
            if (Gfx::Shape::AABox::EIntersects(node.extents, ray)) {
                if (const auto[seMin, seMax] = node.shapeExtents; seMax - seMin) {
                    if constexpr (Gfx::ProgramConfig::embedRayStats) shapeChecks += seMax - seMin;
                    Intersection::Replace(
                      best,
                      Shape::IntersectLinear(ray, shapes.cbegin() + seMin, shapes.cbegin() + seMax)
                    );
                }
                if constexpr (MT) pos = links[0];
                else ++pos;
            } else {
                pos = links[1];
            }
        }

        return best;
    }
};

}
