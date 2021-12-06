#pragma once

#include <concepts>
#include <type_traits>

#include <gfx/common.hpp>
#include <gfx/ray.hpp>
#include <gfx/shape/shapes.hpp>

namespace Gfx {

enum class MajorAxis : int {
    PosX = 0,
    NegX = 1,
    PosY = 2,
    NegY = 3,
    PosZ = 4,
    NegZ = 5,
};

}

namespace Gfx::BVH::Detail {

enum class TraversalOrder {
    PreOrder, InOrder, PostOrder,
    BreadthFirst,
};

}

namespace Gfx::BVH {

/// Requirement for Left() and Right() functions: \n
///  - Left() should return nullptr iff Right() returns nullptr \n
///  - Parent() should return nullptr iff *this is not a children of any other Traversable \n
class Traversable {
  public:
    virtual ~Traversable() noexcept = default;

    template<Detail::TraversalOrder order>
    void Traverse(auto &&cb) {
        using U = std::decay_t<decltype(cb)>;
        return TraverseImpl<U, false, order>(cb);
    }

    template<Detail::TraversalOrder order>
    void Traverse(auto &&cb) const {
        using U = std::decay_t<decltype(cb)>;
        return TraverseImpl<U, true, order>(cb);
    }

    [[nodiscard]] bool IsLeaf() const noexcept { return !Left(); }

    [[nodiscard]] virtual Traversable *Left() noexcept = 0;

    [[nodiscard]] virtual const Traversable *Left() const noexcept = 0;

    [[nodiscard]] virtual Traversable *Right() noexcept = 0;

    [[nodiscard]] virtual const Traversable *Right() const noexcept = 0;

    [[nodiscard]] virtual Traversable *Parent() noexcept = 0;

    [[nodiscard]] virtual const Traversable *Parent() const noexcept = 0;

  private:
    template<bool isConst>
    using node_pointer_t = std::conditional_t<isConst, const Traversable *, Traversable *>;

    template<bool isConst>
    using node_reference_t = std::conditional_t<isConst, const Traversable &, Traversable &>;

    template<typename Callback, bool isConst, Detail::TraversalOrder order>
    void TraverseImpl(Callback &cb) const {
        if constexpr (order == Detail::TraversalOrder::BreadthFirst) return TraverseImplImplBFS<Callback, isConst>(cb);
        else return TraverseImplImpl<Callback, isConst, order>(cb);
    }

    template<typename Callback, bool isConst, Detail::TraversalOrder order>
    void TraverseImplImpl(Callback &cb) const {
        auto lhs = [this, &cb]() { if (!IsLeaf()) Left()->TraverseImpl<Callback, isConst, order>(cb); };
        auto self = [this, &cb]() { std::invoke(cb, const_cast<node_reference_t<isConst>>(*this)); };
        auto rhs = [this, &cb]() { if (!IsLeaf()) Right()->TraverseImpl<Callback, isConst, order>(cb); };

        if constexpr (order == Detail::TraversalOrder::PreOrder) {
            self();
            lhs();
            rhs();
        } else if constexpr (order == Detail::TraversalOrder::InOrder) {
            lhs();
            rhs();
            self();
        } else if constexpr (order == Detail::TraversalOrder::PostOrder) {
            lhs();
            self();
            rhs();
        }
    }

    template<typename Callback, bool isConst>
    void TraverseImplImplBFS(Callback &cb) const {
        std::list<node_pointer_t<isConst>> queue{};
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

            if (current->Left()) {
                queue.push_back(const_cast<node_pointer_t<isConst>>(current->Left()));
                queue.push_back(const_cast<node_pointer_t<isConst>>(current->Right()));
            }
        }
    }
};

struct TraversableTree {
    virtual ~TraversableTree() noexcept = default;

    [[nodiscard]] virtual Traversable &Root() noexcept = 0;

    [[nodiscard]] virtual const Traversable &Root() const noexcept = 0;
};

template<typename ShapeT = void>
class TraversableBVHNode : public Traversable, public ShapeStore {
  public:
    using shape_t = Shape::boundable_shape_t<ShapeT>;

    ~TraversableBVHNode() noexcept override = default;

    virtual std::pair<Point, Point> GetExtents() const noexcept = 0;

    virtual std::span<const shape_t> GetShapes() const noexcept = 0;

    virtual Point GetCenter() const noexcept {
        auto extents = GetExtents();
        return (extents.second - extents.first) / Real{2};
    }

    [[nodiscard]] std::optional<Intersection> IntersectImpl(Ray ray, std::size_t &boundChecks, std::size_t &shapeChecks) const noexcept override {
        if constexpr (Gfx::ProgramConfig::EmbedRayStats) ++boundChecks;

        if (!Gfx::Shape::AABox::EIntersects(GetExtents(), ray)) return std::nullopt;

        if (Left()) {
            std::optional<Intersection> best = std::nullopt;
            Intersection::Replace(best, dynamic_cast<const TraversableBVHNode *>(Left())->IntersectImpl(ray, boundChecks, shapeChecks));
            Intersection::Replace(best, dynamic_cast<const TraversableBVHNode *>(Right())->IntersectImpl(ray, boundChecks, shapeChecks));
            return best;
        } else {
            const auto s = GetShapes();
            if constexpr (Gfx::ProgramConfig::EmbedRayStats) shapeChecks += s.size();
            return Shape::IntersectLinear(ray, s.begin(), s.end());
        }
    }

