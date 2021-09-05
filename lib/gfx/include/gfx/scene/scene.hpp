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
        std::visit([this] < Concepts::Shape
        T > (T && s)
        {
            if constexpr (Concepts::Boundable<T>) {
                boundableShapes.emplace_back(s);
            } else {
                unboundableShapes.emplace_back(s);
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

        for (const auto &shape : boundableShapes) std::visit(Visitor, shape);
        for (const auto &shape : unboundableShapes) std::visit(Visitor, shape);

        return intersection;
    }

    [[nodiscard]] const Material &GetMaterial(std::size_t idx) const noexcept { return materials[idx]; }

  private:
    std::vector<Shape::Shape> unboundableShapes;
    std::vector<Shape::Shape> boundableShapes;
    std::vector<Material> materials;
};

}