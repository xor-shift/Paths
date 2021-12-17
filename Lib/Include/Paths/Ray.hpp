#pragma once

#include <optional>

#include "Common.hpp"
#include "Maths/Random.hpp"

namespace Paths {

namespace Detail {

enum class EMajorAxis : int {
    PosX = 0,
    NegX = 1,
    PosY = 2,
    NegY = 3,
    PosZ = 4,
    NegZ = 5,
};

static constexpr EMajorAxis get_major_direction(const Point &direction) noexcept {
    const Point abs = Maths::abs(direction);
    std::size_t major_axis = std::distance(abs.cbegin(), std::max_element(abs.cbegin(), abs.cend()));
    return static_cast<EMajorAxis>(static_cast<std::size_t>(direction[major_axis] < 0) + major_axis * 2);
}

}

struct Ray {
    constexpr Ray(Point origin, Point direction)
        : m_origin(origin)
        , m_direction(direction)
        , m_direction_reciprocals(Maths::reciprocal(direction))
        , m_major_direction(Detail::get_major_direction(direction)) { }

    Point m_origin;
    Point m_direction;
    Point m_direction_reciprocals;
    Detail::EMajorAxis m_major_direction;
};

/*
\vec{l}
\   |
 \  |
  \1|
   \| n1  v1
----o-----
    |\ n2 v2
    ||
    |2\
    | |
    |  \

 1 -> \theta_1
 2 -> \theta_2
 n_n -> IOR_n
 v_n -> speed of light_n

 reflection:
 \cos{\theta_1} = -\vec{n}\cdot{\vec{l}}
 \vec{v}_{reflect} = \vec{l} + 2\cos{\theta_1}\vec{n}
 \vec{v}_{reflect} = \vec{l} - 2(\vec{n}\cdot{\vec{l}})\vec{n}

 refraction:
 \frac{\sin{\theta_2}}{\sin{\theta_1}}
 = \frac{v_2}{v_1}
 = \frac{n_1}{n_2}

 knowing that \sin{\theta_2} = (\frac{n_1}{n_2})\sin{\theta_1} (snell's law)
 and given \sin^2{\alpha} = 1-\cos^2{\alpha} hence \sin{\alpha} = \sqrt{1-\cos^2{\alpha}}
 we can say that \sin{\theta_2} = (\frac{n_1}{n_2})\sqrt{1-\cos^2{\theta_1}}

 similarly, \cos{\theta_2} = \sqrt{1-\sin^2{\theta_2}} = \sqrt{1-(\frac{n_1}{n_2})^2(1-\cos^2{\theta_1})}

 \vec{v}_{refract} = (\frac{n_1}{n_2})\vec{l} + (\frac{n_1}{n_2}\cos{\theta_1}-\cos{\theta_2})\vec{n}
 = (\frac{n_1}{n_2})\vec{l} + (\frac{n_1}{n_2}\cos{\theta_1}-\sqrt{1-(\frac{n_1}{n_2})^2(1-\cos^2{\theta_1})})\vec{n}

 substituting r for \frac{n_1}{n_2} and c for \cos^2{\theta_1} = -\vec{n}\cdot{\vec{l}} we get:
 \vec{v}_{refract} = r\vec{l} + (rc-\sqrt{1-r^2(1-c^2)})
 */
namespace Detail {

/// Reflects a vector
/// \param vec ray direction, pointing towards the surface, normalised
/// \param normal oriented surface normal
static constexpr Point reflect_vector(Point vec, Point normal) noexcept {
    return vec - normal * Maths::dot(normal, vec) * static_cast<Real>(2);
}

/// Refracts a vector
/// \param vec ray direction, pointing towards the surface, normalised
/// \param normal oriented surface normal
/// \param r n_1/n_2 where n_1 is the ior of the volume the ray is coming from and n_2 is the ior of the one being
/// entered
static constexpr Point refract_vector(Point vec, Point normal, Real r) noexcept {
    const Real c = -Maths::dot(vec, normal);
    return vec * r + normal * (r * c - std::sqrt(1 - r * r * (1 - c * c)));
}

///
/// \param light The position of the light
/// \param intersection The point of ray-surface intersection
/// \param n The oriented normal at the point of intersection
/// \param v The vector pointing from the intersection to the viewer
/// \return in order, the lambertian and the specular coefficient
static inline std::pair<Real, Real> blinn_phong_coefficients(
    Point light, Point intersection, Point n, Point v, Real shininess = 16) noexcept {
    const auto l = Maths::normalized(light - intersection);
    // const auto r = Detail::Reflect(-l, n);
    const auto h = Maths::normalized(l + v);

    const Real spec_angle = std::max<Real>(Maths::dot(h, n), 0);
    const Real specular = std::pow(spec_angle, shininess);
    const Real lambertian = std::max<Real>(Maths::dot(l, n), 0);

    return { lambertian, specular };
}

}

static_assert(std::is_trivially_copyable_v<Ray>);
static_assert(std::is_trivially_destructible_v<Ray>);

struct Intersection {
    constexpr Intersection(const Ray &ray, size_t mat_index, Real distance, Point normal = { 0, 0, 0 },
        Maths::Vector<Real, 2> uv = { 0, 0 }

        )
        : m_mat_index(mat_index)
        , m_distance(distance)
        , m_intersection_point(ray.m_origin + ray.m_direction * distance)
        , m_normal(normal)
        , m_going_in(Maths::dot(normal, ray.m_direction) < 0)
        , m_oriented_normal(m_going_in ? normal : -normal)
        , m_uv(uv) { }

    std::size_t m_mat_index {};

    Real m_distance {};
    Point m_intersection_point;

    Point m_normal {};
    bool m_going_in {};
    Point m_oriented_normal {};

    Maths::Vector<Real, 2> m_uv { 0, 0 };

    static constexpr bool replace(std::optional<Intersection> &old, std::optional<Intersection> &&with) noexcept {
        if (with && (!old || ((with->m_distance < old->m_distance) && with->m_distance > 0))) {
            old.operator=(std::forward<Intersection &&>(*with));
            return true;
        }
        return false;
    }
};

static_assert(std::is_trivially_copyable_v<Intersection>);
static_assert(std::is_trivially_destructible_v<Intersection>);

}
