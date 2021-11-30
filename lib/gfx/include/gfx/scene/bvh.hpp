#pragma once

#include <gfx/shape/shapes.hpp>

#include <span>
#include <stack>

namespace Gfx::Concepts::BVH {

template<typename T>
concept Traversable = requires(T &t, const T &ct) {
    { t.left() } -> std::same_as<T *>;
    { t.right() } -> std::same_as<T *>;
    { ct.left() } -> std::same_as<const T *>;
    { ct.right() } -> std::same_as<const T *>;
};

}

namespace Gfx::BVH::Detail {

enum class MajorAxis : int {
    PosX = 0,
    NegX = 1,
    PosY = 2,
    NegY = 3,
    PosZ = 4,
    NegZ = 5,
};

enum class TraversalOrder {
    PreOrder, InOrder, PostOrder,
};


template<Concepts::BVH::Traversable T>
struct Traverser {
    template<TraversalOrder order>
    static void Traverse(T &tree, auto &&cb) {
        using U = std::decay_t<decltype(cb)>;
        return TraverseImpl<U, false, order>(tree, cb);
    }

    template<TraversalOrder order>
    static void Traverse(const T &tree, auto &&cb) {
        using U = std::decay_t<decltype(cb)>;
        return TraverseImpl<U, true, order>(tree, cb);
    }

  private:
    template<typename Callback, bool isConst, TraversalOrder order>
    static void TraverseImpl(std::conditional_t<isConst, const T &, T &> node, Callback &cb) {
        auto lhs = [&cb, &node]() { if (node.left()) TraverseImpl<Callback, isConst, order>(*node.left(), cb); };
        auto self = [&cb, &node]() { std::invoke(cb, node); };
        auto rhs = [&cb, &node]() { if (node.right()) TraverseImpl<Callback, isConst, order>(*node.right(), cb); };

        if constexpr (order == TraversalOrder::PreOrder) {
            self();
            lhs();
            rhs();
        } else if constexpr (order == TraversalOrder::InOrder) {
            lhs();
            rhs();
            self();
        } else if constexpr (order == TraversalOrder::PostOrder) {
            lhs();
            self();
            rhs();
        }
    }
};

/// The base BVH node type, used mainly for computing other representations but can be used as a store standalone
/// \tparam genericBoundable if true, Shape::BoundableShape is used for the shapes vector
/// \tparam ShapeT if !genericBoundable, this type is used for the shapes vector
template<typename ShapeT = void>
struct FatBVHNode final : public ShapeStore {
    typedef Shape::boundable_shape_t <ShapeT> shape_t;

    std::vector<shape_t> shapes{};
    std::pair<Point, Point> extents{};
    std::array<std::unique_ptr<FatBVHNode>, 2> children{nullptr, nullptr};
    FatBVHNode *parent{nullptr};
    std::size_t id = 0; //for TBVH construction
    std::size_t totalShapeCount = 0; //memoization

    bool Split(std::size_t maxDepth, std::size_t minShapes) noexcept { return SplitImpl(maxDepth, minShapes, 0); }

    [[nodiscard]] FatBVHNode *left() { return children[0].get(); }

    [[nodiscard]] const FatBVHNode *left() const { return children[0].get(); }

    [[nodiscard]] FatBVHNode *right() { return children[1].get(); }

    [[nodiscard]] const FatBVHNode *right() const { return children[1].get(); }

    /// Breadth-first traverses the tree, calling the callback with every node's cref and optionally with the stack depth at the time of call (first call, for example, will have 0 passed in as the depth argument)
    /// \tparam Callable
    /// \param cb
    template<typename Callable>
    void BreadthFirstTraverse(Callable &&cb) const noexcept {
        std::list<const FatBVHNode *> queue{};
        queue.push_back(this);

        while (!queue.empty()) {
            const auto *current = queue.front();
            queue.pop_front();

            if constexpr (std::is_invocable_v<Callable, const FatBVHNode &>) {
                std::invoke(cb, static_cast<const FatBVHNode &>(*current));
            } else if constexpr (std::is_invocable_v<Callable, const FatBVHNode &, std::size_t>) {
                std::invoke(cb, static_cast<const FatBVHNode &>(*current), queue.size());
            } else {
                static_assert(std::is_same_v<Callable, Callable>, "Bad Callable");
            }

            for (const auto &c: current->children) if (c) queue.push_back(c.get());
        }
    }

    void ReorderChildren(MajorAxis axis) noexcept {
        if (!children[0]) return;

        Point lhs = (children[0]->extents.second - children[0]->extents.first) / static_cast<Real>(2);
        Point rhs = (children[0]->extents.second - children[0]->extents.first) / static_cast<Real>(2);

        bool doReorder = false;
        switch (axis) {
            case MajorAxis::PosX:
                doReorder = lhs[0] > rhs[0];
                break;
            case MajorAxis::NegX:
                doReorder = lhs[0] < rhs[0];
                break;
            case MajorAxis::PosY:
                doReorder = lhs[1] > rhs[1];
                break;
            case MajorAxis::NegY:
                doReorder = lhs[1] < rhs[1];
                break;
            case MajorAxis::PosZ:
                doReorder = lhs[2] > rhs[2];
                break;
            case MajorAxis::NegZ:
                doReorder = lhs[2] < rhs[2];
                break;
        }

        if (doReorder)
            children[0].swap(children[1]);
    }

