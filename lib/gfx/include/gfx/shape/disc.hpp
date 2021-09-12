#pragma once

#include "shape.hpp"

#include "plane.hpp"

namespace Gfx::Shape {

/**
 * Won't calculate UVs, is meant only for lighting (makes explicit lighting easy)
 */
class Disc {
  private:
    const Plane impl;
    const Real radius;

    //https://math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d
    [[nodiscard]] static Math::Matrix<Real, 3, 3> RotMatFor(const Point &from, const Point &to) noexcept {
        auto v = Math::Cross(from, to);
        auto c = Math::Dot(from, to);
        const Math::Matrix<Real, 3, 3> ssm = Math::SSC(v);
        return Math::Identity<Real, 3>() + ssm + ssm * ssm * (1. / (1. + c));
    }

    template<bool lazy>
    [[nodiscard]] static constexpr std::pair<Point, Point> Extents(const Point &center, Real radius, const Point &normal) {
        if constexpr (lazy) {
            return {center - Point{{radius, radius, radius}}, center + Point{{radius, radius, radius}}};
        } else { //i am tired
            //const auto rot0 = RotMatFor({{0, 0, 1}}, normal);

            const Real l_x = Math::Dot(normal, Point{{1, 0, 0}}) * radius;
            const Real l_y = Math::Dot(normal, Point{{0, 1, 0}}) * radius;
            const Real l_z = Math::Dot(normal, Point{{0, 0, 1}}) * radius;

            return {{{
                center[0] - l_x,
                center[1] - l_y,
                center[2] - l_z,
            }}, {{
                center[0] + l_x,
                center[1] + l_y,
                center[2] + l_z,
            }}};
        }
    }

  public:
    Disc(size_t matIndex, Point center, Point normal, Real radius)
      : impl(center, normal, 0)
        , radius(radius)
        , center(center)
        , matIndex(matIndex)
        , extents(Extents<true>(center, radius, normal)) {}

    [[nodiscard]] constexpr std::optional<Intersection> Intersect(const Ray &ray) const noexcept {
        auto i = impl.Intersect(ray);
        i->matIndex = matIndex;
        const auto d = i->intersectionPoint - impl.center;

        if (Math::Dot(d, d) > radius) return std::nullopt;
        else return i;
    }

    const Point center;
    const size_t matIndex;
    const std::pair<Point, Point> extents;
};

}