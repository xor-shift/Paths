#include <gtest/gtest.h>

TEST(Test, Test) {
    EXPECT_EQ(true, true);
    EXPECT_NE(false, true);
    EXPECT_NE(true, false);
    EXPECT_EQ(false, false);
}