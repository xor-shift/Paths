#include <gtest/gtest.h>

#include "maths_utils.hpp"

TEST(maths, vecops) {
    Math::Vector<double, 3> v0{{1, 2, 3}};
    Math::Vector<float, 3> v1{{4, 5, 6}};

    auto expr = -(v0 + v1) * M_PI;
    Math::Vector<long double, 3> expected{{-5 * M_PI, -7 * M_PI, -9 * M_PI                                          }};

    EXPECT_TRUE(Equals::Vector(expr, expected));
}

TEST(maths, matops) {}

TEST(maths, matvecops) {}
