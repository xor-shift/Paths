#pragma once

#include <gfx/shape/shapes.hpp>

#include <span>
#include <stack>

#include "traversal.hpp"

namespace Gfx::BVH::Detail {

/// The base BVH node type, used mainly for computing other representations but can be used as a store standalone
/// \tparam genericBoundable if true, Shape::BoundableShape is used for the shapes vector
/// \tparam ShapeT if !genericBoundable, this type is used for the shapes vector
template<typename ShapeT = void>
struct FatBVHNode final
  : public IntrudableBVHTree<ShapeT>, public IntrudableBVHNode<ShapeT> {
    typedef Shape::boundable_shape_t<ShapeT> shape_t;

    //Traversable<T> typedefs
    typedef FatBVHNode<ShapeT> node_t;
    typedef const FatBVHNode<ShapeT> const_node_t;
    typedef node_t *node_pointer_t;
    typedef const node_t *const_node_pointer_t;

    std::vector<shape_t> shapes{};
    std::pair<Point, Point> extents{};
    std::array<std::unique_ptr<FatBVHNode>, 2> children{nullptr, nullptr};
    FatBVHNode *parent{nullptr};
    std::size_t id = 0; //for TBVH construction
    std::size_t totalShapeCount = 0; //memoization

    bool Split(std::size_t maxDepth, std::size_t minShapes) noexcept { return SplitImpl(maxDepth, minShapes, 0); }

    [[nodiscard]] node_t &Root() noexcept override { return *this; }

    [[nodiscard]] const_node_t &Root() const noexcept override { return *this; }

    [[nodiscard]] node_pointer_t Left() noexcept override { return children[0].get(); }

    [[nodiscard]] const_node_pointer_t Left() const noexcept override { return children[0].get(); }

    [[nodiscard]] node_pointer_t Right() noexcept override { return children[1].get(); }

    [[nodiscard]] const_node_pointer_t Right() const noexcept override { return children[1].get(); }

    [[nodiscard]] node_pointer_t Parent() noexcept override { return parent; }

    [[nodiscard]] const_node_pointer_t Parent() const noexcept override { return parent; }

    [[nodiscard]] std::size_t size() const noexcept { return totalShapeCount; }

    void ReorderChildren(MajorAxis axis) noexcept {
        if (!children[0]) return;

        auto lhs = (children[0]->extents.second - children[0]->extents.first) / static_cast<Real>(2);
        auto rhs = (children[1]->extents.second - children[1]->extents.first) / static_cast<Real>(2);

        if (DecideReorder(axis, lhs, rhs))
            children[0].swap(children[1]);
    }

    [[nodiscard]] std::optional<Intersection> LinearIntersect(Ray ray) const noexcept {
        return Shape::IntersectLinear(ray, shapes.cbegin(), shapes.cend());
    }

    [[nodiscard]] std::size_t LinearSize() const noexcept { return shapes.size(); }

    [[nodiscard]] std::pair<Point, Point> GetExtents(const node_t &node) const noexcept { return node.extents; }

    [[nodiscard]] std::span<const shape_t> GetShapes(const node_t &node) const noexcept { return {node.shapes.cbegin(), node.shapes.cend()}; }

  protected:
    void SwapChildren() noexcept override { children[0].swap(children[1]); }

    [[nodiscard]] std::size_t GetID() const noexcept override { return id; }

    void SetID(std::size_t id) noexcept override { this->id = id; }

    [[nodiscard]] std::pair<Point, Point> GetExtents() const noexcept override { return extents; }

    [[nodiscard]] std::span<shape_t> GetShapes() noexcept override { return {shapes.begin(), shapes.end()}; }

    [[nodiscard]] std::span<const shape_t> GetShapes() const noexcept override { return {shapes.cbegin(), shapes.cend()}; }

    void Split(std::size_t rhsStartIndex) noexcept override {
        rhsStartIndex = std::max(rhsStartIndex, shapes.size());
        children = {std::make_unique<FatBVHNode>(), std::make_unique<FatBVHNode>()};

        children[0]->shapes.reserve(rhsStartIndex);
        std::copy(shapes.cbegin(), shapes.cbegin() + rhsStartIndex, std::back_inserter(children[0]->shapes));
        children[1]->shapes.reserve(shapes.size() - rhsStartIndex);
        std::copy(shapes.cbegin() + rhsStartIndex, shapes.cend(), std::back_inserter(children[1]->shapes));

        for (auto &c: children) {
            c->parent = this;
            c->CalculateExtents();
        }
    }

    void UnsplitOnce() noexcept override {
        shapes.reserve(children[0]->shapes.size() + children[1]->shapes.size());
        for (const auto &c: children)
            std::copy(c->shapes.cbegin(), c->shapes.cbegin(), std::back_inserter(shapes));
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
};

}

namespace Gfx::BVH {

template<typename ShapeT, typename From>
std::vector<Shape::boundable_shape_t<ShapeT>> ConvertShapesVector(const From &store) {
    std::vector<Gfx::Shape::boundable_shape_t<ShapeT>> extracted;

    for (const auto &s: store) {
        Gfx::Shape::Apply(s, [&extracted](auto &&s) {
            using T = std::decay_t<decltype(s)>;

            if constexpr (std::is_same_v<ShapeT, void>) {
                if constexpr (Gfx::Concepts::Boundable<T>) extracted.emplace_back(s);
            } else {
                if constexpr (std::is_same_v<ShapeT, T>) extracted.emplace_back(s);
            }
        });
    }

    return extracted;
}

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
