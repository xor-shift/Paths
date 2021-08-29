/**
 * This file contains a Plane shape, described by a center and a normal, extending infinitely
 * UV calculations are not done due to the nature of the shape so IntersectionOptions::calcUV is ineffective (always assumed to be false)
 * The normal is a defining property so IntersectionOptions::calcNormal is ineffective (always assumed to be true)
 */

#pragma once

#include <gfx/concepts/shape.hpp>

namespace Gfx::Shape {

struct Plane {
    [[nodiscard]] constexpr std::optional<Intersection> Intersect(const Ray &ray) const noexcept {
        LIBGFX_NORMAL_CHECK(ray.direction);
        LIBGFX_NORMAL_CHECK(normal);

        auto denom = normal.Dot(ray.direction);

        if (std::abs(denom) <= Epsilon) return std::nullopt;

        return Intersection{
            .theRay = ray,
            .distance = (center - ray.origin).Dot(normal) / denom,
            .normal = normal,
            .matIndex = matIndex,
            .uv = {0, 0},
        };
    }

    Point center{};
    Point normal{};

    std::size_t matIndex{0};
    constexpr static bool boundable = false;
};

}
