#pragma once

#include "traversal.hpp"

namespace Gfx::BVH::Detail {

template<typename ShapeT>
class BVHNode final : public IntrudableBVHNode<ShapeT> {
  public:
    typedef Shape::boundable_shape_t <ShapeT> shape_t;

    BVHNode() noexcept = default;

    ~BVHNode() noexcept override = default;

    explicit BVHNode(std::shared_ptr<std::vector<shape_t>> shapes, std::pair<std::size_t, std::size_t> sExtents = {0, 0})
      : shapesImpl(std::move(shapes))
        , shapeExtents(sExtents) { if (!this->GetShapes().empty()) this->CalculateExtents(); }

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

    [[nodiscard]] std::span<shape_t> GetShapes() noexcept override {
        return {shapesImpl->begin() + shapeExtents.first,
                shapesImpl->begin() + shapeExtents.second};
    }

    [[nodiscard]] std::span<const shape_t> GetShapes() const noexcept override {
        return {shapesImpl->begin() + shapeExtents.first,
                shapesImpl->begin() + shapeExtents.second};
    }

    [[nodiscard]] std::size_t TotalShapeCount() const noexcept override { return totalShapeCount; }

  protected:
    void SplitAt(std::size_t rhsStartIndex) noexcept override {
        auto shapes = this->GetShapes();
        rhsStartIndex = std::min(rhsStartIndex, shapes.size());
        children = {std::make_unique<BVHNode>(shapesImpl), std::make_unique<BVHNode>(shapesImpl)};

        children[0]->shapeExtents = {shapeExtents.first, shapeExtents.first + rhsStartIndex};
        children[1]->shapeExtents = {shapeExtents.first + rhsStartIndex, shapeExtents.second};

        for (auto &c: children) {
            c->totalShapeCount = c->GetShapes().size();
            c->parent = this;
            c->CalculateExtents();
        }

        shapeExtents = {0, 0};
    }

    void UnsplitOnce() noexcept override {
        shapeExtents = {
          children[0]->shapeExtents.first,
          children[1]->shapeExtents.second,
        };

        children = {nullptr, nullptr};
    }

    void SetExtents(std::pair<Point, Point> e) noexcept override { extents = e; }

    std::shared_ptr<std::vector<shape_t>> shapesImpl{nullptr};
    std::pair<std::size_t, std::size_t> shapeExtents{0, 0};
    std::pair<Point, Point> extents{};
    std::array<std::unique_ptr<BVHNode>, 2> children{nullptr, nullptr};
    BVHNode *parent{nullptr};
    std::size_t id = 0;
    std::size_t totalShapeCount = 0;
};

template<typename ShapeT = void>
class BVHTree : public IntrudableBVHTree<ShapeT> {
  public:
    typedef BVHNode<ShapeT> node_t;
    typedef Shape::boundable_shape_t <ShapeT> shape_t;

    explicit BVHTree(std::vector<shape_t> &&vec)
      : shapes(std::make_unique<std::vector<shape_t>>(std::move(vec)))
        , root(std::make_unique<node_t>(shapes, std::make_pair<std::size_t, std::size_t>(0, shapes->size()))) {
        //NewFatBVHNode<ShapeT> asd(shapes, {0, shapes->size()});
    }

    [[nodiscard]] node_t &Root() noexcept override { return *root; }

    [[nodiscard]] const node_t &Root() const noexcept override { return *root; }

  private:
    std::shared_ptr<std::vector<shape_t>> shapes;
    std::unique_ptr<node_t> root{nullptr};
};

}
