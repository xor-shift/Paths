#pragma once

#include "Common.hpp"
#include "Maths/MatVec.hpp"
#include "Maths/Matrix.hpp"
#include "Maths/Vector.hpp"
#include "Ray.hpp"

namespace Paths {

struct Camera {
    Point m_position {};
    Maths::Vector<std::size_t, 2> m_resolution {};
    Matrix m_ray_transform {};

    Real m_fov_hint { 45 };

    Real m_focal_distance { 1 };
    Real m_aperture_diameter { 1 };

    Camera &set_look_deg(Point l);

    Camera &set_look_rad(Point l);

    Camera &set_look_at(Point v_1);

    Ray make_ray(std::size_t x, std::size_t y);

    void prepare();

private:
    Real m_resolution_scale;
    Maths::Vector<Real, 2> m_scaled_resolution;
    Real m_viewing_plane_distance;
};

}