#pragma once

#include "Paths/Common.hpp"
#include "Paths/Material/material.hpp"
#include "Paths/Ray.hpp"
#include "Paths/Shape/Shapes.hpp"

#include "Store.hpp"

#include <vector>

namespace Paths {

class Scene final : public ShapeStore {
public:
    std::size_t insert_material(Material mat, const std::string &alias = "") noexcept {
        m_materials.push_back(mat);
        const std::size_t index = m_materials.size() - 1;
        if (!alias.empty())
            m_material_aliases[alias] = index;
        return index;
    }

    std::size_t resolve_material(const std::string &alias) const noexcept {
        if (!m_material_aliases.contains(alias))
            return m_materials.size() - 1;
        else
            return m_material_aliases.at(alias);
    }

    void insert_store(std::shared_ptr<ShapeStore> store) noexcept { m_stores.push_back(std::move(store)); }

    [[nodiscard]] Material get_material(std::size_t i) const noexcept {
        if (m_materials.empty())
            std::abort();
        return m_materials[std::clamp<std::size_t>(i, 0, m_materials.size() - 1)];
    }

protected:
    [[nodiscard]] std::optional<Intersection> intersect_impl(
        Ray ray, std::size_t &bound_checks, std::size_t &shape_checks) const noexcept override {
        std::optional<Intersection> best_intersection = std::nullopt;

        for (const auto &store : m_stores)
            Intersection::replace(best_intersection, store->intersect_ray(ray, bound_checks, shape_checks));

        return best_intersection;
    }

private:
    std::vector<std::shared_ptr<ShapeStore>> m_stores {};
    std::vector<Material> m_materials {};
    std::unordered_map<std::string, std::size_t> m_material_aliases {};
};

}