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

struct TreeNode {
    TreeNode(size_t depth, size_t maxDepth, size_t maxObjects, bounds_type extents = {
      {{-sensibleInf, -sensibleInf, -sensibleInf}},
      {{sensibleInf,  sensibleInf,  sensibleInf}}
    })
      : extents(std::move(extents)), depth(depth), maxDepth(maxDepth), maxObjects(maxObjects) {}

    void Fit(const std::vector<Shape::BoundableShape> &store);

    void Split(const std::vector<Shape::BoundableShape> &store);

    template<size_t direction>
    void Order() {
        if (childNodes[0] == nullptr) return;

        std::array<Point, 2> centers = {
          (childNodes[0]->extents.first + childNodes[0]->extents.second) / 2.,
          (childNodes[1]->extents.first + childNodes[1]->extents.second) / 2.,
        };

        bool swap = false;

        if constexpr (direction == 0) swap = centers[0][0] >= centers[1][0];
        else if constexpr (direction == 1) swap = centers[0][0] <= centers[1][0];
        else if constexpr (direction == 2) swap = centers[0][1] >= centers[1][1];
        else if constexpr (direction == 3) swap = centers[0][1] <= centers[1][1];
        else if constexpr (direction == 4) swap = centers[0][2] >= centers[1][2];
        else if constexpr (direction == 5) swap = centers[0][2] <= centers[1][2];
        else static_assert(direction != direction);

        if (swap) std::swap(childNodes[0], childNodes[1]);

        for (auto &child : childNodes) child->Order<direction>();
    }

    [[nodiscard]] bool Leaf() { return childNodes[0] == nullptr; }

    void BuildLinks(TreeNode *right = nullptr);

    //regular tree stuff
    bounds_type extents;
    std::vector<size_t> shapeIndices{};

    std::array<std::unique_ptr<TreeNode>, 2> childNodes{nullptr, nullptr};

    //thread specific stuff
    size_t selfID = 0;
    //+x, -x, +y, -y, +z, -z
    std::array<TreeNode *, 2> links;

    //const stuff
    const size_t depth;
    const size_t maxDepth;
    const size_t maxObjects;
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
    BVHBuilder(size_t maxObjectsPerNode = 2, size_t maxDepth = 31)
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
