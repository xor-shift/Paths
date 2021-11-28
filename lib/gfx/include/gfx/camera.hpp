#pragma once

#include <gfx/common.hpp>
#include <maths/vector.hpp>
#include <maths/matrix.hpp>

namespace Gfx {

struct Camera {
    Point position{};
    Real fovHint{45}; // Hint for the integrator for the horizontal field of view in degrees, not guaranteed to be respected
    Maths::Vector<std::size_t, 2> resolution{};

    Matrix rayTransform{};

    Camera &SetLookDeg(Point l);

    Camera &SetLookRad(Point l);

    Camera &SetLookAt(Point p);
};

}