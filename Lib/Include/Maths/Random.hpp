#pragma once

#include <random>

#include "Vector.hpp"

namespace Maths::Random::Conv {

namespace Detail {

// marsaglia polar method
static inline Maths::Vector<double, 2> to_normal_mp(Maths::Vector<double, 2> sample) {
    double x, y, s;

    do {
        x = sample[0] * 2. - 1.;
        y = sample[1] * 2. - 1.;
        s = x * x + y * y;
    } while (s >= 1. || s == 0.);

    s = std::sqrt(-2. * log(s) / s);
    return { x * s, y * s };
}

// box-muller transform
static inline Maths::Vector<double, 2> to_normal_bm(Maths::Vector<double, 2> sample) {
    const auto u_0 = sample[0], u_1 = sample[1];

    const auto inner = M_PI * 2. * u_1;
    const auto rhs_0 = std::cos(inner), rhs_1 = std::sin(inner);
    const auto lhs = std::sqrt(-2. * std::log(u_0));

    return { lhs * rhs_0, lhs * rhs_1 };
}

}

/// Converts a pair of uniform samples in the range (0., 1.] into a pair of normally distributed samples with mu=0,
/// sigma=1 \param sample \return
static inline Maths::Vector<double, 2> to_normal(Maths::Vector<double, 2> sample) {
    return Detail::to_normal_mp(sample);
}

}

namespace Maths::Random {

/// A linear congruential generator without modulo
/// \tparam a
/// \tparam c
/// \tparam m The value to **and** with, not a modulo
/// \tparam offset
template<uint64_t a = 0x5deece66dull, uint64_t c = 11, uint64_t m = (1ull << 48) - 1ull> struct LCG {
    using result_type = uint64_t;

    LCG() noexcept = default;

    explicit LCG(result_type seed) noexcept
        : m_state_new(seed) { }

    template<typename Gen> [[deprecated("not yet implemented")]] explicit LCG(Gen &gen) noexcept(noexcept(gen())) {
        std::abort();
    }

    constexpr void seed(result_type v) noexcept { m_state_new = v & m; }

    [[nodiscard("call discard() to discard values")]] constexpr result_type operator()() noexcept {
        return (m_state_new = advance_new(m_state_new));
    }

    constexpr void discard() noexcept { m_state_new = advance_new(m_state_new); }

    constexpr result_type max() const noexcept { return m; }

    constexpr result_type min() const noexcept { return 0; }

private:
    static const constexpr uint64_t m_modulo_mask = (1ull << 48) - 1ull;

    uint64_t m_state_new = 0x330E;

    constexpr static uint64_t advance_impl(std::array<uint16_t, 3> &x_i) noexcept {
        uint64_t x_next
            = static_cast<uint64_t>(x_i[2]) << 32 | static_cast<uint64_t>(x_i[1]) << 16 | static_cast<uint64_t>(x_i[0]);

        x_next *= a;
        x_next += c;

        x_i[0] = x_next & 0xFFFF;
        x_i[1] = (x_next >> 16) & 0xFFFF;
        x_i[2] = (x_next >> 32) & 0xFFFF;

        return x_next;
    }

    constexpr static uint64_t advance_new(uint64_t state) noexcept { return (state * a + c) & m; }
};

struct Erand48Gen {
    using result_type = double;

    constexpr result_type min() const noexcept { return 0.; }

    constexpr result_type max() const noexcept { return 1.; }

    template<typename Engine> result_type operator()(Engine &engine) const noexcept {
        constexpr const auto num_bits = sizeof(typename Engine::result_type) * 8;
        static_assert(num_bits == 64);

        // return std::ldexp(static_cast<double>(engine() & 0xFFFF'FFFF'FFFF), -48);
        return static_cast<double>(engine() & 0xFFFF'FFFF'FFFF) * std::pow<double>(2, -48);
    }
};

namespace Detail {

static std::random_device s_random_device {};
static thread_local LCG s_default_engine { s_random_device() };
static Erand48Gen s_default_uniform_generator {};

}

static inline double uniform_normalised() { return Detail::s_default_uniform_generator(Detail::s_default_engine); }

static inline Maths::Vector<double, 2> unit_square() { return { uniform_normalised(), uniform_normalised() }; }

namespace Detail {

inline Maths::Vector<double, 2> rejection_sampled_unit_disk() {
    Maths::Vector<double, 2> sample;

    do {
        sample = unit_square();
    } while (sample[0] * sample[0] + sample[1] * sample[1] >= 1);

    return sample;
}

}

static inline Maths::Vector<double, 2> unit_disk() { return Detail::rejection_sampled_unit_disk(); }

static inline Maths::Vector<double, 2> normal_pair() { return Conv::to_normal(unit_square()); }

static inline double normal() {
    static thread_local bool b = true;
    static thread_local Maths::Vector<double, 2> mem { normal_pair() };

    const auto ret = mem[!b];
    if (!b)
        mem = normal_pair();
    return ret;
}

static inline Maths::Vector<double, 3> unit_vector() {
    double x_1, x_2;

    do {
        x_1 = uniform_normalised() * 2. - 1.;
        x_2 = uniform_normalised() * 2. - 1.;
    } while (x_1 * x_1 + x_2 * x_2 >= 1);

    const auto rhs = std::sqrt(1. - x_1 * x_1 - x_2 * x_2);
    const auto x = 2. * x_1 * rhs;
    const auto y = 2. * x_2 * rhs;
    const auto z = 1. - 2. * (x_1 * x_1 + x_2 * x_2);

    return { x, y, z };
}

}
