/**
 * This file contains 2 shapes, Triangle and Parallelogram
 * Both are based off of the same base shape, TriangleImpl, they differ by a single check inside of the intersection
 * routine UV calculations are done out of necessity so IntersectionOptions::calcUV is ineffective (always assumed to be
 * true) The normal is pre-calculated so IntersectionOptions::calcNormal is ineffective (always assumed to be true)
 */

#pragma once

#include "Shape.hpp"

namespace Paths::Shape {

namespace Detail {

enum class ETriangleCenterType {
    InCenter,
    Centroid, // center of mass
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
template<bool parallelogram> class TriangleImpl {
private:
    std::array<Point, 3> m_vertices;
    std::array<Point, 2> m_edges;

public:
    constexpr explicit TriangleImpl(size_t mat_index, std::array<Point, 3> vertices)
        : m_vertices(vertices)
        , m_edges({ vertices[1] - vertices[0], vertices[2] - vertices[0] })
        , m_extents(parallelogram
                  ? std::pair<Point, Point> { Maths::min(Maths::min(vertices[0], vertices[1]), vertices[2]),
                      vertices[0] + (m_edges[0] + m_edges[1]) }
                  : std::pair<Point, Point> { Maths::min(Maths::min(vertices[0], vertices[1]), vertices[2]),
                      Maths::max(Maths::max(vertices[0], vertices[1]), vertices[2]) })
        , m_center(parallelogram ? (m_extents.first + m_extents.second) / 2. : p_vec_calc_center(vertices))
        , m_mat_index(mat_index)
        , m_normal(Maths::normalized(Maths::cross(m_edges[0], m_edges[1]))) { }

    [[nodiscard]] constexpr std::optional<Intersection> intersect_ray(const Ray &ray) const noexcept {
        LIBGFX_NORMAL_CHECK(ray.m_direction);
        LIBGFX_NORMAL_CHECK(m_normal);

        const auto h = Maths::cross(ray.m_direction, m_edges[1]);
        const auto a = Maths::dot(m_edges[0], h);

        if (std::abs(a) <= sensible_eps)
            return std::nullopt;

        const auto f = static_cast<Real>(1) / a;

        const auto s = ray.m_origin - m_vertices[0];
        const auto u = f * Maths::dot(s, h);
        if (u < 0 || u > 1)
            return std::nullopt;

        const auto q = Maths::cross(s, m_edges[0]);
        const auto v = f * Maths::dot(ray.m_direction, q);
        if (v < 0 || (parallelogram ? v > 1 : u + v > 1))
            return std::nullopt;

        const auto t = f * Maths::dot(m_edges[1], q);
        if (t <= sensible_eps)
            return std::nullopt;

        return Intersection(ray, m_mat_index, t, m_normal, { u, v });
    }

    std::pair<Point, Point> m_extents;
    Point m_center;
    std::size_t m_mat_index;
    Point m_normal;

private:
    [[nodiscard]] static constexpr std::array<Real, 3> p_vec_k_calc(const std::array<Point, 3> &vertices) noexcept {
        return {
            Maths::Magnitude(vertices[1] - vertices[2]),
            Maths::Magnitude(vertices[2] - vertices[1]),
            Maths::Magnitude(vertices[0] - vertices[1]),
        };
    }

    [[nodiscard]] static constexpr Point p_vec_calc(
        Real w_a, Real w_b, Real w_c, const std::array<Point, 3> &vertices) noexcept {
        return (vertices[0] * w_a + vertices[1] * w_b + vertices[2] * w_c) / (w_a + w_b + w_c);
    }

    template<ETriangleCenterType type = ETriangleCenterType::Centroid>
    [[nodiscard]] static constexpr Point p_vec_calc_center(const std::array<Point, 3> &vertices) {
        const auto [a, b, c] = p_vec_k_calc(vertices);

        if constexpr (type == ETriangleCenterType::InCenter) {
            return p_vec_calc(a, b, c, vertices);
        } else if constexpr (type == ETriangleCenterType::Centroid) {
            return p_vec_calc(1, 1, 1, vertices);
        } else if constexpr (type == ETriangleCenterType::Circumcenter) {
            return p_vec_calc(a * a * (b * b + c * c - a * a), b * b * (c * c + a * a - b * b),
                c * c * (a * a + b * b - c * c), vertices);
        } else if constexpr (type == ETriangleCenterType::Orthocenter) {
            return p_vec_calc(a * a * a * a - (b * b - c * c) * (b * b - c * c),
                b * b * b * b - (c * c - a * a) * (c * c - a * a), c * c * c * c - (a * a - b * b) * (a * a - b * b),
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