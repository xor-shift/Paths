#pragma once

#include "shape.hpp"

namespace Gfx::Shape {

struct Sphere {
    constexpr Sphere(Point center, Real radius, size_t matIndex)
      : center(center)
        , radius(radius)
        , extents(center - Point{radius, radius, radius},
                  center + Point{radius, radius, radius})
        , matIndex(matIndex) {}

    [[nodiscard]] constexpr std::optional<Intersection> Intersect(const Ray &ray) const noexcept {
        const auto temp = ray.origin - center;

        const Real
          a = Math::Ops::Vector::Dot(ray.direction, ray.direction),
          b = 2. * Math::Ops::Vector::Dot(temp, ray.direction),
          c = Math::Ops::Vector::Dot(temp, temp) - radius * radius;

        const Real disc = b * b - a * c * 4.;
        if (disc < 0) return std::nullopt;
        const Real sqDisc = std::sqrt(disc);

        const Real
          s0 = (-b + sqDisc) / (2. * a),
          s1 = (-b - sqDisc) / (2. * a);

        const Real t = (s0 < 1) ? s1 : (s1 < 1) ? s0 : std::min(s0, s1);

        auto isect = Intersection{
          .theRay = ray,

          .distance = t,
          .normal = {},
          .matIndex = matIndex,

          .uv = {},
        };

        isect.ComputeIntersectionPoint();
        isect.normal = Math::Ops::Vector::Normalized(isect.intersectionPoint - center);

        isect.uv = {
          0.5 + std::atan2(isect.normal[0], isect.normal[2]) * 0.5 * M_1_PI,
          0.5 - std::asin(isect.normal[1]) * M_1_PI,
        };

        return isect;
    }

    const Point center;
    const Real radius;

    const std::pair<Point, Point> extents;

    const size_t matIndex;
};

}
