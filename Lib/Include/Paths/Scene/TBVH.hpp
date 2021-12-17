#pragma once

#include "Tree.hpp"

namespace Paths::BVH::Detail {

template<typename ShapeT = void, bool MT = true> class ThreadedBVH final : public ShapeStore {
    struct Node {
        std::array<std::size_t, 2> m_shape_extents {};
        std::pair<Point, Point> m_extents {};
    };

    typedef std::array<std::size_t, 2> link_t;

    static std::size_t get_miss_link(const ThreadableBVHNode<ShapeT> &node) noexcept {
        using const_node_pointer_t = const ThreadableBVHNode<ShapeT> *;

        const_node_pointer_t current = std::addressof(node);

        while (current->parent()) {
            const auto &parent = dynamic_cast<const ThreadableBVHNode<ShapeT> &>(*current->parent());
            bool current_is_left = parent.left() == current;
            if (current_is_left)
                return dynamic_cast<const ThreadableBVHNode<ShapeT> &>(*parent.right()).get_id();
            current = std::addressof(parent);
        }

        return m_npos;
    }

public:
    typedef Shape::BoundableShapeT<ShapeT> shape_t;
    static constexpr std::size_t m_npos = std::numeric_limits<std::size_t>::max();

    explicit ThreadedBVH(ThreadableBVHTree<ShapeT> &tree) noexcept {
        using node_t = ThreadableBVHNode<ShapeT>;
        auto &root = dynamic_cast<ThreadableBVHNode<ShapeT> &>(tree.root());

        m_extents = root.get_extents();
        std::size_t node_count = 0;
        root.template traverse<ETraversalOrder::PreOrder>([this, &node_count](auto &n) {
            auto &node = dynamic_cast<node_t &>(n);
            node.set_id(node_count);
            ++node_count;

            auto node_shapes = node.get_shapes();

            const std::size_t start = m_shapes.size();
            std::copy(node_shapes.begin(), node_shapes.end(), std::back_inserter(m_shapes));
            const std::size_t end = m_shapes.size();

            m_nodes.push_back({ { start, end }, node.get_extents() });
        });

        auto generate_links = [&root, node_count]() -> std::vector<link_t> {
            std::vector<link_t> t_links(node_count);
            root.template traverse<ETraversalOrder::PreOrder>([&t_links](auto &n) {
                auto &node = dynamic_cast<node_t &>(n);
                std::size_t miss = get_miss_link(node);
                std::size_t hit;
                if (bool is_leaf = !node.left(); is_leaf)
                    hit = miss;
                else
                    hit = dynamic_cast<const node_t *>(node.left())->get_id();
                t_links[node.get_id()] = { hit, miss };
            });
            return t_links;
        };

        if constexpr (MT) {
            for (std::size_t i = 0; i < 6; i++) {
                root.template traverse<ETraversalOrder::PreOrder>([axis = static_cast<EMajorAxis>(i)](auto &n) {
                    auto &node = dynamic_cast<node_t &>(n);
                    node.reorder_children(axis);
                });
                m_links_lists[i] = generate_links();
            }
        } else {
            m_links_lists[0] = generate_links();
        }
    }

private:
    std::vector<shape_t> m_shapes {};
    std::pair<Point, Point> m_extents {};
    std::vector<Node> m_nodes;
    std::array<std::vector<link_t>, 6> m_links_lists {};

protected:
    [[nodiscard]] std::optional<Intersection> intersect_impl(
        Ray ray, std::size_t &bound_checks, std::size_t &shape_checks) const noexcept override {
        std::optional<Intersection> best = std::nullopt;

        const auto axis = static_cast<std::size_t>(ray.m_major_direction);
        const auto &links_list = m_links_lists[MT ? axis : 0];

        for (std::size_t pos = 0; /*pos != npos*/ pos < links_list.size();) {
            const auto &node = m_nodes[pos];
            const auto &links = links_list[pos];
            if constexpr (Paths::ProgramConfig::embed_ray_stats)
                ++bound_checks;
            if (Paths::Shape::AxisAlignedBox::ray_intersects(node.m_extents, ray)) {
                if (const auto [se_min, se_max] = node.m_shape_extents; se_max - se_min) {
                    if constexpr (Paths::ProgramConfig::embed_ray_stats)
                        shape_checks += se_max - se_min;
                    Intersection::replace(
                        best, Shape::intersect_linear(ray, m_shapes.cbegin() + se_min, m_shapes.cbegin() + se_max));
                }
                if constexpr (MT)
                    pos = links[0];
                else
                    ++pos;
            } else {
                pos = links[1];
            }
        }

        return best;
    }
};

}
