/**
 * This file contains 2 shapes, Triangle and Parallelogram
 * Both are based off of the same base shape, TriangleImpl, they differ by a single check inside of the intersection routine
 * UV calculations are done out of necessity so IntersectionOptions::calcUV is ineffective (always assumed to be true)
 * The normal is pre-calculated so IntersectionOptions::calcNormal is ineffective (always assumed to be true)
 */

#pragma once

#include "shape.hpp"

namespace Gfx::Shape {

namespace Detail {

enum class TriangleCenterType {
    InCenter,
    Centroid, //center of mass
    Circumcenter,
    Orthocenter,
};

/**
 * Triangle base
 * V2    x -> V3 when parallelogram == true
 * | \
 * |  \
 * E1  \
 * |    \
 * V0-E0-V1
 * @tparam parallelogram
 */
template<bool parallelogram>
class TriangleImpl {
  private:
    std::array<Point, 3> vertices;
    std::array<Point, 2> edges;

  public:
    constexpr explicit TriangleImpl(size_t matIndex, std::array<Point, 3> vertices)
      : vertices(vertices), edges({vertices[1] - vertices[0], vertices[2] - vertices[0]}), extents(parallelogram
                                                                                                   ? std::pair<Point, Point>{
        Maths::Min(Maths::Min(vertices[0], vertices[1]), vertices[2]),
        vertices[0] + (edges[0] + edges[1])}
                                                                                                   : std::pair<Point, Point>{
        Maths::Min(Maths::Min(vertices[0], vertices[1]), vertices[2]),
        Maths::Max(Maths::Max(vertices[0], vertices[1]), vertices[2])}), center(parallelogram ? (extents.first + extents.second) / 2. : PVecCalcCenter(vertices)), matIndex(
      matIndex), normal(Maths::Normalized(Maths::Cross(edges[0], edges[1]))) {}

    [[nodiscard]] constexpr std::optional<Intersection> Intersect(const Ray &ray) const noexcept {
        LIBGFX_NORMAL_CHECK(ray.direction);
        LIBGFX_NORMAL_CHECK(normal);

        const auto h = Maths::Cross(ray.direction, edges[1]);
        const auto a = Maths::Dot(edges[0], h);

        if (std::abs(a) <= sensibleEps) return std::nullopt;

        const auto f = static_cast<Real>(1) / a;

        const auto s = ray.origin - vertices[0];
        const auto u = f * Maths::Dot(s, h);
        if (u < 0 || u > 1) return std::nullopt;

        const auto q = Maths::Cross(s, edges[0]);
        const auto v = f * Maths::Dot(ray.direction, q);
        if (v < 0 || (parallelogram ? v > 1 : u + v > 1)) return std::nullopt;

        const auto t = f * Maths::Dot(edges[1], q);
        if (t <= sensibleEps) return std::nullopt;

        return Intersection(ray, matIndex, t, normal, {u, v});
    }

    std::pair<Point, Point> extents;
    Point center;
    std::size_t matIndex;
    Point normal;

  private:
    [[nodiscard]] static constexpr std::array<Real, 3> PVecKCalc(const std::array<Point, 3> &vertices) noexcept {
        return {Maths::Magnitude(vertices[1] - vertices[2]),
                Maths::Magnitude(vertices[2] - vertices[1]),
                Maths::Magnitude(vertices[0] - vertices[1]),};
    }

    [[nodiscard]] static constexpr Point PVecCalc(Real wA, Real wB, Real wC, const std::array<Point, 3> &vertices) noexcept {
        return (vertices[0] * wA + vertices[1] * wB + vertices[2] * wC) / (wA + wB + wC);
    }

    template<TriangleCenterType type = TriangleCenterType::Centroid>
    [[nodiscard]] static constexpr Point PVecCalcCenter(const std::array<Point, 3> &vertices) {
        const auto[a, b, c] = PVecKCalc(vertices);

        if constexpr (type == TriangleCenterType::InCenter) {
            return PVecCalc(a, b, c, vertices);
        } else if constexpr (type == TriangleCenterType::Centroid) {
            return PVecCalc(1, 1, 1, vertices);
        } else if constexpr (type == TriangleCenterType::Circumcenter) {
            return PVecCalc(
              a * a * (b * b + c * c - a * a),
              b * b * (c * c + a * a - b * b),
              c * c * (a * a + b * b - c * c),
              vertices);
        } else if constexpr (type == TriangleCenterType::Orthocenter) {
            return PVecCalc(
              a * a * a * a - (b * b - c * c) * (b * b - c * c),
              b * b * b * b - (c * c - a * a) * (c * c - a * a),
              c * c * c * c - (a * a - b * b) * (a * a - b * b),
              vertices);
        } else {
            static_assert(type != type, "Cannot calculate triangle center with the given type");
        }
    }
};

}

typedef Detail::TriangleImpl<false> Triangle;
typedef Detail::TriangleImpl<true> Parallelogram;

}