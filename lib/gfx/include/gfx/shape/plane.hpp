/**
 * This file contains a Plane shape, described by a center and a normal, extending infinitely
 * UV calculations are not done due to the nature of the shape so IntersectionOptions::calcUV is ineffective (always assumed to be false)
 * The normal is a defining property so IntersectionOptions::calcNormal is ineffective (always assumed to be true)
 */

#pragma once

#include "shape.hpp"

namespace Gfx::Shape {

struct Plane {
    /**
     * Does not calculate UV coordinates (by necessity)
     * @param ray
     * @return
     */
    [[nodiscard]] constexpr std::optional<Intersection> Intersect(const Ray &ray) const noexcept {
        LIBGFX_NORMAL_CHECK(ray.direction);
        LIBGFX_NORMAL_CHECK(normal);

        const auto denom = Math::Dot(normal, ray.direction);

        if (std::abs(denom) <= sensibleEps) return std::nullopt;

        return Intersection(ray, matIndex, Math::Dot(center - ray.origin, normal) / denom, normal, {{0, 0}});
    }

    const Point center{};
    const Point normal{};

    const std::size_t matIndex{0};
};

}
