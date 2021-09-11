#pragma once

#include <vector>

#include <gfx/shape/shapes.hpp>
#include <gfx/material.hpp>

#include "bvh.hpp"

namespace Gfx {

class Scene {
  public:
    Scene() = default;

    ~Scene() = default;

    Scene &operator<<(Shape::Shape &&shape) {
        //std::visit([this]<Concepts::Shape T> (T &&s) {
        std::visit([this](auto &&s)
        requires Concepts::Shape<std::decay_t<decltype(s)>>
        {
            typedef std::decay_t<decltype(s)> T; //i blame clion formatting

            if constexpr (Concepts::Boundable<T>) {
                //boundableShapes.emplace_back(s);
                bvh << std::forward<T>(s);
            } else {
                unboundableShapes.emplace_back(std::forward<T>(s));
            }
        }, std::forward<Shape::Shape>(shape));
        return *this;
    }

    Scene &operator<<(Material &&material) {
        materials.emplace_back(material);
        return *this;
    }

    [[nodiscard]] std::optional<Intersection> Intersect(const Ray &ray) const {
        std::optional<Intersection> intersection{std::nullopt};
        Real closest = std::numeric_limits<Real>::max();

        auto Visitor = [ray, &closest, &intersection]<Concepts::Shape T>(const T &shape) -> void {
            std::optional<Intersection> res = shape.Intersect(ray);
            if (res && res->distance >= 0. && res->distance < closest) {
                closest = res->distance;
                intersection.emplace(std::move(*res));
            }
        };

        for (const auto &shape : unboundableShapes) std::visit(Visitor, shape);

        if (auto bvhRes = bvh.Intersect(ray); bvhRes && bvhRes->distance >= 0 && bvhRes->distance < closest) {
            closest = bvhRes->distance;
            intersection.emplace(std::move(*bvhRes));
        }

        return intersection;
    }

    void Finalize() { bvh.Finalize(); }

    [[nodiscard]] const Material &GetMaterial(std::size_t idx) const noexcept { return materials[idx]; }

  private:
    std::vector<Shape::Shape> unboundableShapes;
    std::vector<Shape::BoundableShape> boundableShapes;
    std::vector<Material> materials;

    BVH bvh{31, 2};
};

}