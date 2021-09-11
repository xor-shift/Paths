#include <gfx/scene/bvh.hpp>

namespace Gfx {

namespace Impl {

template<typename C>
std::optional<Intersection> CheckIntersectThrough(const C &container, const Ray &ray, const auto transform = [](const auto &v) { return v; }) noexcept {
    std::optional<Intersection> intersection{std::nullopt};
    Real closest = std::numeric_limits<Real>::max();

    auto Visitor = [ray, &closest, &intersection]<Concepts::Shape T>(const T &shape) -> void {
        std::optional<Intersection> res = shape.Intersect(ray);
        if (res && res->distance >= 0. && res->distance < closest) {
            closest = res->distance;
            intersection.emplace(std::move(*res));
        }
    };

    for (const auto &shape : container) std::visit(Visitor, transform(shape));

    return intersection;
}

BVHNode::BVHNode(BVH &base, size_t depth, size_t maxDepth, size_t maxShapesPerNode, bounds_type extents)
  : base(base)
    , depth(depth)
    , maxDepth(maxDepth)
    , maxShapesPerNode(maxShapesPerNode)
    , extents(std::move(extents)) {}

void BVHNode::Fit() {
    extents = {
      {{sensibleInf,  sensibleInf,  sensibleInf}},
      {{-sensibleInf, -sensibleInf, -sensibleInf}},
    };

    for (auto shapeIndex : shapeIndices) {
        const auto &shapeExtents = std::visit([](const auto &s) {
            return s.extents;
        }, base.ShapeAtIdx(shapeIndex));

        extents.first = Math::Min(extents.first, shapeExtents.first);
        extents.second = Math::Max(extents.second, shapeExtents.second);
    }

    extents.first -= EpsilonVector;
    extents.second += EpsilonVector;

#ifndef NDEBUG
    for (auto shapeIndex : shapeIndices) {
        assert(std::visit([this](const auto &s) {
            return Shape::InBounds(extents, s.extents);
        }, base.ShapeAtIdx(shapeIndex)));
    }
#endif
}

void BVHNode::Split() {
#ifndef NDEBUG
    assert(childNodes[0] == nullptr);
    assert(childNodes[1] == nullptr);
#endif

    Fit();

    if (depth >= maxDepth || shapeIndices.size() <= maxShapesPerNode) return;

    std::array<size_t, 3> splitOrder{0, 1, 2};

    Point axisLengths(extents.second - extents.first);

    std::sort(splitOrder.begin(), splitOrder.end(), [&axisLengths](const std::size_t &lhs, const std::size_t &rhs) -> bool {
        return axisLengths[lhs] > axisLengths[rhs];
    });

    for (const size_t axis : splitOrder) {
        bounds_type leftBounds = extents;
        bounds_type rightBounds = extents;

        leftBounds.second[axis] -= axisLengths[axis] / 2.;
        rightBounds.first[axis] += axisLengths[axis] / 2.;

        leftBounds.first -= EpsilonVector;
        rightBounds.first -= EpsilonVector;
        leftBounds.second += EpsilonVector;
        rightBounds.second += EpsilonVector;

        childNodes[0] = std::make_unique<BVHNode>(std::ref(base), depth + 1, maxDepth, maxShapesPerNode, std::move(leftBounds));
        childNodes[1] = std::make_unique<BVHNode>(std::ref(base), depth + 1, maxDepth, maxShapesPerNode, std::move(rightBounds));

        for (const auto index : shapeIndices) {
            if (const auto &extentsAtIndex = std::visit(
                  [](const auto &s) {
                      return s.extents;
                  }, base.ShapeAtIdx(index)); Shape::InBounds(childNodes[0]->extents, extentsAtIndex)) {
                childNodes[0]->Insert(index);
            } else {
                childNodes[1]->Insert(index);
            }
        }

        if (childNodes[0]->shapeIndices.empty() || childNodes[1]->shapeIndices.empty()) {
            childNodes = {nullptr, nullptr};
            //try another axis now
        } else {
            shapeIndices.clear();
            childNodes[0]->Fit();
            childNodes[1]->Fit();
            break;
        }
    }

    if (childNodes[0] != nullptr) {
        childNodes[0]->Split();
        childNodes[1]->Split();
    }
}

std::optional<Intersection> BVHNode::Intersect(const Ray &ray) const {
    if (!Shape::AABox::EIntersects(extents, ray)) return std::nullopt;
    //auto selfIntersect = CheckIntersectThrough(shapeIndices, ray, [this](const size_t i) { return base.ShapeAtIdx(i); });

    std::optional<Intersection> chosenIntersection = std::nullopt;

    auto ReplaceIsect = [&chosenIntersection](std::optional<Intersection> &&newIsect) {
        if (!newIsect || newIsect->distance < 0) return;
        if (!chosenIntersection || chosenIntersection->distance > newIsect->distance) chosenIntersection = std::move(newIsect);
    };

    ReplaceIsect(CheckIntersectThrough(shapeIndices, ray, [this](const size_t i) { return base.ShapeAtIdx(i); }));
    if (childNodes[0] != nullptr) {
        ReplaceIsect(childNodes[0]->Intersect(ray));
        ReplaceIsect(childNodes[1]->Intersect(ray));
    }

    return chosenIntersection;
}

}

BVH::BVH(size_t maxDepth, size_t maxShapesPerNode)
  : rootNode(*this, 0, maxDepth, maxShapesPerNode) {}

[[nodiscard]] std::optional<Intersection> BVH::Intersect(const Ray &ray) const {
    return rootNode.Intersect(ray);
}

void BVH::Finalize() {
    for (size_t i = 0; i < shapeStore.size(); i++) rootNode.Insert(i);

    rootNode.Split();
}

}
