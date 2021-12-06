#pragma once

#include "shape.hpp"

namespace Gfx::Shape {

class Sphere {
  public:
    constexpr Sphere(size_t matIndex, Point center, Real radius)
      : center(center), extents(center - Point(radius, radius, radius),
                                center + Point(radius, radius, radius)), radius(radius), matIndex(matIndex) {}

    [[nodiscard]] constexpr std::optional<Intersection> Intersect(const Ray &ray) const noexcept {
        const auto temp = ray.origin - center;

        const Real
          a = Maths::Dot(ray.direction, ray.direction),
          b = static_cast<Real>(2) * Maths::Dot(temp, ray.direction),
          c = Maths::Dot(temp, temp) - radius * radius;

        const Real disc = b * b - a * c * static_cast<Real>(4);
        if (disc < 0) return std::nullopt;
        const Real sqDisc = std::sqrt(disc);

        const Real
          s0 = (-b + sqDisc) / (static_cast<Real>(2) * a),
          s1 = (-b - sqDisc) / (static_cast<Real>(2) * a);

        const Real t = (s0 < 1) ? s1 : (s1 < 1) ? s0 : std::min(s0, s1);

        Intersection isect(ray, matIndex, t);

        isect.normal = Maths::Normalized(isect.intersectionPoint - center);
        isect.goingIn = Maths::Dot(isect.normal, ray.direction) < 0;
        isect.orientedNormal = isect.goingIn ? isect.normal : -isect.normal;

        isect.uv = {static_cast<Real>(.5) + std::atan2(isect.normal[0], isect.normal[2]) * static_cast<Real>(.5) * M_1_PI,
                    static_cast<Real>(.5) - std::asin(isect.normal[1]) * M_1_PI,};

        return isect;
    }

    Point center;
    std::pair<Point, Point> extents;

  private:
    Real radius;
    size_t matIndex;
};

}
