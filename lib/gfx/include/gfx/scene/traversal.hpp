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

namespace Gfx::Concepts::BVH {

/// Traversable concept \n
/// Runtime requirements: \n
/// {n, cn}.left() == nullptr iff {n, cn}.right() == nullptr \n
/// \tparam T
template<typename T>
concept Traversable = requires(
  T &t, const T &ct,
  typename T::node_t &n, typename T::const_node_t &cn,
  typename T::node_pointer_t np, typename T::const_node_pointer_t cnp) {
    { t.Root() } -> std::same_as<typename T::node_t &>;
    { ct.Root() } -> std::same_as<typename T::const_node_t &>;

    { n.Left() } -> std::same_as<typename T::node_pointer_t>;
    { n.Right() } -> std::same_as<typename T::node_pointer_t>;
    { n.Parent() } -> std::same_as<typename T::node_pointer_t>;
    { cn.Left() } -> std::same_as<typename T::const_node_pointer_t>;
    { cn.Right() } -> std::same_as<typename T::const_node_pointer_t>;
    { cn.Parent() } -> std::same_as<typename T::const_node_pointer_t>;

    { *np } -> std::convertible_to<typename T::node_t &>;
    { *cnp } -> std::convertible_to<typename T::const_node_t &>;
};

/// A Traversable<T> that is also a BVH. \n
/// Basically an added requirement of being able to call Shape::LinearIntersect on a Traversable<T> tree
/// \tparam T
template<typename T>
concept TraversableBVH = requires(const T &ct, typename T::const_node_t &cn, Ray r) {
    Traversable<T>;

    { ct.GetExtents(cn) } -> std::convertible_to<std::pair<Point, Point>>;
    { ct.GetShapes(cn) } -> std::same_as<std::span<const typename T::shape_t>>;
};

/// A BVH that can be made into a threaded BVH. \n
/// The children of a node_t of a tree can be reordered according to an axis. \n
/// The nodes must contain a mutable member called 'id' that must be at least as large as std::size_t.
/// \tparam T
template<typename T>
concept ThreadableBVH = requires(const T &ct, typename T::node_t &n, MajorAxis axis) {
    TraversableBVH<T>;

    { n.ReorderChildren(axis) } -> std::same_as<void>;
    { n.id } -> std::convertible_to<std::size_t>;
};

/// A BVH that can be "intruded" which means that: \n
///  - It's shapes can be moved around and altered freely. \n
///  - It can be split at a given index, the split does nothing if the given index is invalid. \n
/// This makes it so that the BVH can be split using Splitter<T>
/// \tparam T
template<typename T>
concept IntrudableBVH = requires(T &t, typename T::node_t &n, std::size_t atIndex) {
    { t.GetShapes(n) } -> std::same_as<std::span<typename T::shape_t>>;
    { t.Split(n, atIndex) } -> std::same_as<void>;
    { t.Unsplit(n) } -> std::same_as<void>;
};

}

namespace Gfx::BVH::Detail {

enum class TraversalOrder {
    PreOrder, InOrder, PostOrder,
    BreadthFirst,
};

template<Concepts::BVH::Traversable T>
struct Traverser {
    template<TraversalOrder order>
    static void Traverse(T &tree, auto &&cb) {
        using U = std::decay_t<decltype(cb)>;
        return TraverseImpl<U, false, order>(&tree.Root(), cb);
    }

    template<TraversalOrder order>
    static void Traverse(const T &tree, auto &&cb) {
        using U = std::decay_t<decltype(cb)>;
        return TraverseImpl<U, true, order>(&tree.Root(), cb);
    }

  private:
    template<bool isConst>
    using node_pointer_t = std::conditional_t<isConst, typename T::const_node_pointer_t, typename T::node_pointer_t>;

    template<bool isConst>
    using node_reference_t = std::conditional_t<isConst, typename T::const_node_t &, typename T::node_t &>;

    template<typename Callback, bool isConst, TraversalOrder order>
    static void TraverseImpl(node_pointer_t<isConst> node, Callback &cb) {
        if constexpr (order == TraversalOrder::BreadthFirst) return TraverseImplImplBFS<Callback, isConst>(node, cb);
        else return TraverseImplImpl<Callback, isConst, order>(node, cb);
    }

