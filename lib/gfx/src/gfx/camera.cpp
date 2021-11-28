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

}