#include "benchmark/benchmark.h"

#include "Maths/Random.hpp"

static thread_local std::random_device s_random_device {};
static thread_local std::linear_congruential_engine<uint_fast64_t, 0x5DEECE66DULL, 11, 1ull << 48> s_lc_engine {
    s_random_device()
};
static thread_local std::default_random_engine s_default_engine { s_random_device() };
static thread_local Maths::Random::LCG s_maths_engine { s_random_device() };

static thread_local std::uniform_real_distribution<double> s_real_dist(0., 1.);
static thread_local Maths::Random::Erand48Gen s_maths_gen {};

static void rand_lc_engine(benchmark::State &state) {
    for (auto _ : state)
        std::ignore = s_real_dist(s_lc_engine);
}

BENCHMARK(rand_lc_engine);

static void rand_def_engine(benchmark::State &state) {
    for (auto _ : state)
        std::ignore = s_real_dist(s_default_engine);
}

BENCHMARK(rand_def_engine);

static void rand_maths_engine(benchmark::State &state) {
    for (auto _ : state)
        std::ignore = s_real_dist(s_maths_engine);
}

BENCHMARK(rand_maths_engine);

static void rand_maths(benchmark::State &state) {
    for (auto _ : state)
        std::ignore = Maths::Random::uniform_normalised();
}

BENCHMARK(rand_maths);

static void rand_maths_manual(benchmark::State &state) {
    for (auto _ : state)
        std::ignore = s_maths_gen(s_maths_engine);
}

BENCHMARK(rand_maths_manual);

static void rand_e_rand_48(benchmark::State &state) {
    thread_local static unsigned short x_i[3] = { 0x330E, 0xFDF3, static_cast<unsigned short>(s_random_device()) };
    for (auto _ : state)
        std::ignore = erand48(x_i);
}

BENCHMARK(rand_e_rand_48);

static void rand_drand48(benchmark::State &state) {
    for (auto _ : state)
        std::ignore = drand48();
}

BENCHMARK(rand_drand48);

BENCHMARK_MAIN();
