#pragma once

#include <concepts>
#include <random>
#include <thread>

#include "vector.hpp"
#include "matrix.hpp"
#include "matvec.hpp"

namespace Math {

namespace Concepts {

template<typename T>
concept ODHPSampler = requires(T c) {{ c() } -> std::same_as<double>; };

template<typename T>
concept V2HPSampler = requires(T c) {{ c() } -> std::same_as<Math::Vector<double, 2>>; };

template<typename T>
concept V3HPSampler = requires(T c) {{ c() } -> std::same_as<Math::Vector<double, 3>>; };

}

/**
 * Similar to C's erand48
 * @return Uniformly distributed doubles in the range [0, 1)
 */
inline double RandomDouble() noexcept {
    [[maybe_unused]] auto ImplErand = [] {
        static constexpr uint64_t randMult = 0x0005deece66dll; //posix
        static constexpr uint64_t randAdd = 0xbll; //posix

        thread_local static unsigned short Xi[3] = {
          0x330E /* posix */,
          0xFDF3 /* asked wolframalpha to give me a random number */,
          static_cast<uint16_t>(std::hash<std::thread::id>()(std::this_thread::get_id()))}; /* noise */

        auto Advance = [&] {
            uint64_t t0, t1;
            t0 = static_cast<uint64_t>(Xi[2]) << 32 |
                 static_cast<uint64_t>(Xi[1]) << 16 |
                 static_cast<uint64_t>(Xi[0]);

            t1 = t0 * randMult + randAdd;

            Xi[0] = t1 & 0xFFFF;
            Xi[1] = (t1 >> 16) & 0xFFFF;
            Xi[2] = (t1 >> 32) & 0xFFFF;

            return t1;
        };

        return std::ldexp(static_cast<double>(Advance() & 0xFFFF'FFFF'FFFF), -48);
    };

    return ImplErand();
}

/**
 *
 * @return RandomDouble() + epsilon, resulting in a uniform distribution in the range (0, 1]
 */
inline double RandomDoubleEps() noexcept {
    return RandomDouble() + std::numeric_limits<double>::epsilon();
}

/**
 * Creates uniformly distributed samples in the range 0 <= x < 1, 0 <= y < 1
 * @return
 */
inline Math::Vector<double, 2> SampleSquareUniform() noexcept { return {{RandomDouble(), RandomDouble()}}; }

/**
 * Creates two distinct normally distributed numbers
 * Uses Box-Muller Transform
 * @return The samples
 */
inline Math::Vector<double, 2> SampleNormalBM(double mu = 0, double sigma = 1) noexcept {
    const auto U0 = RandomDoubleEps(), U1 = RandomDoubleEps();

    const auto inner = M_PI * 2. * U1;
    const auto
      rhs0 = std::cos(inner),
      rhs1 = std::sin(inner);
    const auto lhs = sigma * std::sqrt(-2. * std::log(U0));

    return {{lhs * rhs0 + mu, lhs * rhs1 + mu}};
}

inline Math::Vector<double, 2> SampleSNDNormalBM() noexcept { return SampleNormalBM(); }

/**
 * Creates a normally distributed number
 * Uses the Marsaglia polar method
 * Equivalent to std::normal_distribution i believe
 * @return The sample
 */
inline Math::Vector<double, 2> SampleNormalMP(double mu = 0, double sigma = 1) noexcept {
    double x, y, s;

    do {
        x = RandomDoubleEps() * 2. - 1.;
        y = RandomDoubleEps() * 2. - 1.;
        s = x * x + y * y;
    } while (s >= 1. || s == 0.);

    s = std::sqrt(-2. * log(s) / s);
    return {{mu + sigma * x * s, mu + sigma * y * s}};
}

inline Math::Vector<double, 2> SampleSNDNormalMP() noexcept { return SampleNormalMP(); }

inline Math::Vector<double, 3> D3RandomUnitVector() {
    auto ImplUniform = []() {
        double x_1, x_2;

        do {
            x_1 = RandomDouble() * 2. - 1.;
            x_2 = RandomDouble() * 2. - 1.;
        } while (x_1 * x_1 + x_2 * x_2 >= 1);

        const auto rhs = std::sqrt(1. - x_1 * x_1 - x_2 * x_2);
        const auto x = 2. * x_1 * rhs;
        const auto y = 2. * x_2 * rhs;
        const auto z = 1. - 2. * (x_1 * x_1 + x_2 * x_2);

        return Math::Vector<double, 3>{{x, y, z}};
    };

    return ImplUniform();
}

}
