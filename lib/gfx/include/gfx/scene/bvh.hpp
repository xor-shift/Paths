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
    typedef Shape::boundable_shape_t <ShapeT> shape_t;

    std::vector<shape_t> shapes{};
    std::pair<Point, Point> extents{};
    std::array<std::unique_ptr<FatBVHNode>, 2> children{nullptr, nullptr};
    FatBVHNode *parent{nullptr};
    std::size_t id = 0; //for TBVH construction

    bool Split(std::size_t maxDepth, std::size_t minShapes) noexcept { return SplitImpl(maxDepth, minShapes, 0); }

    template<TraversalOrder order = TraversalOrder::InOrder>
    void Traverse(auto &&cb) const noexcept { return TraverseImpl<std::decay_t<decltype(cb)>, false, order>(std::forward(cb)); }

    template<typename Callback, TraversalOrder order = TraversalOrder::InOrder>
    void Traverse(Callback &&cb) noexcept { return TraverseImpl<Callback, true, order>(std::forward(cb)); }

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
    template<typename Callback, bool constCast, TraversalOrder order>
    void TraverseImpl(Callback &&cb) const noexcept {
        auto lhs = [this, cb = std::forward(cb)]() { if (children[0]) children[0]->template TraverseImpl<Callback, constCast, order>(cb); };
        auto self = [this, cb = std::forward(cb)]() {
            if constexpr (constCast) std::invoke(cb, const_cast<FatBVHNode<ShapeT> &>(*this));
            else std::invoke(cb, static_cast<const FatBVHNode<ShapeT> &>(*this));
        };
        auto rhs = [this, cb = std::forward(cb)]() { if (children[1]) children[1]->template TraverseImpl<Callback, constCast, order>(cb); };

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

template<typename ShapeT = void>
class ThreadedBVH {
    struct Node {
        std::array<std::size_t, 2> shapeExtents{};
        std::pair<Point, Point> extents{};
        //std::array<std::size_t, 2> links{};
        std::size_t skipLink = 0;
    };

  public:
    typedef Shape::boundable_shape_t <ShapeT> shape_t;
    static constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();

    /// This will modify the given tree in a non-destructive manner (will reorder children of nodes as needed)
    /// \param fat
    explicit ThreadedBVH(FatBVHNode<ShapeT> &fat) noexcept {
        auto ReorderSingle = [](MajorAxis axis, FatBVHNode<ShapeT> &node) {
            if (!node.children[0]) return;

            Point lhs = (node.children[0]->extents.second - node.children[0]->extents.first) / static_cast<Real>(2);
            Point rhs = (node.children[0]->extents.second - node.children[0]->extents.first) / static_cast<Real>(2);

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
                node.children[0].swap(node.children[1]);
        };

        auto ReassignIDs = [ReorderSingle, &fat](MajorAxis reorderAxis) {
            fat.Traverse<TraversalOrder::PreOrder>([ReorderSingle, id = 0](FatBVHNode<ShapeT> &node) mutable {
                ReorderSingle(node);
                node.id = id;
            });
        };

        std::vector<Node> tNodes;
        fat.Traverse<TraversalOrder::PreOrder>([ReorderSingle](FatBVHNode<ShapeT> &node) {
            ReorderSingle(MajorAxis::PosX, node);

            std::size_t miss = npos;
        });
    }

  protected:
  private:
    std::vector<shape_t> shapes{};
    std::pair<Point, Point> extents{};

    std::array<std::vector<Node>, 6> nodes{};

    std::size_t GetMissLink(const FatBVHNode<ShapeT> &node) {
        if (!node.children[0]) return node.id + 1;
    }
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
