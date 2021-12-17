#pragma once

#include <concepts>
#include <span>
#include <stack>
#include <type_traits>

#include "Paths/Common.hpp"
#include "Paths/Ray.hpp"
#include "Paths/Shape/Shapes.hpp"

namespace Paths {

enum class EMajorAxis : int {
    PosX = 0,
    NegX = 1,
    PosY = 2,
    NegY = 3,
    PosZ = 4,
    NegZ = 5,
};

}

namespace Paths::BVH::Detail {

enum class ETraversalOrder {
    PreOrder,
    InOrder,
    PostOrder,
    BreadthFirst,
};

}

namespace Paths::BVH {

/// Requirement for left() and right() functions: \n
///  - left() should return nullptr iff right() returns nullptr \n
///  - parent() should return nullptr iff *this is not a children of any other Traversable \n
class BinaryTreeNode {
public:
    virtual ~BinaryTreeNode() noexcept = default;

    template<Detail::ETraversalOrder order> void traverse(auto &&cb) {
        using U = std::decay_t<decltype(cb)>;
        return traverse_impl<U, false, order>(cb);
    }

    template<Detail::ETraversalOrder order> void traverse(auto &&cb) const {
        using U = std::decay_t<decltype(cb)>;
        return traverse_impl<U, true, order>(cb);
    }

    [[nodiscard]] bool is_leaf() const noexcept { return left() == nullptr; }

    [[nodiscard]] virtual BinaryTreeNode *left() noexcept = 0;

    [[nodiscard]] virtual const BinaryTreeNode *left() const noexcept = 0;

    [[nodiscard]] virtual BinaryTreeNode *right() noexcept = 0;

    [[nodiscard]] virtual const BinaryTreeNode *right() const noexcept = 0;

    [[nodiscard]] virtual BinaryTreeNode *parent() noexcept = 0;

    [[nodiscard]] virtual const BinaryTreeNode *parent() const noexcept = 0;

private:
    template<bool isConst> using node_pointer_t = std::conditional_t<isConst, const BinaryTreeNode *, BinaryTreeNode *>;

    template<bool isConst>
    using node_reference_t = std::conditional_t<isConst, const BinaryTreeNode &, BinaryTreeNode &>;

    template<typename Callback, bool isConst, Detail::ETraversalOrder order> void traverse_impl(Callback &cb) const {
        if constexpr (order == Detail::ETraversalOrder::BreadthFirst)
            return traverse_impl_impl_bfs<Callback, isConst>(cb);
        else
            return traverse_impl_impl<Callback, isConst, order>(cb);
    }

    template<typename Callback, bool isConst, Detail::ETraversalOrder order>
    void traverse_impl_impl(Callback &cb) const {
        auto lhs = [this, &cb]() {
            if (!is_leaf())
                left()->traverse_impl<Callback, isConst, order>(cb);
        };
        auto self = [this, &cb]() { std::invoke(cb, const_cast<node_reference_t<isConst>>(*this)); };
        auto rhs = [this, &cb]() {
            if (!is_leaf())
                right()->traverse_impl<Callback, isConst, order>(cb);
        };

        if constexpr (order == Detail::ETraversalOrder::PreOrder) {
            self();
            lhs();
            rhs();
        } else if constexpr (order == Detail::ETraversalOrder::InOrder) {
            lhs();
            rhs();
            self();
        } else if constexpr (order == Detail::ETraversalOrder::PostOrder) {
            lhs();
            self();
            rhs();
        }
    }

    template<typename Callback, bool isConst> void traverse_impl_impl_bfs(Callback &cb) const {
        std::list<node_pointer_t<isConst>> queue {};
        queue.push_back(const_cast<node_pointer_t<isConst>>(this));

        while (!queue.empty()) {
            const auto *current = queue.front();
            queue.pop_front();

            if constexpr (std::is_invocable_v<Callback, node_reference_t<isConst>>) {
                std::invoke(cb, static_cast<node_reference_t<isConst>>(*current));
            } else if constexpr (std::is_invocable_v<Callback, node_reference_t<isConst>, std::size_t>) {
                std::invoke(cb, static_cast<node_reference_t<isConst>>(*current), queue.size());
            } else {
                static_assert(std::is_same_v<Callback, Callback>, "Bad Callable");
            }

            if (current->left()) {
                queue.push_back(const_cast<node_pointer_t<isConst>>(current->left()));
                queue.push_back(const_cast<node_pointer_t<isConst>>(current->right()));
            }
        }
    }
};

struct BinaryTree {
    virtual ~BinaryTree() noexcept = default;

    [[nodiscard]] virtual BinaryTreeNode &root() noexcept = 0;

