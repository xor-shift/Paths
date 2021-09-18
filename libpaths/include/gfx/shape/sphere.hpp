#pragma once

#include "shape.hpp"

namespace Gfx::Shape {

class Sphere {
  public:
    constexpr Sphere(size_t matIndex, Point center, Real radius)
      : center(center)
        , extents(center - Point{{radius, radius, radius}},
                  center + Point{{radius, radius, radius}})
        , radius(radius)
        , matIndex(matIndex) {}

    [[nodiscard]] constexpr std::optional<Intersection> Intersect(const Ray &ray) const noexcept {
        const auto temp = ray.origin - center;

        const Real
          a = Math::Dot(ray.direction, ray.direction),
          b = 2. * Math::Dot(temp, ray.direction),
          c = Math::Dot(temp, temp) - radius * radius;

        const Real disc = b * b - a * c * 4.;
        if (disc < 0) return std::nullopt;
        const Real sqDisc = std::sqrt(disc);

        const Real
          s0 = (-b + sqDisc) / (2. * a),
          s1 = (-b - sqDisc) / (2. * a);

        const Real t = (s0 < 1) ? s1 : (s1 < 1) ? s0 : std::min(s0, s1);

        Intersection isect(ray, matIndex, t);

        isect.normal = Math::Normalized(isect.intersectionPoint - center);

        isect.uv = {{
                      0.5 + std::atan2(isect.normal[0], isect.normal[2]) * 0.5 * M_1_PI,
                      0.5 - std::asin(isect.normal[1]) * M_1_PI,
                    }};

        return isect;
    }

    const Point center;
    const std::pair<Point, Point> extents;

  private:
    const Real radius;
    const size_t matIndex;
};

}