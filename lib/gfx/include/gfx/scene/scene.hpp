#pragma once

#include <gfx/common.hpp>
#include <gfx/ray.hpp>
#include <gfx/shape/shapes.hpp>
#include "gfx/material/material.hpp"

#include "store.hpp"

#include <vector>

namespace Gfx {

class Scene final : public ShapeStore {
  public:
    std::size_t InsertMaterial(Material mat, const std::string &alias = "") noexcept {
        materials.push_back(mat);
        const std::size_t index = materials.size() - 1;
        if (!alias.empty()) materialAliases[alias] = index;
        return index;
    }

    std::size_t ResolveMaterial(const std::string &alias) const noexcept {
        if (!materialAliases.contains(alias)) return materials.size() - 1;
        else return materialAliases.at(alias);
    }

    void InsertStore(std::shared_ptr<ShapeStore> store) noexcept {
        stores.push_back(std::move(store));
    }

    [[nodiscard]] Material GetMaterial(std::size_t i) const noexcept {
        if (materials.empty()) std::abort();
        return materials[std::clamp<std::size_t>(i, 0, materials.size() - 1)];
    }

  protected:
    [[nodiscard]] std::optional<Intersection> IntersectImpl(Ray ray, std::size_t &boundChecks, std::size_t &shapeChecks) const noexcept override {
        std::optional<Intersection> bestIntersection = std::nullopt;

        for (const auto &store: stores)
            Intersection::Replace(bestIntersection, store->Intersect(ray, boundChecks, shapeChecks));

        return bestIntersection;
    }

  private:
    std::vector<std::shared_ptr<ShapeStore>> stores{};
    std::vector<Material> materials{};
    std::unordered_map<std::string, std::size_t> materialAliases{};
};

}