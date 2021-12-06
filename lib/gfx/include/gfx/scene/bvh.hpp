#pragma once

#include <gfx/shape/shapes.hpp>

#include <span>
#include <stack>

#include "traversal.hpp"

namespace Gfx::BVH::Detail {

template<typename ShapeT = void>
class FatBVHNode final : public IntrudableBVHNode<ShapeT> {
  public:
    typedef Shape::boundable_shape_t<ShapeT> shape_t;

    ~FatBVHNode() noexcept override = default;

    [[nodiscard]] Traversable *Left() noexcept override { return children[0].get(); }

    [[nodiscard]] const Traversable *Left() const noexcept override { return children[0].get(); }

    [[nodiscard]] Traversable *Right() noexcept override { return children[1].get(); }

    [[nodiscard]] const Traversable *Right() const noexcept override { return children[1].get(); }

    [[nodiscard]] Traversable *Parent() noexcept override { return parent; }

    [[nodiscard]] const Traversable *Parent() const noexcept override { return parent; }

    void SwapChildren() noexcept override { children[0].swap(children[1]); }

    [[nodiscard]] std::size_t GetID() const noexcept override { return id; }

    void SetID(std::size_t i) noexcept override { id = i; }

    [[nodiscard]] std::pair<Point, Point> GetExtents() const noexcept override { return extents; }

    [[nodiscard]] std::span<shape_t> GetShapes() noexcept override { return {shapes.begin(), shapes.end()}; }

    [[nodiscard]] std::span<const shape_t> GetShapes() const noexcept override { return {shapes.cbegin(), shapes.cend()}; }

    [[nodiscard]] std::size_t TotalShapeCount() const noexcept override { return totalShapeCount; }

    std::vector<shape_t> shapes{};
    std::pair<Point, Point> extents{};
    std::array<std::unique_ptr<FatBVHNode>, 2> children{nullptr, nullptr};
    FatBVHNode *parent{nullptr};
    std::size_t id = 0;
    std::size_t totalShapeCount = 0;

  protected:
    void SplitAt(std::size_t rhsStartIndex) noexcept override {
        rhsStartIndex = std::min(rhsStartIndex, shapes.size());
        children = {std::make_unique<FatBVHNode>(), std::make_unique<FatBVHNode>()};

        children[0]->shapes.reserve(rhsStartIndex);
        std::copy(shapes.cbegin(), shapes.cbegin() + rhsStartIndex, std::back_inserter(children[0]->shapes));
        children[1]->shapes.reserve(shapes.size() - rhsStartIndex);
        std::copy(shapes.cbegin() + rhsStartIndex, shapes.cend(), std::back_inserter(children[1]->shapes));

        for (auto &c: children) {
            c->totalShapeCount = c->shapes.size();
            c->parent = this;
            c->CalculateExtents();
        }

        shapes.clear();
    }

    void UnsplitOnce() noexcept override {
        shapes.reserve(children[0]->shapes.size() + children[1]->shapes.size());
        for (const auto &c: children)
            std::copy(c->shapes.cbegin(), c->shapes.cbegin(), std::back_inserter(shapes));

        children = {nullptr, nullptr};
    }

    void SetExtents(std::pair<Point, Point> e) noexcept override { extents = e; }
};

template<typename ShapeT = void>
class FatBVHTree final : public IntrudableBVHTree<ShapeT> {
  public:
    typedef FatBVHNode<ShapeT> node_t;
    typedef Shape::boundable_shape_t <ShapeT> shape_t;

    explicit FatBVHTree(std::vector<shape_t> &&vec)
    : root(std::make_unique<node_t>()) {
        root->totalShapeCount = vec.size();
        root->shapes = std::move(vec);
    }

    [[nodiscard]] node_t &Root() noexcept override { return *root; }

    [[nodiscard]] const node_t &Root() const noexcept override { return *root; }

  private:
    std::unique_ptr<node_t> root{nullptr};
};

}

namespace Gfx::BVH {

template<typename ShapeT = void>
std::shared_ptr<Detail::FatBVHNode<ShapeT>> LinearToFat(const Gfx::LinearShapeStore<ShapeT> &store, std::size_t maxDepth = 7, std::size_t minShapes = 4) {
    auto ret = std::make_shared<Detail::FatBVHNode<ShapeT>>();

    auto conv = Shape::ConvertShapesVector<ShapeT>(store.shapes);
    ret->shapes = std::move(conv);

    /*if constexpr (Concepts::Boundable<ShapeT>) {
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
    }*/

    ret->totalShapeCount = ret->shapes.size();
    ret->Split(maxDepth, minShapes);

    return ret;
}

}
