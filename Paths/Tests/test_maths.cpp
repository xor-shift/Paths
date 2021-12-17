#include <gtest/gtest.h>

#include "maths_utils.hpp"

TEST(maths, vecops) {
    Maths::Vector<double, 3> v_0 { 1, 2, 3 };
    Maths::Vector<float, 3> v_1 { 4, 5, 6 };

    auto expr = -(v_0 + v_1 + v_0) * M_PI;
    Maths::Vector<long double, 3> expected { -6 * M_PI, -9 * M_PI, -12 * M_PI };

    EXPECT_TRUE(Equals::vector_eq(expr, expected));
}

TEST(maths, matops) { }

TEST(maths, matvecops) { }
