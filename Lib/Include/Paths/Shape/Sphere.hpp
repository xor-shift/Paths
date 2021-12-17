#pragma once

#include "Shape.hpp"

namespace Paths::Shape {

class Sphere {
public:
    constexpr Sphere(size_t mat_index, Point center, Real radius)
        : m_center(center)
        , m_extents(center - Point(radius, radius, radius), center + Point(radius, radius, radius))
        , m_radius(radius)
        , m_mat_index(mat_index) { }

    [[nodiscard]] constexpr std::optional<Intersection> intersect_ray(const Ray &ray) const noexcept {
        const auto temp = ray.m_origin - m_center;

        const Real a = Maths::dot(ray.m_direction, ray.m_direction);
        const Real b = static_cast<Real>(2) * Maths::dot(temp, ray.m_direction);
        const Real c = Maths::dot(temp, temp) - m_radius * m_radius;

        const Real disc = b * b - a * c * static_cast<Real>(4);
        if (disc < 0)
            return std::nullopt;
        const Real sq_disc = std::sqrt(disc);

        const Real s_0 = (-b + sq_disc) / (static_cast<Real>(2) * a);
        const Real s_1 = (-b - sq_disc) / (static_cast<Real>(2) * a);

        const Real t = (s_0 < 1) ? s_1 : (s_1 < 1) ? s_0 : std::min(s_0, s_1);

        Intersection isect(ray, m_mat_index, t);

        isect.m_normal = Maths::normalized(isect.m_intersection_point - m_center);
        isect.m_going_in = Maths::dot(isect.m_normal, ray.m_direction) < 0;
        isect.m_oriented_normal = isect.m_going_in ? isect.m_normal : -isect.m_normal;

        isect.m_uv = {
            static_cast<Real>(.5) + std::atan2(isect.m_normal[0], isect.m_normal[2]) * static_cast<Real>(.5) * M_1_PI,
            static_cast<Real>(.5) - std::asin(isect.m_normal[1]) * M_1_PI,
        };

        return isect;
    }

    Point m_center;
    std::pair<Point, Point> m_extents;

private:
    Real m_radius;
    size_t m_mat_index;
};

}
