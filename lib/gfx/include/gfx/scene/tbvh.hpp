#pragma once

#include "bvh.hpp"
#include "tree.hpp"

namespace Gfx::BVH::Detail {

template<typename ShapeT = void>
class ThreadedBVH final : public ShapeStore {
    struct Node {
        std::array<std::size_t, 2> shapeExtents{};
        std::pair<Point, Point> extents{};
    };

    typedef std::array<std::size_t, 2> link_t;

  public:
    typedef Shape::boundable_shape_t <ShapeT> shape_t;
    static constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();

    /// This will modify the given tree in a non-destructive manner (will reorder children of nodes as needed)
    /// \param tree
    template<Concepts::BVH::ThreadableBVH Tree>
    explicit ThreadedBVH(Tree &tree) noexcept {
        using node_t = typename Tree::node_t;

        node_t &root = tree.Root();

        //shapes.reserve(tree.GetShapes(root).size());
        extents = tree.GetExtents(root);
        std::size_t nodeCount = 0;
        Detail::Traverser<Tree>::template Traverse<TraversalOrder::PreOrder>(tree, [this, &tree, &nodeCount](node_t &node) {
            node.id = nodeCount;
            ++nodeCount;

            auto nodeShapes = tree.GetShapes(node);

            const std::size_t start = shapes.size();
            std::copy(nodeShapes.begin(), nodeShapes.end(), std::back_inserter(shapes));
            const std::size_t end = shapes.size();

            nodes.push_back({{start, end}, node.extents});
        });

        for (std::size_t i = 0; i < 6; i++) {
            Detail::Traverser<Tree>::template Traverse<TraversalOrder::PreOrder>(tree, [axis = static_cast<MajorAxis>(i)](node_t &node) {
                node.ReorderChildren(axis);
            });

            std::vector<link_t> tLinks(nodeCount);
            Detail::Traverser<Tree>::template Traverse<TraversalOrder::PreOrder>(tree, [&tLinks](node_t &node) {
                std::size_t miss = GetMissLink<Tree>(node);
                std::size_t hit;
                if (bool isLeaf = !node.Left(); isLeaf) hit = miss;
                else hit = node.Left()->id;
                tLinks[node.id] = {hit, miss};
            });

            linksLists[i] = tLinks;
        }
    }

  private:
    std::vector<shape_t> shapes{};
    std::pair<Point, Point> extents{};
    std::vector<Node> nodes;
    std::array<std::vector<link_t>, 6> linksLists{};

    template<Concepts::BVH::ThreadableBVH T>
    static std::size_t GetMissLink(const typename T::node_t &node) noexcept {
        using const_node_pointer_t = typename T::const_node_pointer_t;

        const_node_pointer_t current = std::addressof(node);

        while (current->Parent()) {
            const auto &parent = *current->Parent();
            bool currentIsLeft = parent.Left() == current;
            if (currentIsLeft) return parent.Right()->id;
            current = std::addressof(parent);
        }

        return npos;
    }

  protected:
    [[nodiscard]] std::optional<Intersection> IntersectImpl(Ray ray, std::size_t &boundChecks, std::size_t &shapeChecks) const noexcept override {
        std::optional<Intersection> best = std::nullopt;

        const auto axis = static_cast<std::size_t>(ray.majorDirection);
        const auto &linksList = linksLists[axis];

        for (std::size_t pos = 0; pos != npos;) {
            const auto &node = nodes[pos];
            const auto &links = linksList[pos];
            if constexpr (Gfx::ProgramConfig::EmbedRayStats) ++boundChecks;
            if (Gfx::Shape::AABox::EIntersects(node.extents, ray)) {
                if (const auto[seMin, seMax] = node.shapeExtents; seMax - seMin) {
                    if constexpr (Gfx::ProgramConfig::EmbedRayStats) shapeChecks += seMax - seMin;
                    Intersection::Replace(
                      best,
                      Shape::IntersectLinear(ray, shapes.cbegin() + seMin, shapes.cbegin() + seMax)
                    );
                }
                pos = links[0];
            } else {
                pos = links[1];
            }
        }

        return best;
    }
};

}
