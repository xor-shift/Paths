#include <gtest/gtest.h>

#include "maths_utils.hpp"

TEST(maths, vecops) {
    Maths::Vector<double, 3> v0{{1, 2, 3}};
    Maths::Vector<float, 3> v1{{4, 5, 6}};

    auto expr = -(v0 + v1 + v0) * M_PI;
    Maths::Vector<long double, 3> expected{{-6 * M_PI, -9 * M_PI, -12 * M_PI}};

    EXPECT_TRUE(Equals::Vector(expr, expected));
}

TEST(maths, matops) {}

TEST(maths, matvecops) {}
