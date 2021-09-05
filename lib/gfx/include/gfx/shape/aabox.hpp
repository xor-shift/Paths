/**
 * This file contains a AABox shape, described by two points in space, representing an axis aligned box
 * The main purpose of this shape is use in BVHs
 */

#pragma once

#include <chrono>

#include "shape.hpp"

namespace Gfx::Shape {

struct AABox {
  private:
    template<bool getDistance>
    using IntersectionType = std::conditional_t<getDistance, std::pair<bool, Real>, bool>;

    template<bool getDistance>
    [[nodiscard]] static constexpr IntersectionType<getDistance> Func(Real tMin, Real tMax) noexcept {
        const bool isect = tMax > std::max<Real>(tMin, 0.);

        if constexpr (getDistance) {
            return std::pair<bool, Real>{isect, tMin > 0. ? tMin : tMax};
        } else {
            return isect;
        }
    }

  public:
    constexpr AABox(Point min, Point max, size_t matIndex) noexcept
      : extents(min, max)
        , center((min + max) / 2.)
        , matIndex(matIndex) {}

    template<bool getDistance = false>
    [[nodiscard]] constexpr auto Intersects(const Ray &ray) const noexcept {
        Real
          tMin = std::numeric_limits<Real>::min(),
          tMax = std::numeric_limits<Real>::max();

        for (size_t i = 0; i < 3; ++i) {
            const auto t1 = (extents.first[i] - ray.origin[i]) * ray.directionReciprocals[i];
            const auto t2 = (extents.second[i] - ray.origin[i]) * ray.directionReciprocals[i];

            tMin = std::max(tMin, std::min(std::min(t1, t2), tMax));
            tMax = std::min(tMax, std::max(std::max(t1, t2), tMin));
        }

        return Func<getDistance>(tMin, tMax);
    }

    [[nodiscard]] constexpr std::optional<Intersection> Intersect(const Ray &ray) const noexcept {
        if (const auto &[isect, dist] = Intersects<true>(ray); isect) {
            auto isection = Intersection{
              .theRay = ray,
              .distance = dist,
              .normal = Point{0, 0, 0},
              .matIndex = matIndex,
              .uv = {0, 0},
            };

            if (bool calcNormal = true; calcNormal) {
                isection.ComputeIntersectionPoint();

                auto p = isection.intersectionPoint - (extents.first + extents.second) * .5;
                auto d = (extents.second - extents.first) * .5;
                Real bias = 1.00000001;

                isection.normal = Math::Ops::Vector::Normalized(Point{
                  std::floor(p[0] / d[0] * bias),
                  std::floor(p[1] / d[1] * bias),
                  std::floor(p[2] / d[2] * bias),
                });
            }

            if (bool calcUV = true; calcUV) {

            }

            return isection;
        } else return std::nullopt;
    }

    //satisfy Boundable
    const std::pair<Point, Point> extents;
    const Point center;

    //satisfy Shape
    const std::size_t matIndex;
};

}
