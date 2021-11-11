#pragma once

#include "concepts.hpp"
#include "vector.hpp"
#include "matrix.hpp"
#include "matvec.hpp"

namespace Math {

template<typename T = float>
struct Quaternion {
    //T a, b, c, d;
    T data[4];

#define q2qOperAssignEqualsFactory(oper) \
    template<typename U> \
    constexpr Quaternion &operator oper##= (const Quaternion<U> &other) noexcept { \
        for (std::size_t i = 0; i < 4; i++) data[i] oper##= other.data[i]; \
        return *this; \
    }

    q2qOperAssignEqualsFactory(+)

    q2qOperAssignEqualsFactory(-)

#undef q2qOperAssignEqualsFactory

#define q2qOperFactory(oper)
#undef q2qOperFactory

    template<typename U>
    friend constexpr Quaternion operator*(const Quaternion &lhs, const Quaternion<U> &rhs) {
        const auto &[a1, b1, c1, d1] = lhs.data;
        const auto &[a2, b2, c2, d2] = rhs.data;

        return {
          a1 * a2 - b1 * b2 - c1 * c2 - d1 * d2,
          a1 * b2 + b1 * a2 + c1 * d2 - d1 * c2,
          a1 * c2 - b1 * d2 + c1 * a2 + d1 * b2,
          a1 * d2 + b1 * c2 - c1 * b2 + d1 * a2
        };
    }
};

}
