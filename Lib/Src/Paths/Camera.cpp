#include "Paths/Camera.hpp"

namespace Paths {

Camera &Camera::set_look_deg(Point l) { return set_look_rad(l * static_cast<Real>(M_PI / 180.)); }

Camera &Camera::set_look_rad(Point l) {
    m_ray_transform = Maths::Matrix<Real, 3, 3>::rotation(l[0], l[1], l[2]);
    return *this;
}

Camera &Camera::set_look_at(Point p) {
    Point d = Maths::normalized(p - m_position);

    auto rot_align = [](Point v_1, Point v_2) -> Matrix {
        Point axis = Maths::cross(v_1, v_2);

        const Real cos_a = Maths::dot(v_1, v_2);
        const Real k = 1.0f / (1.0f + cos_a);

        Matrix result((axis[0] * axis[0] * k) + cos_a, (axis[1] * axis[0] * k) - axis[2],
            (axis[2] * axis[0] * k) + axis[1],

            (axis[0] * axis[1] * k) + axis[2], (axis[1] * axis[1] * k) + cos_a, (axis[2] * axis[1] * k) - axis[0],

            (axis[0] * axis[2] * k) - axis[1], (axis[1] * axis[2] * k) + axis[0], (axis[2] * axis[2] * k) + cos_a);

        return Maths::transpose(result);
    };

    m_ray_transform = rot_align(d, Paths::Point(0, 0, 1));

    return *this;
}

Ray Camera::make_ray(std::size_t x, std::size_t y) {
    const auto nudge = Maths::Random::unit_disk();
    const Point base_vector { (static_cast<Real>(x) + nudge[0] - .5) * m_resolution_scale - m_scaled_resolution[0] / 2.,
        (-static_cast<Real>(y) + nudge[1] - .5) * m_resolution_scale + m_scaled_resolution[1] / 2., m_focal_distance };

    if (m_aperture_diameter > 0.001) {
        const auto aperture_offset = Maths::Random::unit_disk() * m_aperture_diameter;
        const Point aperture_offset_point { aperture_offset[0], aperture_offset[1] };

        return { m_position + m_ray_transform * aperture_offset_point,
            Maths::normalized(m_ray_transform * base_vector - aperture_offset_point) };
    }

    return { m_position, Maths::normalized(m_ray_transform * base_vector) };
}

void Camera::prepare() {
    const auto w = static_cast<Real>(m_resolution[0]) / 2.;
    const auto varphi = m_fov_hint / 2.;

    m_viewing_plane_distance = w / std::tan(varphi / 180. * M_PI);

    m_resolution_scale = m_focal_distance / m_viewing_plane_distance;
    m_scaled_resolution = m_resolution;
    m_scaled_resolution *= m_resolution_scale;
}

}