    template<typename Callback, bool isConst, TraversalOrder order>
    static void TraverseImplImpl(node_pointer_t<isConst> node, Callback &cb) {
        auto lhs = [&cb, &node]() { if (node->Left()) TraverseImpl<Callback, isConst, order>(node->Left(), cb); };
        auto self = [&cb, &node]() { std::invoke(cb, *node); };
        auto rhs = [&cb, &node]() { if (node->Right()) TraverseImpl<Callback, isConst, order>(node->Right(), cb); };

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

    template<typename Callback, bool isConst>
    static void TraverseImplImplBFS(node_pointer_t<isConst> node, Callback &cb) {
        std::list<node_pointer_t<isConst>> queue{};
        queue.push_back(node);

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
                queue.push_back(current->Left());
                queue.push_back(current->Right());
            }
        }
    }
};

static bool DecideReorder(MajorAxis axis, auto lhs, auto rhs) {
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
    return doReorder;
}

enum class SplitMethod {
    MajorAxis
};

template<SplitMethod method, Concepts::BVH::IntrudableBVH T>
struct Splitter {};

template<Concepts::BVH::IntrudableBVH T>
struct Splitter<SplitMethod::MajorAxis, T> {
    using node_t = typename T::node_t;
    using shape_t = typename T::shape_t;

    static void Split(T &tree, std::size_t maxDepth, std::size_t minShapes) {

    }

    static void SplitSingle(T &tree, node_t &node, std::size_t axis, Real margin) {
        auto &nodeShapes = tree.GetShapes(node);
        const auto nodeExtents = tree.GetExtents(node);
        const auto axisExtent = (nodeExtents.second - nodeExtents.first)[axis];
        const auto splitPoint = Maths::Lerp<decltype(axisExtent)>(0, axisExtent, margin);

        const auto rhsStart = std::partition(
          nodeShapes.begin(),
          nodeShapes.end(),
          [k = splitPoint + nodeExtents.first[axis], axis](const shape_t &s) -> bool { return s.center[axis] < k; }
        );

        tree.Split(node, std::distance(nodeShapes.begin(), rhsStart));
    }
};

template<Concepts::BVH::TraversableBVH T>
inline std::optional<Intersection> IntersectTraversable(const T &tree, const typename T::node_t &node, Ray ray, std::size_t &boundChecks, std::size_t &shapeChecks) {
    if constexpr (Gfx::ProgramConfig::EmbedRayStats) ++boundChecks;

    if (!Gfx::Shape::AABox::EIntersects(tree.GetExtents(node), ray)) return std::nullopt;

    if (node.Left()) {
        std::optional<Intersection> best = std::nullopt;
        Intersection::Replace(best, IntersectTraversable<T>(tree, *(node.Left()), ray, boundChecks, shapeChecks));
        Intersection::Replace(best, IntersectTraversable<T>(tree, *(node.Right()), ray, boundChecks, shapeChecks));
        return best;
    } else {
        const auto s = tree.GetShapes(node);
        if constexpr (Gfx::ProgramConfig::EmbedRayStats) shapeChecks += s.size();
        return Shape::IntersectLinear(ray, s.begin(), s.end());
    }
}

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
};

template<typename ShapeT = void>
struct TraversableBVHTree : public TraversableTree { ~TraversableBVHTree() noexcept override = default; };

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
    using base_t = TraversableBVHNode<ShapeT>;
    using shape_t = typename TraversableBVHNode<ShapeT>::shape_t;

    ~IntrudableBVHNode() noexcept override = default;

    virtual std::span<shape_t> GetShapes() noexcept = 0;

    /// This function should only be valid when this->IsLeaf()
    /// \param rhsStartIndex The index to GetShapes() with which the right hand side node should begin with
    virtual void Split(std::size_t rhsStartIndex) noexcept = 0;

    /// This function should only be valid to call when children nodes are leaf nodes
    virtual void UnsplitOnce() noexcept = 0;
};

template<typename ShapeT = void>
struct IntrudableBVHTree : public ThreadableBVHTree<ShapeT> { ~IntrudableBVHTree() noexcept override = default; };

}
