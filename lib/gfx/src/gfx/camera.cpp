#include <gfx/camera.hpp>

namespace Gfx {

Camera &Camera::SetLookDeg(Point l) {
    return SetLookRad(l * static_cast<Real>(M_PI / 180.));
}

Camera &Camera::SetLookRad(Point l) {
    rayTransform = Maths::Matrix<Real, 3, 3>::Rotation(l[0], l[1], l[2]);
    return *this;
}

Camera &Camera::SetLookAt(Point p) {
    Point d = Maths::Normalized(p - position);

    auto RotAlign = [](Point v1, Point v2) -> Matrix {
        Point axis = Maths::Cross(v1, v2);

        const Real cosA = Maths::Dot(v1, v2);
        const Real k = 1.0f / (1.0f + cosA);

        Matrix result(
          (axis[0] * axis[0] * k) + cosA,
          (axis[1] * axis[0] * k) - axis[2],
          (axis[2] * axis[0] * k) + axis[1],

          (axis[0] * axis[1] * k) + axis[2],
          (axis[1] * axis[1] * k) + cosA,
          (axis[2] * axis[1] * k) - axis[0],

          (axis[0] * axis[2] * k) - axis[1],
          (axis[1] * axis[2] * k) + axis[0],
          (axis[2] * axis[2] * k) + cosA
        );

        return Maths::Transpose(result);
    };

    rayTransform = RotAlign(d, Gfx::Point(0, 0, 1));

    return *this;
}

Ray Camera::MakeRay(std::size_t x, std::size_t y) {
    const auto nudge = Maths::Random::UnitDisk();
    const Point baseVector{(static_cast<Real>(x) + nudge[0] - .5) * resolutionScale - scaledResolution[0] / 2.,
                           (-static_cast<Real>(y) + nudge[1] - .5) * resolutionScale + scaledResolution[1] / 2.,
                           focalDistance};

    if (apertureDiameter > 0.001) {
        const auto apertureOffset = Maths::Random::UnitDisk() * apertureDiameter;
        const Point apertureOffsetPoint{apertureOffset[0], apertureOffset[1]};

        return {position + rayTransform * apertureOffsetPoint, Maths::Normalized(rayTransform * baseVector - apertureOffsetPoint)};
    } else {
        return {position, Maths::Normalized(rayTransform * baseVector)};
    }
}

void Camera::Prepare() {
    const auto w = static_cast<Real>(resolution[0]) / 2.;
    const auto varphi = fovHint / 2.;

    viewingPlaneDistance = w / std::tan(varphi / 180. * M_PI);

    resolutionScale = focalDistance / viewingPlaneDistance;
    scaledResolution = resolution;
    scaledResolution *= resolutionScale;
}

}
