#pragma once

#include <gfx/shape/shapes.hpp>

#include <span>
#include <stack>

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

/// The base BVH node type, used mainly for computing other representations but can be used as a store standalone
/// \tparam genericBoundable if true, Shape::BoundableShape is used for the shapes vector
/// \tparam ShapeT if !genericBoundable, this type is used for the shapes vector
template<typename ShapeT = void>
struct FatBVHNode final : public ShapeStore {
    typedef Shape::boundable_shape_t<ShapeT> shape_t;

    std::vector<shape_t> shapes{};
    std::pair<Point, Point> extents{};
    std::array<std::unique_ptr<FatBVHNode>, 2> children{nullptr, nullptr};

    bool Split(std::size_t maxDepth, std::size_t minShapes) noexcept { return SplitImpl(maxDepth, minShapes, 0); }

    /// In-order traverses the tree, calling the given callback with every node's cref
    /// \param cb callback
    void Traverse(auto &&cb) const noexcept {
        if (children[0]) children[0]->Traverse(cb);
        std::invoke(cb, static_cast<const FatBVHNode<ShapeT> &>(*this));
        if (children[1]) children[1]->Traverse(cb);
    }

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
        ret[1]->extents = rhsExtents;

        for (const auto &s: shapes) {
            Shape::Apply(s, [this, &ret](const auto &s) {
                if (Gfx::Shape::InBounds(ret[0]->extents, s.center)) ret[0]->shapes.push_back(s);
                else ret[1]->shapes.push_back(s);
            });
        }

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
    [[nodiscard]] std::optional<Intersection> IntersectImpl(Ray ray) const noexcept override {
        if (!Gfx::Shape::AABox::EIntersects(extents, ray)) return std::nullopt;

        std::optional<Intersection> best = std::nullopt;

        if (shapes.empty()) {
            Intersection::Replace(best, children[0]->IntersectImpl(ray));
            Intersection::Replace(best, children[1]->IntersectImpl(ray));
        } else {
            for (const auto &s: shapes) {
                Shape::Apply(s, [ray, &best]<Concepts::Shape T>(const T &s) {
                    Intersection::Replace(best, std::move(s.Intersect(ray)));
                });
            }
        }

        return best;
    }
};

struct ThreadedBVHNode {
    std::array<std::size_t, 2> shapeExtents;
    std::pair<Point, Point> extents{};
    std::array<std::size_t, 2> links;
};

}

namespace Gfx::BVH {

template<typename ShapeT = void>
std::shared_ptr<Detail::FatBVHNode<ShapeT>> LinearToFat(Gfx::LinearShapeStore<ShapeT> &store, std::size_t maxDepth = 7, std::size_t minShapes = 4) {
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

    ret->Split(maxDepth, minShapes);

    return ret;
}

}
