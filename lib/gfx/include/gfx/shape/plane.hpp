/**
 * This file contains a Plane shape, described by a center and a normal, extending infinitely
 * UV calculations are not done due to the nature of the shape so IntersectionOptions::calcUV is ineffective (always assumed to be false)
 * The normal is a defining property so IntersectionOptions::calcNormal is ineffective (always assumed to be true)
 */

#pragma once

#include "shape.hpp"

namespace Gfx::Shape {

class Disc;

class Plane {
    friend class Disc;

    const Point center{};
    const Point normal{};

  public:

    constexpr Plane(Point center, Point normal, size_t matIndex)
      : center(center)
        , normal(normal)
        , matIndex(matIndex) {}

    [[nodiscard]] constexpr std::optional<Intersection> Intersect(const Ray &ray) const noexcept {
        LIBGFX_NORMAL_CHECK(ray.direction);
        LIBGFX_NORMAL_CHECK(normal);

        auto t = IntersectImpl(ray);
        if (t < 0) return std::nullopt;

        return Intersection(ray, matIndex, t, normal, {{0, 0}});
    }

    [[nodiscard]] constexpr Real IntersectImpl(const Ray &ray) const noexcept {
        LIBGFX_NORMAL_CHECK(ray.direction);
        LIBGFX_NORMAL_CHECK(normal);

        const auto denom = Math::Dot(normal, ray.direction);

        if (std::abs(denom) <= sensibleEps) return -1;

        return Math::Dot(center - ray.origin, normal) / denom;
    }

    const std::size_t matIndex{0};
};

}
