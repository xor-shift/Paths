#include <gtest/gtest.h>

#include "maths_utils.hpp"
#include <maths/rand.hpp>

static thread_local std::random_device randomDevice{};
static thread_local Maths::Random::LCG mathsEngine{randomDevice()};
static thread_local Maths::Random::Erand48Gen mathsGen{};

TEST(maths, prng) {
    double asd = 0.;

    for (std::size_t i = 0; i < 5000; i++) {
        if (i % 50 == 0) fmt::print("{}, {}\n", i / 2, asd);
        const auto temp = mathsGen(mathsEngine);
        asd += temp;
    }

    fmt::print("{}\n", asd);
}
