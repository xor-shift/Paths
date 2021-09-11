#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <gfx/shape/shapes.hpp>

namespace Gfx {

class BVH;

namespace Impl {

class BVHNode {
  public:
    typedef std::pair<Point, Point> bounds_type;

    BVHNode(BVH &base, size_t depth, size_t maxDepth, size_t maxShapesPerNode, bounds_type extents = {
      {{-sensibleInf, -sensibleInf, -sensibleInf}},
      {{sensibleInf,  sensibleInf,  sensibleInf}}
    });

    void Insert(size_t shapeIndex) { shapeIndices.push_back(shapeIndex); }

    void Fit();

    void Split();

    [[nodiscard]] std::optional<Intersection> Intersect(const Ray &ray) const;

  private:
    BVH &base;
    const size_t depth;
    const size_t maxDepth;
    const size_t maxShapesPerNode;

    bounds_type extents;
    std::vector<size_t> shapeIndices{};

    std::array<std::unique_ptr<BVHNode>, 2> childNodes{nullptr, nullptr};
};

}

class BVH {
  public:
    BVH(size_t maxDepth, size_t maxShapesPerNode);

    BVH(const BVH &) = delete;

    BVH(BVH &&) = delete;

    template<Concepts::Boundable T>
    BVH &operator<<(T &&shape) {
        shapeStore.emplace_back(std::forward<T>(shape));
        return *this;
    }

    [[nodiscard]] const Shape::BoundableShape &ShapeAtIdx(size_t index) { return shapeStore.at(index); }

    [[nodiscard]] std::optional<Intersection> Intersect(const Ray &ray) const;

    void Finalize();

  private:
    std::vector<Shape::BoundableShape> shapeStore{};

    Impl::BVHNode rootNode;
};

}
