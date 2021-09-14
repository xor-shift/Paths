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

    template<bool getDistance = false>
    [[nodiscard]] static constexpr auto IntersectsImpl(const std::pair<Point, Point> &extents, const Ray &ray) noexcept {
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

  public:
    constexpr AABox(size_t matIndex, Point min, Point max) noexcept
      : extents(min, max)
        , center((min + max) / 2.)
        , matIndex(matIndex) {}

    static constexpr bool EIntersects(const std::pair<Point, Point> &extents, const Ray &ray) noexcept { return IntersectsImpl<false>(extents, ray); }

    template<bool getDistance = false>
    [[nodiscard]] constexpr auto Intersects(const Ray &ray) const noexcept { return IntersectsImpl<getDistance>(extents, ray); }

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

    const std::pair<Point, Point> extents;
    const Point center;

  private:
    const std::size_t matIndex;
};

/**
 * Checks if b0 contains b1
 * @param b0
 * @param b1
 * @return
 */
constexpr bool InBounds(const std::pair<Point, Point> &b0, const std::pair<Point, Point> &b1) {
    for (size_t i = 0; i < Point::vectorSize; i++) {
        if (b0.first[i] > b1.first[i]) return false;
        if (b0.second[i] < b1.second[i]) return false;
    }
    return true;
}

}
