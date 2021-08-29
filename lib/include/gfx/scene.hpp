#pragma once

#include <vector>

#include "concepts/shape.hpp"
#include "shapes/shapes.hpp"
#include "material.hpp"

namespace Gfx {

class Scene {
  public:
    Scene() = default;

    ~Scene() = default;

    Scene &operator<<(Shape::Shape &&shape) {
        shapes.emplace_back(shape);
        return *this;
    }

    Scene &operator<<(Material &&material) {
        materials.emplace_back(material);
        return *this;
    }

    [[nodiscard]] std::optional<Intersection> Intersect(const Ray &ray) const {
        std::optional<Intersection> intersection{std::nullopt};
        Real closest = std::numeric_limits<Real>::max();

        for (const auto &shape : shapes) {
            std::visit([ray, &closest, &intersection]<Concepts::Shape T>(const T &shape) {
                std::optional<Intersection> res = shape.Intersect(ray);
                if (res && res->distance >= 0. && res->distance < closest) {
                    closest = res->distance;
                    intersection.emplace(std::move(*res));
                }
            }, shape);
        }

        return intersection;
    }

    [[nodiscard]] const Material &GetMaterial(std::size_t idx) const noexcept { return materials[idx]; }

  private:
    std::vector<Shape::Shape> shapes;
    std::vector<Material> materials;
};

}