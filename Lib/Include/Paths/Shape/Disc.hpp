#pragma once

#include "Plane.hpp"
#include "Shape.hpp"

#include "Maths/MatVec.hpp"

namespace Paths::Shape {

namespace Impl {
// https://math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d
[[nodiscard]] inline Maths::Matrix<Real, 3, 3> rot_mat_for(const Point &from, const Point &to) noexcept {
    auto v = Maths::cross(from, to);
    auto c = Maths::dot(from, to);
    const Maths::Matrix<Real, 3, 3> ssm = Maths::ssc(v);
    return Maths::identity_matrix<Real, 3>() + ssm + ssm * ssm * (1. / (1. + c));
}

template<bool lazy>
[[nodiscard]] constexpr std::pair<Point, Point> extents(const Point &center, Real radius, const Point &normal) {
    if constexpr (lazy) {
        return { center - Point(radius, radius, radius), center + Point(radius, radius, radius) };
    } else { // i am tired
        // const auto rot0 = RotMatFor({{0, 0, 1}}, normal);

        const Real l_x = Maths::dot(normal, Point(1, 0, 0)) * radius;
        const Real l_y = Maths::dot(normal, Point(0, 1, 0)) * radius;
        const Real l_z = Maths::dot(normal, Point(0, 0, 1)) * radius;

        return { { center[0] - l_x, center[1] - l_y, center[2] - l_z },
            { center[0] + l_x, center[1] + l_y, center[2] + l_z } };
    }
}
}

class Disc {
public:
    Disc(size_t mat_index, Point center, Point normal, Real radius)
        : m_extents(Impl::extents<true>(center, radius, normal))
        , m_center(center)
        , m_impl(0, center, normal)
        , m_radius(radius)
        , m_mat_index(mat_index) { }

    [[nodiscard]] constexpr std::optional<Intersection> intersect_ray(const Ray &ray) const noexcept {
        auto i = m_impl.intersect_ray(ray);
        i->m_mat_index = m_mat_index;
        const auto d = i->m_intersection_point - m_impl.m_center;

        if (Maths::dot(d, d) > m_radius)
            return std::nullopt;
        else
            return i;
    }

    std::pair<Point, Point> m_extents;
    Point m_center;

private:
    Plane m_impl;
    Real m_radius;
    size_t m_mat_index;
};

}