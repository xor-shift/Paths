#include <gtest/gtest.h>

#include "Maths/Random.hpp"
#include "maths_utils.hpp"

static thread_local std::random_device s_random_device {};
static thread_local Maths::Random::LCG s_maths_engine { s_random_device() };
static thread_local Maths::Random::Erand48Gen s_maths_gen {};

TEST(maths, prng) {
    double asd = 0.;

    for (std::size_t i = 0; i < 5000; i++) {
        if (i % 50 == 0)
            fmt::print("{}, {}\n", i / 2, asd);
        const auto temp = s_maths_gen(s_maths_engine);
        asd += temp;
    }

    fmt::print("{}\n", asd);
}