    [[nodiscard]] virtual const BinaryTreeNode &root() const noexcept = 0;
};

template<typename ShapeT = void>
class TraversableBVHNode
    : public BinaryTreeNode
    , public ShapeStore {
public:
    using shape_t = Shape::BoundableShapeT<ShapeT>;

    ~TraversableBVHNode() noexcept override = default;

    virtual std::pair<Point, Point> get_extents() const noexcept = 0;

    virtual std::span<const shape_t> get_shapes() const noexcept = 0;

    virtual Point get_center() const noexcept {
        auto extents = get_extents();
        return (extents.second - extents.first) / Real { 2 };
    }

    [[nodiscard]] std::optional<Intersection> intersect_impl(
        Ray ray, std::size_t &bound_checks, std::size_t &shape_checks) const noexcept override {
        if constexpr (Paths::ProgramConfig::embed_ray_stats)
            ++bound_checks;

        if (!Paths::Shape::AxisAlignedBox::ray_intersects(get_extents(), ray))
            return std::nullopt;

        if (left()) {
            std::optional<Intersection> best = std::nullopt;
            Intersection::replace(best,
                dynamic_cast<const TraversableBVHNode *>(left())->intersect_impl(ray, bound_checks, shape_checks));
            Intersection::replace(best,
                dynamic_cast<const TraversableBVHNode *>(right())->intersect_impl(ray, bound_checks, shape_checks));
            return best;
        }

        const auto s = get_shapes();
        if constexpr (Paths::ProgramConfig::embed_ray_stats)
            shape_checks += s.size();
        return Shape::intersect_linear(ray, s.begin(), s.end());
    }

    void calculate_extents() noexcept {
        std::pair<Point, Point> extents { { +sensible_inf, +sensible_inf, +sensible_inf },
            { -sensible_inf, -sensible_inf, -sensible_inf } };

        for (const auto &shapes = get_shapes(); const auto &s : shapes) {
            Shape::apply(s, [&extents](const auto &s) {
                extents.first = Maths::min(extents.first, s.m_extents.first);
                extents.second = Maths::max(extents.second, s.m_extents.second);
            });
        }

        extents.first = extents.first - epsilon_point;
        extents.second = extents.second + epsilon_point;
        set_extents(extents);
    }

    [[nodiscard]] std::array<std::size_t, 3> get_major_axes() const noexcept {
        const auto extents = get_extents();

        std::array<std::size_t, 3> ret { 0, 1, 2 };

        std::sort(ret.begin(), ret.end(),
            [axis_lengths = static_cast<Point>(extents.second - extents.first)](std::size_t lhs, std::size_t rhs) {
                const Real lhs_length = axis_lengths[lhs];
                const Real rhs_length = axis_lengths[rhs];

                return lhs_length > rhs_length;
            });

        return ret;
    }

protected:
    virtual void set_extents(std::pair<Point, Point>) noexcept = 0;
};

template<typename ShapeT = void>
class TraversableBVHTree
    : public BinaryTree
    , public ShapeStore {
public:
    ~TraversableBVHTree() noexcept override = default;

protected:
    [[nodiscard]] std::optional<Intersection> intersect_impl(
        Ray ray, std::size_t &bound_checks, std::size_t &isect_checks) const noexcept override {
        return dynamic_cast<const TraversableBVHNode<ShapeT> &>(this->root())
            .intersect_impl(ray, bound_checks, isect_checks);
    }
};

template<typename ShapeT = void> class ThreadableBVHNode : public TraversableBVHNode<ShapeT> {
public:
    using base_t = TraversableBVHNode<ShapeT>;
    using shape_t = typename TraversableBVHNode<ShapeT>::shape_t;

    ~ThreadableBVHNode() noexcept override = default;

    virtual void reorder_children(EMajorAxis axis) noexcept {
        if (this->is_leaf() || !this->left()->is_leaf() || !this->right()->is_leaf())
            return;

        auto lhs = dynamic_cast<const base_t *>(this->left())->get_center();
        auto rhs = dynamic_cast<const base_t *>(this->right())->get_center();

        bool do_reorder = false;
        switch (axis) {
        case EMajorAxis::PosX:
            do_reorder = lhs[0] > rhs[0];
            break;
        case EMajorAxis::NegX:
            do_reorder = lhs[0] < rhs[0];
            break;
        case EMajorAxis::PosY:
            do_reorder = lhs[1] > rhs[1];
            break;
        case EMajorAxis::NegY:
            do_reorder = lhs[1] < rhs[1];
            break;
        case EMajorAxis::PosZ:
            do_reorder = lhs[2] > rhs[2];
            break;
        case EMajorAxis::NegZ:
            do_reorder = lhs[2] < rhs[2];
            break;
        }

        if (do_reorder)
            swap_children();
    }

    /// This function should be valid when there are no children of the child nodes
    virtual void swap_children() noexcept = 0;

    [[nodiscard]] virtual std::size_t get_id() const noexcept = 0;

    virtual void set_id(std::size_t id) noexcept = 0;
};

template<typename ShapeT = void> struct ThreadableBVHTree : public TraversableBVHTree<ShapeT> {
    ~ThreadableBVHTree() noexcept override = default;
};

template<typename ShapeT = void> class IntrudableBVHNode : public ThreadableBVHNode<ShapeT> {
public:
    using shape_t = typename TraversableBVHNode<ShapeT>::shape_t;

    ~IntrudableBVHNode() noexcept override = default;

    using ThreadableBVHNode<ShapeT>::get_shapes; // have the const function in this scope

    virtual std::span<shape_t> get_shapes() noexcept = 0;

    bool split(std::size_t max_depth, std::size_t min_shapes) { return split_impl(max_depth, min_shapes, 0); }

    /// This function should only be valid when this->is_leaf()
    /// \param rhs_start_index The index to get_shapes() with which the right hand side node should begin with
    virtual void split_at(std::size_t rhs_start_index) noexcept = 0;

    /// This function should only be valid to call when children nodes are leaf nodes
    virtual void unsplit_once() noexcept = 0;

private:
    enum class EPartitionType { Middle, Median, Average };

    template<EPartitionType partitionType> struct Partitioner {
        static std::size_t partition(std::size_t) { static_assert(partitionType != partitionType); }
    };

    template<> struct Partitioner<EPartitionType::Median> {
        IntrudableBVHNode &m_self;

        std::size_t partition(std::size_t axis) {
            auto shapes = m_self.get_shapes();

            std::sort(shapes.begin(), shapes.end(), [axis](const auto &lhs, const auto &rhs) -> bool {
                auto lhs_center = Shape::apply(lhs, [](const auto &s) { return s.m_center; });
                auto rhs_center = Shape::apply(rhs, [](const auto &s) { return s.m_center; });
                return lhs_center[axis] < rhs_center[axis];
            });

            return shapes.size() / 2;
        }
    };

    template<> struct Partitioner<EPartitionType::Middle> {
        IntrudableBVHNode &m_self;

        std::size_t partition(std::size_t axis) {
            auto shapes = m_self.get_shapes();
            auto extents = m_self.get_extents();

            std::pair<Point, Point> rhs_extents = extents;
            const auto half_extent = (extents.second[axis] - extents.first[axis]) / static_cast<Real>(2);
            rhs_extents.first[axis] += half_extent;

            auto rhs_count = std::distance(
                std::partition(shapes.begin(), shapes.end(),
                    [&rhs_extents](const shape_t &s) -> bool {
                        return Shape::in_bounds(rhs_extents, Shape::apply(s, [](const auto &s) { return s.m_center; }));
                    }),
                shapes.end());

            return shapes.size() - rhs_count;
        }
    };

    inline std::size_t select_axis_major() { return this->get_major_axes()[0]; }

    inline std::size_t select_axis_round_robin(std::size_t depth) { return depth % 3; }

    /*std::pair<std::size_t, bool> PartitionMajorAxis(std::size_t minShapes) {
        auto extents = this->GetExtents();
        auto shapes = this->GetShapes();
        const auto splitOrder = this->GetMajorAxes();

        for (auto axis: splitOrder) {
            std::pair<Point, Point> rhsExtents = extents;
            const auto halfExtent = (extents.second[axis] - extents.first[axis]) / static_cast<Real>(2);
            rhsExtents.first[axis] += halfExtent;

            auto rhsCount = std::distance(
              std::partition(shapes.begin(), shapes.end(), [&rhsExtents](const shape_t &s) -> bool {
                  return Shape::InBounds(rhsExtents, Shape::Apply(s, [](const auto &s) {
                      return s.center;
                  }));
              }),
              shapes.end());

            LIBGFX_ASSERT(rhsCount >= 0 && static_cast<std::size_t>(rhsCount) <= shapes.size());
            if (static_cast<std::size_t>(rhsCount) < minShapes ||
                static_cast<std::size_t>(shapes.size() - rhsCount) < minShapes)
                continue;

            return {shapes.size() - rhsCount, true};
        }

        return {0, false};
    }*/

    bool split_impl(std::size_t max_depth, std::size_t min_shapes, std::size_t depth) {
        this->calculate_extents();
        auto shapes = this->get_shapes();

        if (depth >= max_depth)
            return false;
        if (shapes.size() <= min_shapes)
            return false;

        auto major_axes = this->get_major_axes();
        auto partitioner = Partitioner<EPartitionType::Middle> { *this };

        for (auto axis : major_axes) {
            auto split_point = partitioner.partition(axis);

            if (static_cast<std::size_t>(split_point) < min_shapes
                || static_cast<std::size_t>(shapes.size() - split_point) < min_shapes)
                continue;

            this->split_at(split_point);

            dynamic_cast<IntrudableBVHNode *>(this->left())->split_impl(max_depth, min_shapes, depth + 1);
            dynamic_cast<IntrudableBVHNode *>(this->right())->split_impl(max_depth, min_shapes, depth + 1);

            return true;
        }

        return false;
    }
};

template<typename ShapeT = void> struct IntrudableBVHTree : public ThreadableBVHTree<ShapeT> {
    ~IntrudableBVHTree() noexcept override = default;
};

}