    void CalculateExtents() noexcept {
        std::pair<Point, Point> extents{
          {+sensibleInf, +sensibleInf, +sensibleInf},
          {-sensibleInf, -sensibleInf, -sensibleInf}
        };

        for (const auto &shapes = GetShapes(); const auto &s: shapes) {
            Shape::Apply(s, [&extents](const auto &s) {
                extents.first = Maths::Min(extents.first, s.extents.first);
                extents.second = Maths::Max(extents.second, s.extents.second);
            });
        }

        extents.first = extents.first - epsilonPoint;
        extents.second = extents.second + epsilonPoint;
        SetExtents(extents);
    }

    [[nodiscard]] std::array<std::size_t, 3> GetMajorAxes() const noexcept {
        const auto extents = GetExtents();

        std::array<std::size_t, 3> ret{0, 1, 2};

        std::sort(ret.begin(), ret.end(), [axisLengths = static_cast<Point>(extents.second - extents.first)](std::size_t lhs, std::size_t rhs) {
            const Real lhsLength = axisLengths[lhs];
            const Real rhsLength = axisLengths[rhs];

            return lhsLength > rhsLength;
        });

        return ret;
    }

  protected:
    virtual void SetExtents(std::pair<Point, Point>) noexcept = 0;
};

template<typename ShapeT = void>
class TraversableBVHTree : public TraversableTree, public ShapeStore {
  public:
    ~TraversableBVHTree() noexcept override = default;

  protected:
    [[nodiscard]] std::optional<Intersection> IntersectImpl(Ray ray, std::size_t &boundChecks, std::size_t &isectChecks) const noexcept override {
        return dynamic_cast<const TraversableBVHNode<ShapeT> &>(this->Root()).IntersectImpl(ray, boundChecks, isectChecks);
    }
};

template<typename ShapeT = void>
class ThreadableBVHNode : public TraversableBVHNode<ShapeT> {
  public:
    using base_t = TraversableBVHNode<ShapeT>;
    using shape_t = typename TraversableBVHNode<ShapeT>::shape_t;

    ~ThreadableBVHNode() noexcept override = default;

    virtual void ReorderChildren(MajorAxis axis) noexcept {
        if (this->IsLeaf() || !this->Left()->IsLeaf() || !this->Right()->IsLeaf()) return;

        auto
          lhs = dynamic_cast<const base_t *>(this->Left())->GetCenter(),
          rhs = dynamic_cast<const base_t *>(this->Right())->GetCenter();

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

        if (doReorder) SwapChildren();
    }

    /// This function should be valid when there are no children of the child nodes
    virtual void SwapChildren() noexcept = 0;

    [[nodiscard]] virtual std::size_t GetID() const noexcept = 0;

    virtual void SetID(std::size_t id) noexcept = 0;
};

template<typename ShapeT = void>
struct ThreadableBVHTree : public TraversableBVHTree<ShapeT> { ~ThreadableBVHTree() noexcept override = default; };

template<typename ShapeT = void>
class IntrudableBVHNode : public ThreadableBVHNode<ShapeT> {
  public:
    using shape_t = typename TraversableBVHNode<ShapeT>::shape_t;

    ~IntrudableBVHNode() noexcept override = default;

    using ThreadableBVHNode<ShapeT>::GetShapes; //have the const function in this scope

    virtual std::span<shape_t> GetShapes() noexcept = 0;

    bool Split(std::size_t maxDepth, std::size_t minShapes) {
        return SplitImpl(maxDepth, minShapes, 0);
    }

  protected:
    /// This function should only be valid when this->IsLeaf()
    /// \param rhsStartIndex The index to GetShapes() with which the right hand side node should begin with
    virtual void SplitAt(std::size_t rhsStartIndex) noexcept = 0;

    /// This function should only be valid to call when children nodes are leaf nodes
    virtual void UnsplitOnce() noexcept = 0;

  private:
    bool SplitImpl(std::size_t maxDepth, std::size_t minShapes, std::size_t depth) {
        this->CalculateExtents();
        auto shapes = this->GetShapes();
        auto extents = this->GetExtents();

        if (depth >= maxDepth) return false;
        if (shapes.size() <= minShapes) return false;

        const auto splitOrder = this->GetMajorAxes();

        for (auto axis: splitOrder) {
            std::pair<Point, Point> lhsExtents = extents, rhsExtents = extents;
            const auto halfExtent = (extents.second[axis] - extents.first[axis]) / static_cast<Real>(2);
            lhsExtents.second[axis] -= halfExtent;
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

            this->SplitAt(shapes.size() - rhsCount);

            dynamic_cast<IntrudableBVHNode *>(this->Left())->SplitImpl(maxDepth, minShapes, depth + 1);
            dynamic_cast<IntrudableBVHNode *>(this->Right())->SplitImpl(maxDepth, minShapes, depth + 1);

            return true;
        }

        return false;
    }
};

template<typename ShapeT = void>
struct IntrudableBVHTree : public ThreadableBVHTree<ShapeT> { ~IntrudableBVHTree() noexcept override = default; };

}
