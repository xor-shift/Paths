#pragma once

#include <chrono>

#include "Shape.hpp"

namespace Paths::Shape {

struct AxisAlignedBox {
private:
    template<bool getDistance> using IntersectionType = std::conditional_t<getDistance, std::pair<bool, Real>, bool>;

    template<bool getDistance>
    [[nodiscard]] static constexpr IntersectionType<getDistance> func(Real t_min, Real t_max) noexcept {
        const bool isect = t_max > std::max<Real>(t_min, 0.);

        if constexpr (getDistance) {
            return std::pair<bool, Real> { isect, t_min > 0. ? t_min : t_max };
        } else {
            return isect;
        }
    }

    template<bool getDistance = false>
    [[nodiscard]] static constexpr auto intersects_impl(
        const std::pair<Point, Point> &extents, const Ray &ray) noexcept {
        Real t_min = std::numeric_limits<Real>::min();
        Real t_max = std::numeric_limits<Real>::max();

        for (size_t i = 0; i < 3; ++i) {
            const auto t_1 = (extents.first[i] - ray.m_origin[i]) * ray.m_direction_reciprocals[i];
            const auto t_2 = (extents.second[i] - ray.m_origin[i]) * ray.m_direction_reciprocals[i];

            t_min = std::max(t_min, std::min(std::min(t_1, t_2), t_max));
            t_max = std::min(t_max, std::max(std::max(t_1, t_2), t_min));
        }

        return func<getDistance>(t_min, t_max);
    }

public:
    constexpr AxisAlignedBox(size_t mat_index, Point min, Point max) noexcept
        : m_extents(min, max)
        , m_center((min + max) / 2.)
        , m_mat_index(mat_index) { }

    static constexpr bool ray_intersects(const std::pair<Point, Point> &extents, const Ray &ray) noexcept {
        return intersects_impl<false>(extents, ray);
    }

    template<bool getDistance = false> [[nodiscard]] constexpr auto ray_intersects(const Ray &ray) const noexcept {
        return intersects_impl<getDistance>(m_extents, ray);
    }

    [[nodiscard]] constexpr std::optional<Intersection> intersect_ray(const Ray &ray) const noexcept {
        const auto &[intersects, dist] = ray_intersects<true>(ray);

        if (!intersects)
            return std::nullopt;

        auto isection = Intersection(ray, m_mat_index, dist);

        const auto p = isection.m_intersection_point - (m_extents.first + m_extents.second) * static_cast<Real>(.5);
        const auto d = (m_extents.first - m_extents.second) * static_cast<Real>(.5);
        constexpr Real bias = 1.000001;

        isection.m_normal = Maths::normalized(Point(std::trunc(p[0] / std::abs(d[0]) * bias),
            std::trunc(p[1] / std::abs(d[1]) * bias), std::trunc(p[2] / std::abs(d[2]) * bias)));

        return isection;
    }

    std::pair<Point, Point> m_extents;
    Point m_center;

private:
    std::size_t m_mat_index;
};

/**
 * Checks if b0 contains b1
 * @param b_0
 * @param b_1
 * @return
 */
constexpr bool in_bounds(const std::pair<Point, Point> &b_0, const std::pair<Point, Point> &b_1) {
    for (size_t i = 0; i < Point::m_vector_size; i++) {
        if (b_0.first[i] > b_1.first[i])
            return false;
        if (b_0.second[i] < b_1.second[i])
            return false;
    }
    return true;
}

constexpr bool in_bounds(const std::pair<Point, Point> &b_0, Point p) {
    for (size_t i = 0; i < Point::m_vector_size; i++) {
        if (b_0.first[i] > p[i])
            return false;
        if (b_0.second[i] < p[i])
            return false;
    }
    return true;
}

}
