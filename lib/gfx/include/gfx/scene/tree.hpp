#pragma once

#include "traversal.hpp"

namespace Gfx::BVH::Detail {

template<typename ShapeT>
struct FatTreeNode final : public IntrudableBVHNode<ShapeT>, public ShapeStore {
    using base_t = IntrudableBVHNode<ShapeT>;
    typedef Shape::boundable_shape_t <ShapeT> shape_t;
    typedef std::vector<shape_t> store_t;

    std::shared_ptr<store_t> store;

    std::pair<std::size_t, std::size_t> shapeExtents{};
    std::pair<Gfx::Point, Gfx::Point> extents{};

    std::array<std::unique_ptr<FatTreeNode>, 2> children{nullptr, nullptr};

    //memoization, should be set on initialization
    std::size_t depth = 0;
    FatTreeNode *parent = nullptr;
    std::size_t id = 0;

    [[nodiscard]] FatTreeNode *Left() noexcept { return children[0].get(); }

    [[nodiscard]] const FatTreeNode *Left() const noexcept { return children[0].get(); }

    [[nodiscard]] FatTreeNode *Right() noexcept { return children[1].get(); }

    [[nodiscard]] const FatTreeNode *Right() const noexcept { return children[1].get(); }

    [[nodiscard]] FatTreeNode *Parent() noexcept { return parent; }

    [[nodiscard]] const FatTreeNode *Parent() const noexcept { return parent; }

    [[nodiscard]] std::span<const shape_t> GetShapes() { return {store.cbegin() + shapeExtents.first, store.cbegin() + shapeExtents.second}; }

    [[nodiscard]] std::span<shape_t> GetSpan() { return {store.begin() + shapeExtents.first, store.begin() + shapeExtents.second}; }

    [[nodiscard]] constexpr std::size_t size() const noexcept { return shapeExtents.second - shapeExtents.first; }

    //[[nodiscard]] constexpr std::size_t TotalShapeCount() const noexcept { return totalShape}

    [[nodiscard]] constexpr bool empty() const noexcept { return shapeExtents.first == shapeExtents.second; }

    [[nodiscard]] static constexpr std::size_t max_size() noexcept { return std::numeric_limits<std::size_t>::max(); }

    void ReorderChildren(MajorAxis axis);

    void CalculateExtents() noexcept;

    void UnsplitSingle() noexcept;

    void UnsplitRecursive() noexcept;

    [[nodiscard]] std::array<std::size_t, 3> GetMajorAxes() const noexcept;
};

template<typename ShapeT = void>
class FatBVHTree : public IntrudableBVHTree<ShapeT>, public ShapeStore {
  public:
    typedef Shape::boundable_shape_t <ShapeT> shape_t;
    typedef std::vector<shape_t> store_t;

    typedef FatTreeNode<ShapeT> node_t;
    typedef const FatTreeNode<ShapeT> const_node_t;
    typedef node_t *node_pointer_t;
    typedef const node_t *const_node_pointer_t;

    FatBVHTree() = delete;

    FatBVHTree(const FatBVHTree &) = delete;

    FatBVHTree(FatBVHTree &&) = delete;

    FatBVHTree(store_t &&s) noexcept
      : shapes(std::forward<store_t>(s))
        , rootNode(std::make_unique<node_t>()) {
        rootNode->shapeExtents = {0, shapes.size()};
        rootNode->CalculateExtents(shapes);
    }

    FatBVHTree(const store_t &s) noexcept
      : shapes(s)
        , rootNode(std::make_unique<node_t>()) {
        rootNode->shapeExtents = {0, shapes.size()};
        rootNode->CalculateExtents(shapes);
    }

    [[nodiscard]] node_pointer_t Root() noexcept { return rootNode.get(); }

    [[nodiscard]] const_node_pointer_t Root() const noexcept { return rootNode.get(); }

    void UnsplitNodes() {
        rootNode->children = {nullptr, nullptr};
        rootNode->shapeExtents = {0, shapes.size()};
        rootNode->CalculateExtents(shapes);
    }

    /// Recursively splits up the root node without recursing on the callstack
    /// \param maxDepth
    /// \param minShapes
    void SplitNodes(std::size_t maxDepth, std::size_t minShapes) {
        std::stack<node_pointer_t> nodesToSplit{};
        nodesToSplit.push(rootNode.get());

        while (!nodesToSplit.empty()) {
            auto current = nodesToSplit.top();
            nodesToSplit.pop();

            if (current->depth >= maxDepth) continue;
            if (current->size() <= minShapes) continue;
            if (current->Left() || current->Right()) std::abort();

            SplitSingleNodeFinal(*current, 0, 0.5);
            bool validSplit = true;
            for (const auto &c: current->children) {
                if (c->size() < minShapes) validSplit = false;
                //add other checks here mayhaps?
            }

            if (!validSplit) {
                current->UnsplitSingle();
            } else {
                for (const auto &c: current->children)
                    nodesToSplit.push(c.get());
            }

        }
    }

