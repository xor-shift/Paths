#pragma once

#include <cmath>

namespace Math {

template<typename T, size_t M, size_t N>
struct Matrix {
    T data[M][N] {};

    static Matrix Rotation(double theta) requires (M == 2 && N == 2) {
        T sinTheta = std::sin(theta),
          cosTheta = std::cos(theta);

        Matrix<T, 2, 2> mat{
          {cosTheta, -sinTheta},
          {sinTheta, cosTheta}
        };

        return mat;
    }

    static Matrix Rotation(double yaw, double pitch, double roll) requires (M == 3 && N == 3) {
        T
          sa = std::sin(yaw),
          ca = std::cos(yaw),
          sb = std::sin(pitch),
          cb = std::cos(pitch),
          sg = std::sin(roll),
          cg = std::cos(roll);

        Matrix<T, 3, 3> mat{{
          {ca * cb, ca * sb * sg - sa * cg, ca * sb * cg + sa * sg},
          {sa * cb, sa * sb * sg + ca * cg, sa * sb * cg - ca * sg},
          {-sb,     cb * sg,                cb * cg},
        }};

        return mat;
    }
};

}
