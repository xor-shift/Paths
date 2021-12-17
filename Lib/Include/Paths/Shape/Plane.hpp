/**
 * This file contains a Plane shape, described by a center and a normal, extending infinitely
 * UV calculations are not done due to the nature of the shape so IntersectionOptions::calcUV is ineffective (always
 * assumed to be false) The normal is a defining property so IntersectionOptions::calcNormal is ineffective (always
 * assumed to be true)
 */

#pragma once

#include "Shape.hpp"

namespace Paths::Shape {

class Disc;

class Plane {
public:
    constexpr Plane(size_t mat_index, Point center, Point normal)
        : m_mat_index(mat_index)
        , m_center(center)
        , m_normal(Maths::normalized(normal)) { }

    [[nodiscard]] constexpr std::optional<Intersection> intersect_ray(const Ray &ray) const noexcept {
        LIBGFX_NORMAL_CHECK(ray.m_direction);
        LIBGFX_NORMAL_CHECK(m_normal);

        auto t = intersect_impl(ray);

        if (t < sensible_eps)
            return std::nullopt;
        return Intersection(ray, m_mat_index, t, m_normal, { 0, 0 });
    }

    [[nodiscard]] constexpr Real intersect_impl(const Ray &ray) const noexcept {
        LIBGFX_NORMAL_CHECK(ray.m_direction);
        LIBGFX_NORMAL_CHECK(m_normal);

        const auto denom = Maths::dot(m_normal, ray.m_direction);

        if (std::abs(denom) <= sensible_eps)
            return -1;

        return Maths::dot(m_center - ray.m_origin, m_normal) / denom;
    }

private:
    friend class Disc;

    size_t m_mat_index { 0 };
    Point m_center {};
    Point m_normal {};
};

}
