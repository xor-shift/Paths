#pragma once

#include "shape.hpp"

#include "plane.hpp"

namespace Gfx::Shape {

namespace Impl {
//https://math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d
[[nodiscard]] inline Maths::Matrix<Real, 3, 3> RotMatFor(const Point &from, const Point &to) noexcept {
    auto v = Maths::Cross(from, to);
    auto c = Maths::Dot(from, to);
    const Maths::Matrix<Real, 3, 3> ssm = Maths::SSC(v);
    return Maths::Identity<Real, 3>() + ssm + ssm * ssm * (1. / (1. + c));
}

template<bool lazy>
[[nodiscard]] constexpr std::pair<Point, Point> Extents(const Point &center, Real radius, const Point &normal) {
    if constexpr (lazy) {
        return {center - Point(radius, radius, radius), center + Point(radius, radius, radius)};
    } else { //i am tired
        //const auto rot0 = RotMatFor({{0, 0, 1}}, normal);

        const Real l_x = Maths::Dot(normal, Point(1, 0, 0)) * radius;
        const Real l_y = Maths::Dot(normal, Point(0, 1, 0)) * radius;
        const Real l_z = Maths::Dot(normal, Point(0, 0, 1)) * radius;

        return {{center[0] - l_x, center[1] - l_y, center[2] - l_z},
                {center[0] + l_x, center[1] + l_y, center[2] + l_z}};
    }
}
}

class Disc {
  public:
    Disc(size_t matIndex, Point center, Point normal, Real radius)
      : extents(Impl::Extents<true>(center, radius, normal)), center(center), impl(0, center, normal), radius(radius), matIndex(matIndex) {}

    [[nodiscard]] constexpr std::optional<Intersection> Intersect(const Ray &ray) const noexcept {
        auto i = impl.Intersect(ray);
        i->matIndex = matIndex;
        const auto d = i->intersectionPoint - impl.center;

        if (Maths::Dot(d, d) > radius) return std::nullopt;
        else return i;
    }

    std::pair<Point, Point> extents;
    Point center;

  private:
    Plane impl;
    Real radius;
    size_t matIndex;
};

}