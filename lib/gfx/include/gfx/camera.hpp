#pragma once

#include <gfx/common.hpp>
#include <gfx/ray.hpp>
#include <maths/vector.hpp>
#include <maths/matrix.hpp>
#include <maths/matvec.hpp>

namespace Gfx {

struct Camera {
    Point position{};
    Maths::Vector<std::size_t, 2> resolution{};
    Matrix rayTransform{};

    Real fovHint{45};

    Real focalDistance{1};
    Real apertureDiameter{1};

    Camera &SetLookDeg(Point l);

    Camera &SetLookRad(Point l);

    Camera &SetLookAt(Point p);

    Ray MakeRay(std::size_t x, std::size_t y);

    void Prepare();

  private:
    Real resolutionScale;
    Maths::Vector<Real, 2> scaledResolution;
    Real viewingPlaneDistance;
};

}