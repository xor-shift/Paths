#pragma once

#include "Traversal.hpp"

namespace Paths::BVH::Detail {

template<typename ShapeT> class BVHNode final : public IntrudableBVHNode<ShapeT> {
public:
    typedef Shape::BoundableShapeT<ShapeT> shape_t;

    BVHNode() noexcept = default;

    ~BVHNode() noexcept override = default;

    explicit BVHNode(
        std::shared_ptr<std::vector<shape_t>> shapes, std::pair<std::size_t, std::size_t> shape_extents = { 0, 0 })
        : m_shapes_impl(std::move(shapes))
        , m_shape_extents(shape_extents) {
        if (!this->get_shapes().empty())
            this->calculate_extents();
    }

    [[nodiscard]] BinaryTreeNode *left() noexcept override { return m_children[0].get(); }

    [[nodiscard]] const BinaryTreeNode *left() const noexcept override { return m_children[0].get(); }

    [[nodiscard]] BinaryTreeNode *right() noexcept override { return m_children[1].get(); }

    [[nodiscard]] const BinaryTreeNode *right() const noexcept override { return m_children[1].get(); }

    [[nodiscard]] BinaryTreeNode *parent() noexcept override { return m_parent; }

    [[nodiscard]] const BinaryTreeNode *parent() const noexcept override { return m_parent; }

    void swap_children() noexcept override { m_children[0].swap(m_children[1]); }

    [[nodiscard]] std::size_t get_id() const noexcept override { return m_id; }

    void set_id(std::size_t i) noexcept override { m_id = i; }

    [[nodiscard]] std::pair<Point, Point> get_extents() const noexcept override { return m_extents; }

    [[nodiscard]] std::span<shape_t> get_shapes() noexcept override {
        return { m_shapes_impl->begin() + m_shape_extents.first, m_shapes_impl->begin() + m_shape_extents.second };
    }

    [[nodiscard]] std::span<const shape_t> get_shapes() const noexcept override {
        return { m_shapes_impl->begin() + m_shape_extents.first, m_shapes_impl->begin() + m_shape_extents.second };
    }

    [[nodiscard]] std::size_t total_shape_count() const noexcept override { return m_total_shape_count; }

    void split_at(std::size_t rhs_start_index) noexcept override {
        auto shapes = this->get_shapes();
        rhs_start_index = std::min(rhs_start_index, shapes.size());
        m_children = { std::make_unique<BVHNode>(m_shapes_impl), std::make_unique<BVHNode>(m_shapes_impl) };

        m_children[0]->m_shape_extents = { m_shape_extents.first, m_shape_extents.first + rhs_start_index };
        m_children[1]->m_shape_extents = { m_shape_extents.first + rhs_start_index, m_shape_extents.second };

        for (auto &c : m_children) {
            c->m_total_shape_count = c->get_shapes().size();
            c->m_parent = this;
            c->calculate_extents();
        }

        m_shape_extents = { 0, 0 };
    }

    void unsplit_once() noexcept override {
        m_shape_extents = {
            m_children[0]->m_shape_extents.first,
            m_children[1]->m_shape_extents.second,
        };

        m_children = { nullptr, nullptr };
    }

protected:
    void set_extents(std::pair<Point, Point> e) noexcept override { m_extents = e; }

    std::shared_ptr<std::vector<shape_t>> m_shapes_impl { nullptr };
    std::pair<std::size_t, std::size_t> m_shape_extents { 0, 0 };
    std::pair<Point, Point> m_extents {};
    std::array<std::unique_ptr<BVHNode>, 2> m_children { nullptr, nullptr };
    BVHNode *m_parent { nullptr };
    std::size_t m_id = 0;
    std::size_t m_total_shape_count = 0;
};

template<typename ShapeT = void> class BVHTree : public IntrudableBVHTree<ShapeT> {
public:
    typedef BVHNode<ShapeT> node_t;
    typedef Shape::BoundableShapeT<ShapeT> shape_t;

    explicit BVHTree(std::vector<shape_t> &&vec)
        : m_shapes(std::make_unique<std::vector<shape_t>>(std::move(vec)))
        , m_root(std::make_unique<node_t>(m_shapes, std::make_pair<std::size_t, std::size_t>(0, m_shapes->size()))) {
        // NewFatBVHNode<ShapeT> asd(shapes, {0, shapes->size()});
    }

    [[nodiscard]] node_t &root() noexcept override { return *m_root; }

    [[nodiscard]] const node_t &root() const noexcept override { return *m_root; }

private:
    std::shared_ptr<std::vector<shape_t>> m_shapes;
    std::unique_ptr<node_t> m_root { nullptr };
};

}
