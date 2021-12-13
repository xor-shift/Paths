#pragma once

#include <random>

#include "vector.hpp"

namespace Maths::Random::Conv {

namespace Detail {

//marsaglia polar method
static inline Maths::Vector<double, 2> ToNormalMP(Maths::Vector<double, 2> sample) {
    double x, y, s;

    do {
        x = sample[0] * 2. - 1.;
        y = sample[1] * 2. - 1.;
        s = x * x + y * y;
    } while (s >= 1. || s == 0.);

    s = std::sqrt(-2. * log(s) / s);
    return {x * s, y * s};
}

//box-muller transform
static inline Maths::Vector<double, 2> ToNormalBM(Maths::Vector<double, 2> sample) {
    const auto U0 = sample[0], U1 = sample[1];

    const auto inner = M_PI * 2. * U1;
    const auto
      rhs0 = std::cos(inner),
      rhs1 = std::sin(inner);
    const auto lhs = std::sqrt(-2. * std::log(U0));

    return {lhs * rhs0, lhs * rhs1};
}

}

/// Converts a pair of uniform samples in the range (0., 1.] into a pair of normally distributed samples with mu=0, sigma=1
/// \param sample
/// \return
static inline Maths::Vector<double, 2> ToNormal(Maths::Vector<double, 2> sample) { return Detail::ToNormalMP(sample); }

}

namespace Maths::Random {


/// A linear congruential generator without modulo
/// \tparam a
/// \tparam c
/// \tparam m The value to **and** with, not a modulo
/// \tparam offset
template<uint64_t a = 0x5deece66dull, uint64_t c = 11, uint64_t m = (1ull << 48) - 1ull>
struct LCG {
    using result_type = uint64_t;

    LCG() noexcept = default;

    explicit LCG(result_type seed) noexcept
      : stateNew(seed) {}

    template<typename Gen>
    [[deprecated("not yet implemented")]] explicit LCG(Gen &gen) noexcept(noexcept(gen())) { std::abort(); }

    constexpr void seed(result_type v) noexcept { stateNew = v & m; }

    [[nodiscard("call discard() to discard values")]] constexpr result_type operator()() noexcept { return (stateNew = AdvanceNew(stateNew)); }

    constexpr void discard() noexcept { stateNew = AdvanceNew(stateNew); }

    constexpr result_type max() const noexcept { return m; }

    constexpr result_type min() const noexcept { return 0; }

  private:
    static const constexpr uint64_t moduloMask = (1ull << 48) - 1ull;

    uint64_t stateNew = 0x330E;

    constexpr static uint64_t AdvanceImpl(std::array<uint16_t, 3> &Xi) noexcept {
        uint64_t xNext = static_cast<uint64_t>(Xi[2]) << 32 |
                         static_cast<uint64_t>(Xi[1]) << 16 |
                         static_cast<uint64_t>(Xi[0]);

        xNext *= a;
        xNext += c;

        Xi[0] = xNext & 0xFFFF;
        Xi[1] = (xNext >> 16) & 0xFFFF;
        Xi[2] = (xNext >> 32) & 0xFFFF;

        return xNext;
    }

    constexpr static uint64_t AdvanceNew(uint64_t state) noexcept {
        return (state * a + c) & m;
    }
};

struct Erand48Gen {
    using result_type = double;

    constexpr result_type min() const noexcept { return 0.; }

    constexpr result_type max() const noexcept { return 1.; }

    template<typename Engine>
    result_type operator()(Engine &engine) const noexcept {
        constexpr const auto numBits = sizeof(typename Engine::result_type) * 8;
        static_assert(numBits == 64);

        return std::ldexp(static_cast<double>(engine() & 0xFFFF'FFFF'FFFF), -48);
    }
};

namespace Detail {

static thread_local std::random_device rd{};
static thread_local LCG defaultEngine{rd()};
static thread_local Erand48Gen defaultNormalGenerator{};

}

static inline double UniformNormalised() { return Detail::defaultNormalGenerator(Detail::defaultEngine); }

static inline Maths::Vector<double, 2> UnitSquare() {
    return {UniformNormalised(), UniformNormalised()};
}

namespace Detail {

inline Maths::Vector<double, 2> RejectionSampledUnitDisk() {
    Maths::Vector<double, 2> sample;

    do {
        sample = UnitSquare();
    } while (sample[0] * sample[0] + sample[1] * sample[1] >= 1);

    return sample;
}

}

static inline Maths::Vector<double, 2> UnitDisk() { return Detail::RejectionSampledUnitDisk(); }

static inline Maths::Vector<double, 2> NormalPair() { return Conv::ToNormal(UnitSquare()); }

static inline double Normal() {
    static thread_local bool b = true;
    static thread_local Maths::Vector<double, 2> mem{NormalPair()};

    const auto ret = mem[!b];
    if (!b) mem = NormalPair();
    return ret;
}

static inline Maths::Vector<double, 3> UnitVector() {
    double x_1, x_2;

    do {
        x_1 = UniformNormalised() * 2. - 1.;
        x_2 = UniformNormalised() * 2. - 1.;
    } while (x_1 * x_1 + x_2 * x_2 >= 1);

    const auto rhs = std::sqrt(1. - x_1 * x_1 - x_2 * x_2);
    const auto x = 2. * x_1 * rhs;
    const auto y = 2. * x_2 * rhs;
    const auto z = 1. - 2. * (x_1 * x_1 + x_2 * x_2);

    return {x, y, z};
}

}
