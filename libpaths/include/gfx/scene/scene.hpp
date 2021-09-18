#pragma once

#include <vector>

#include <gfx/shape/shapes.hpp>
#include <gfx/material.hpp>

#include "bvh.hpp"

namespace Gfx {

class Scene {
  public:
    //construct from resources directly
    Scene(std::vector<Material> &&materials, std::vector<BuiltBVH> &&bbvhs, std::vector<Shape::Shape> &&shapes)
      : materials(std::move(materials))
        , bbvhs(std::move(bbvhs))
        , shapes(std::move(shapes)) {}

    Scene() = default;

    ~Scene() = default;

    template<typename T> requires(Concepts::Shape<std::decay_t<T>>/* && !Concepts::Boundable<T>*/)
    Scene & operator<<(T && shape) {
        shapes.emplace_back(std::forward<T>(shape));
        return *this;
    }

    Scene &operator<<(Material &&material) {
        materials.push_back(std::move(material));
        return *this;
    }

    Scene &operator<<(BuiltBVH &&bvh) {
        bbvhs.push_back(std::move(bvh));
        return *this;
    }

    [[nodiscard]] std::optional<Intersection> Intersect(const Ray &ray) const {
        std::optional<Intersection> chosenIntersection{std::nullopt};

        auto Visitor = [&ray, &chosenIntersection]<Concepts::Shape T>(const T &shape) -> void {
            std::optional<Intersection> res = shape.Intersect(ray);
            Intersection::Replace(chosenIntersection, shape.Intersect(ray));
        };

        for (const auto &shape: shapes) std::visit(Visitor, shape);

        for (const auto &bbvh: bbvhs) Intersection::Replace(chosenIntersection, bbvh.Intersect(ray));

        return chosenIntersection;
    }

    [[nodiscard]] const Material &GetMaterial(std::size_t idx) const noexcept { return materials[idx]; }

  private:
    std::vector<Material> materials{};
    std::vector<BuiltBVH> bbvhs{};
    std::vector<Shape::Shape> shapes{};
};

}