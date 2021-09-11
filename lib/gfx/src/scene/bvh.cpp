#include <gfx/scene/bvh.hpp>

#include <list>

namespace Gfx {

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


namespace Impl {

void TreeNode::Fit(const std::vector<Shape::BoundableShape> &store) {
    extents = {
      {{sensibleInf,  sensibleInf,  sensibleInf}},
      {{-sensibleInf, -sensibleInf, -sensibleInf}},
    };

    for (auto shapeIndex : shapeIndices) {
        const auto &shapeExtents = std::visit([](const auto &s) {
            return s.extents;
        }, store[shapeIndex]);

        extents.first = Math::Min(extents.first, shapeExtents.first);
        extents.second = Math::Max(extents.second, shapeExtents.second);
    }

    extents.first -= EpsilonVector;
    extents.second += EpsilonVector;

#ifndef NDEBUG
    for (auto shapeIndex : shapeIndices) {
        assert(std::visit([this](const auto &s) {
            return Shape::InBounds(extents, s.extents);
        }, store[shapeIndex]));
    }
#endif

}

void TreeNode::Split(const std::vector<Shape::BoundableShape> &store) {
#ifndef NDEBUG
    assert(childNodes[0] == nullptr);
    assert(childNodes[1] == nullptr);
#endif

    Fit(store);

    if (depth >= maxDepth || shapeIndices.size() <= maxObjects) return;

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

        childNodes[0] = std::make_unique<TreeNode>(depth + 1, maxDepth, maxObjects, std::move(leftBounds));
        childNodes[1] = std::make_unique<TreeNode>(depth + 1, maxDepth, maxObjects, std::move(rightBounds));

        for (const auto index : shapeIndices) {
            if (const auto &extentsAtIndex = std::visit(
                  [](const auto &s) {
                      return s.extents;
                  }, store[index]); Shape::InBounds(childNodes[0]->extents, extentsAtIndex)) {
                childNodes[0]->shapeIndices.push_back(index);
            } else {
                childNodes[1]->shapeIndices.push_back(index);
            }
        }

        if (childNodes[0]->shapeIndices.empty() || childNodes[1]->shapeIndices.empty()) {
            childNodes = {nullptr, nullptr};
            //try another axis now
        } else {
            shapeIndices.clear();
            childNodes[0]->Fit(store);
            childNodes[1]->Fit(store);
            break;
        }
    }

    if (childNodes[0] != nullptr) {
        childNodes[0]->Split(store);
        childNodes[1]->Split(store);
    }
}

void TreeNode::BuildLinks(TreeNode *right) {
    if (!Leaf()) {
        links[0] = childNodes[0].get();
        links[1] = right;

        childNodes[0]->BuildLinks(childNodes[1].get());
        childNodes[1]->BuildLinks(right);
    } else {
        for (size_t i = 0; i < 6; i++) {
            links = {right, right};
        }
    }
}

}

BuiltBVH BVHBuilder::Build() {
    Impl::TreeNode node(0, maxDepth, maxObjectsPerNode);

    node.shapeIndices.reserve(shapeStore.size());
    for (size_t i = 0; i < shapeStore.size(); i++)
        node.shapeIndices.push_back(i);

    node.Split(shapeStore);

    auto BFSTraverse = [&](auto &&cb) {
        std::list<Impl::TreeNode *> queue{&node};

        while (!queue.empty()) {
            auto *current = queue.front();
            queue.pop_front();

            std::invoke(cb, current);

            if (current->childNodes[0] != nullptr) {
                queue.push_back(current->childNodes[0].get());
                queue.push_back(current->childNodes[1].get());
            }
        }
    };

    auto BuildFor = [&]<size_t direction>() {
        node.Order<direction>();
        BFSTraverse([i = 0](Impl::TreeNode *node) mutable {
            node->selfID = i++;
        });
        node.BuildLinks();

        std::vector<Impl::ThreadedBVHNode> threadedNodes;
        BFSTraverse([&](Impl::TreeNode *node) {
            std::array<size_t, 2> links = {Impl::ThreadedBVHNode::nLink, Impl::ThreadedBVHNode::nLink};

            for (size_t i = 0; i < 2; i++) {
                auto *ptr = node->links[i];
                links[i] = (ptr == nullptr) ? Impl::ThreadedBVHNode::nLink : ptr->selfID;
            }

            threadedNodes.push_back(Impl::ThreadedBVHNode{
                .links{links},
                .shapeIndices{node->shapeIndices},
                .bounds{node->extents},
                });
        });

        return threadedNodes;
    };

    auto BuildFull = [&]() {
        std::array<std::vector<Impl::ThreadedBVHNode>, 6> nodeLists{};

        nodeLists[0] = BuildFor.operator()<0>();
        nodeLists[1] = BuildFor.operator()<1>();
        nodeLists[2] = BuildFor.operator()<2>();
        nodeLists[3] = BuildFor.operator()<3>();
        nodeLists[4] = BuildFor.operator()<4>();
        nodeLists[5] = BuildFor.operator()<5>();

        return nodeLists;
    };

    return {std::move(shapeStore), BuildFull()};
}

std::optional<Intersection> BuiltBVH::Intersect(const Ray &ray) const {
    std::optional<Intersection> chosenIntersection{std::nullopt};

    const auto &nodes = nodeLists[ray.majorDirection];

    for (size_t idx = 0; idx != Impl::ThreadedBVHNode::nLink;) {
        auto &node = nodes[idx];

        if (Shape::AABox::EIntersects(node.bounds, ray)) {
            if (!node.shapeIndices.empty()) {
                Intersection::Replace(chosenIntersection, CheckIntersectThrough(node.shapeIndices, ray, [&](size_t idx) {
                    return shapeStore[idx];
                }));
            }

            idx = node.links[0];
        } else {
            idx = node.links[1];
        }
    }

    return chosenIntersection;
}

}