    [[nodiscard]] std::optional<Intersection> IntersectImpl(Ray ray, std::size_t &boundChecks, std::size_t &isectChecks) const noexcept override {
        return IntersectTraversable(*this, *Root(), ray, boundChecks, isectChecks);
    }

    [[nodiscard]] std::pair<Point, Point> GetExtents(const node_t &node) const noexcept { return node.extents; }

    [[nodiscard]] std::span<const shape_t> GetShapes(const node_t &node) const noexcept {
        return {
          shapes.cbegin() + node.shapeExtents.first,
          shapes.cbegin() + node.shapeExtents.second,
        };
    }

  protected:
  private:
    std::vector<shape_t> shapes{};
    std::unique_ptr<node_t> rootNode{nullptr};

    /// Splits a node non-recursively using std::partition on the subspan specified by the node of the shapes vector
    /// \param node the node to split, not recursively
    /// \param axis the axis to split on, must be in [0, 2]
    /// \param lhsMargin normalized value specifying the split point in the given axis
    /// \param preferLHS whether to place shapes that fall right in between the two splits to the left hand side node
    void SplitSingleNodeFinal(node_t &node, std::size_t axis, Real lhsMargin, bool preferLHS = true) {
        node.children = {
          std::make_unique<node_t>(),
          std::make_unique<node_t>(),
        };

        auto nodeSpan = node.GetSpan(shapes);

        Real axisExtent = (node.extents.second - node.extents.first)[axis];
        Real splitPoint = Maths::Lerp<Real>(0, axisExtent, lhsMargin);

        auto rhsStart = std::partition(
          nodeSpan.begin(),
          nodeSpan.end(),
          [k = splitPoint + node.extents.first[axis], axis, preferLHS](const shape_t &v) -> bool {
              return preferLHS ?
                     v.center[axis] <= k :
                     v.center[axis] < k;
          });

        node.children[0]->shapeExtents = {node.shapeExtents.first, node.shapeExtents.first + std::distance(nodeSpan.begin(), rhsStart)};
        node.children[1]->shapeExtents = {node.shapeExtents.second - std::distance(rhsStart, nodeSpan.end()) - 1, node.shapeExtents.second};

        for (auto &c: node.children) {
            c->CalculateExtents(shapes);
            c->parent = std::addressof(node);
            c->depth = node.depth + 1;
        }
    }
};

}

///Node implementations
namespace Gfx::BVH::Detail {

template<typename ShapeT>
void FatTreeNode<ShapeT>::CalculateExtents() noexcept {
    extents.first = {+sensibleInf, +sensibleInf, +sensibleInf};
    extents.second = {-sensibleInf, -sensibleInf, -sensibleInf};
    for (auto span = GetSpan(store); const auto &s: span) {
        Shape::Apply(s, [this](const auto &s) {
            extents.first = Maths::Min(extents.first, s.extents.first);
            extents.second = Maths::Max(extents.second, s.extents.second);
        });
    }
    extents.first = extents.first - epsilonPoint;
    extents.second = extents.second + epsilonPoint;
}

template<typename ShapeT>
void FatTreeNode<ShapeT>::ReorderChildren(MajorAxis axis) {
    if (!children[0]) return;

    auto lhs = (children[0]->extents.second - children[0]->extents.first) / static_cast<Real>(2);
    auto rhs = (children[1]->extents.second - children[1]->extents.first) / static_cast<Real>(2);

    if (DecideReorder(axis, lhs, rhs))
        children[0].swap(children[1]);
}

template<typename ShapeT>
void FatTreeNode<ShapeT>::UnsplitSingle() noexcept {
    if (!Left()) return;
    if (Left()->Left()) return;

    extents = {
      Maths::Min(Left()->extents.first, Right()->extents.second),
      Maths::Max(Left()->extents.first, Right()->extents.second),
    };

    shapeExtents = {Left()->shapeExtents.first, Right()->shapeExtents.second};

    children = {nullptr, nullptr};
}

template<typename ShapeT>
void FatTreeNode<ShapeT>::UnsplitRecursive() noexcept {
    if (Left()) {
        Left()->UnsplitRecursive();
        Right()->UnsplitRecursive();
    }

    UnsplitSingle();
}

template<typename ShapeT>
[[nodiscard]] std::array<std::size_t, 3> FatTreeNode<ShapeT>::GetMajorAxes() const noexcept {
    std::array<std::size_t, 3> ret{0, 1, 2};

    std::sort(ret.begin(), ret.end(), [axisLengths = static_cast<Point>(extents.second - extents.first)](std::size_t lhs, std::size_t rhs) {
        const Real lhsLength = axisLengths[lhs];
        const Real rhsLength = axisLengths[rhs];

        return lhsLength > rhsLength;
    });

    return ret;
}

}

///Tree implementations
namespace Gfx::BVH::Detail {}