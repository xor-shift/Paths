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
  public:
    constexpr Plane(size_t matIndex, Point center, Point normal)
      : matIndex(matIndex)
        , center(center)
        , normal(Maths::Normalized(normal)) {}

    [[nodiscard]] constexpr std::optional<Intersection> Intersect(const Ray &ray) const noexcept {
        LIBGFX_NORMAL_CHECK(ray.direction);
        LIBGFX_NORMAL_CHECK(normal);

        if (auto t = IntersectImpl(ray); t < sensibleEps) return std::nullopt;
        else return Intersection(ray, matIndex, t, normal, {0, 0});
    }

    [[nodiscard]] constexpr Real IntersectImpl(const Ray &ray) const noexcept {
        LIBGFX_NORMAL_CHECK(ray.direction);
        LIBGFX_NORMAL_CHECK(normal);

        const auto denom = Maths::Dot(normal, ray.direction);

        if (std::abs(denom) <= sensibleEps) return -1;

        return Maths::Dot(center - ray.origin, normal) / denom;
    }


  private:
    friend class Disc;

    size_t matIndex{0};
    Point center{};
    Point normal{};
};

}