  private:
    bool SplitImpl(std::size_t maxDepth, std::size_t minShapes, std::size_t depth) noexcept {
        CalculateExtents();

        if (depth >= maxDepth) return false;
        if (shapes.size() <= minShapes) return false;

        const auto splitOrder = GetMajorAxes();

        for (auto axis: splitOrder) {
            auto newChildren = GetSplit(axis);

            bool ok = true;
            for (auto &c: newChildren) {
                if (c->shapes.size() < 4) {
                    ok = false;
                    break;
                }
            }

            if (!ok) continue;

            children = std::move(newChildren);
            shapes.clear();

            for (auto &c: children) c->SplitImpl(maxDepth, minShapes, depth + 1);

            return true;
        }

        return false;
    }

    [[nodiscard]] std::array<std::unique_ptr<FatBVHNode>, 2> GetSplit(std::size_t axis) {
        std::pair<Point, Point> lhsExtents = extents, rhsExtents = extents;
        const auto halfExtent = (extents.second[axis] - extents.first[axis]) / static_cast<Real>(2);
        lhsExtents.second[axis] -= halfExtent;
        rhsExtents.first[axis] += halfExtent;

        std::array<std::unique_ptr<FatBVHNode>, 2> ret{
          std::make_unique<FatBVHNode>(), std::make_unique<FatBVHNode>(),
        };

        ret[0]->extents = lhsExtents;
        ret[0]->parent = this;
        ret[1]->extents = rhsExtents;
        ret[1]->parent = this;

        for (const auto &s: shapes) {
            Shape::Apply(s, [this, &ret](const auto &s) {
                if (Gfx::Shape::InBounds(ret[0]->extents, s.center)) ret[0]->shapes.push_back(s);
                else ret[1]->shapes.push_back(s);
            });
        }

        ret[0]->totalShapeCount = ret[0]->shapes.size();
        ret[1]->totalShapeCount = ret[1]->shapes.size();

        return ret;
    }

    [[nodiscard]] std::array<std::size_t, 3> GetMajorAxes() const noexcept {
        std::array<std::size_t, 3> ret{0, 1, 2};

        std::sort(ret.begin(), ret.end(), [axisLengths = static_cast<Point>(extents.second - extents.first)](std::size_t lhs, std::size_t rhs) {
            const Real lhsLength = axisLengths[lhs];
            const Real rhsLength = axisLengths[rhs];

            return lhsLength > rhsLength;
        });

        return ret;
    }

    void CalculateExtents() noexcept {
        extents.first = {+sensibleInf, +sensibleInf, +sensibleInf};
        extents.second = {-sensibleInf, -sensibleInf, -sensibleInf};

        for (const auto &s: shapes) {
            Shape::Apply(s, [this](const auto &s) {
                extents.first = Maths::Min(extents.first, s.extents.first);
                extents.second = Maths::Max(extents.second, s.extents.second);
            });
        }

        extents.first = extents.first - epsilonPoint;
        extents.second = extents.second + epsilonPoint;
    }

  protected:
    [[nodiscard]] std::optional<Intersection> IntersectImpl(Ray ray, std::size_t &boundChecks, std::size_t &shapeChecks) const noexcept override {
        if constexpr (Gfx::ProgramConfig::EmbedRayStats) ++boundChecks;

        if (!Gfx::Shape::AABox::EIntersects(extents, ray)) return std::nullopt;

        std::optional<Intersection> best = std::nullopt;

        if (shapes.empty()) {
            Intersection::Replace(best, children[0]->IntersectImpl(ray, boundChecks, shapeChecks));
            Intersection::Replace(best, children[1]->IntersectImpl(ray, boundChecks, shapeChecks));
        } else {
            if constexpr (Gfx::ProgramConfig::EmbedRayStats) shapeChecks += shapes.size();
            best = Shape::IntersectLinear(ray, shapes.cbegin(), shapes.cend());
        }

        return best;
    }
};

template<typename ShapeT = void>
struct FatBVHTree : public ShapeStore {

};

}

namespace Gfx::BVH {

template<typename ShapeT = void>
std::shared_ptr<Detail::FatBVHNode<ShapeT>> LinearToFat(const Gfx::LinearShapeStore<ShapeT> &store, std::size_t maxDepth = 7, std::size_t minShapes = 4) {
    auto ret = std::make_shared<Detail::FatBVHNode<ShapeT>>();

    if constexpr (Concepts::Boundable<ShapeT>) {
        ret->shapes = store.shapes;
    } else {
        std::vector<Gfx::Shape::boundable_shape_t<>> extracted;
        for (const auto &s: store.shapes) {
            Gfx::Shape::Apply(s, [&extracted](auto &&s) {
                using T = std::decay_t<decltype(s)>;

                if constexpr (Gfx::Concepts::Boundable<T>) extracted.emplace_back(s);
            });
        }
        ret->shapes = std::move(extracted);
    }

    ret->totalShapeCount = ret->shapes.size();
    ret->Split(maxDepth, minShapes);

    return ret;
}

}
