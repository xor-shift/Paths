#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <gfx/shape/shapes.hpp>

namespace Gfx {

typedef std::pair<Point, Point> bounds_type;

class BuiltBVH;

namespace Impl {

struct ThreadedBVHNode {
    static const size_t nLink = std::numeric_limits<size_t>::max();
    const std::array<size_t, 2> links = {nLink, nLink};
    std::vector<size_t> shapeIndices;
    const bounds_type bounds;
};

}

class BuiltBVH {
  public:
    BuiltBVH(std::vector<Shape::BoundableShape> &&shapeStore, std::array<std::vector<Impl::ThreadedBVHNode>, 6> &&nodeLists)
      : shapeStore(std::move(shapeStore))
        , nodeLists(std::move(nodeLists)) {}

    [[nodiscard]] std::optional<Intersection> Intersect(const Ray &ray) const;

  private:
    std::vector<Shape::BoundableShape> shapeStore{};
    std::array<std::vector<Impl::ThreadedBVHNode>, 6> nodeLists;
};

class BVHBuilder {
  public:
    explicit BVHBuilder(size_t maxObjectsPerNode = 2, size_t maxDepth = 31)
      : maxObjectsPerNode(maxObjectsPerNode)
        , maxDepth(maxDepth) {}

    template<Concepts::Boundable T>
    BVHBuilder &operator<<(T &&shape) {
        shapeStore.emplace_back(std::forward<T>(shape));
        return *this;
    }

    /**
     * Builds a threaded BVH tree, clears out shapeStore (this means that a builder with a given configuration can be used multiple times)
     * @return The built threaded BVH tree
     */
    BuiltBVH Build();

  private:
    std::vector<Shape::BoundableShape> shapeStore{};

    const size_t maxObjectsPerNode = 2;
    const size_t maxDepth = 31;
};

}
