/**
 * This file contains 2 shapes, Triangle and Parallelogram
 * Both are based off of the same base shape, TriangleImpl, they differ by a single check inside of the intersection routine
 * UV calculations are done out of necessity so IntersectionOptions::calcUV is ineffective (always assumed to be true)
 * The normal is pre-calculated so IntersectionOptions::calcNormal is ineffective (always assumed to be true)
 */

#pragma once

#include <gfx/concepts/shape.hpp>
#include <math/math.hpp>

namespace Gfx::Shape {

template<bool parallelogram>
struct TriangleImpl {
    explicit TriangleImpl(std::size_t matIndex, std::array<Point, 3> vertices)
      : vertices(vertices)
        , edges({vertices[1] - vertices[0], vertices[2] - vertices[0]})
        , normal(Math::Ops::Vector::Normalized(Math::Ops::Vector::Cross(edges[0], edges[1])))
        , matIndex(matIndex) {}

    [[nodiscard]] constexpr std::optional<Intersection> Intersect(const Ray &ray) const noexcept {
        LIBGFX_NORMAL_CHECK(ray.direction);
        LIBGFX_NORMAL_CHECK(normal);

        auto h = Math::Ops::Vector::Cross(ray.direction, edges[1]);
        auto a = edges[0].Dot(h);

        if (std::abs(a) <= Epsilon) return std::nullopt;

        auto f = 1. / a;

        auto s = ray.origin - vertices[0];
        auto u = f * s.Dot(h);
        if (0. > u || u > 1.) return std::nullopt;

        auto q = Math::Ops::Vector::Cross(s, edges[0]);
        auto v = f * ray.direction.Dot(q);
        if (0. > v || (parallelogram ? v > 1. : u + v > 1.)) return std::nullopt;

        auto t = f * edges[1].Dot(q);
        if (t <= Epsilon) return std::nullopt;

        return Intersection{
          .theRay = ray,
          .distance = t,
          .normal = normal,
          .matIndex = matIndex,
          .uv = {u, v},
        };
    }

    std::array<Point, 3> vertices{};
    std::array<Point, 2> edges{};
    Point normal{};

    std::size_t matIndex{0};
    constexpr static bool boundable = true;
};

typedef TriangleImpl<false> Triangle;
typedef TriangleImpl<true> Parallelogram;

}