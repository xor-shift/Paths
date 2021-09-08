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

    /**
     * Does not calculate UV coordinates
     * @param ray
     * @return
     */
    [[nodiscard]] constexpr std::optional<Intersection> Intersect(const Ray &ray) const noexcept {
        if (const auto &[isect, dist] = Intersects<true>(ray); isect) {
            auto isection = Intersection(ray, matIndex, dist);

            const auto p = isection.intersectionPoint - (extents.first + extents.second) * .5;
            const auto d = (extents.first - extents.second) * .5;
            const Real bias = 1.000001;

            isection.normal = Math::Normalized(Point{{
                                                       std::trunc(p[0] / std::abs(d[0]) * bias),
                                                       std::trunc(p[1] / std::abs(d[1]) * bias),
                                                       std::trunc(p[2] / std::abs(d[2]) * bias),
                                                     }});

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
