#include <benchmark/benchmark.h>

#include <maths/rand.hpp>

static thread_local std::random_device randomDevice{};
static thread_local std::linear_congruential_engine<uint_fast64_t, 0x5DEECE66DULL, 11, 1ull << 48> lcEngine{randomDevice()};
static thread_local std::default_random_engine defaultEngine{randomDevice()};
static thread_local Maths::Random::LCG mathsEngine{randomDevice()};

static thread_local std::uniform_real_distribution<double> realDist(0., 1.);
static thread_local Maths::Random::Erand48Gen mathsGen{};

static void Rand_LCEngine(benchmark::State &state) {
    for (auto _: state) std::ignore = realDist(lcEngine);
}

BENCHMARK(Rand_LCEngine);

static void Rand_DefEngine(benchmark::State &state) {
    for (auto _: state) std::ignore = realDist(defaultEngine);
}

BENCHMARK(Rand_DefEngine);

static void Rand_MathsEngine(benchmark::State &state) {
    for (auto _: state) std::ignore = realDist(mathsEngine);
}

BENCHMARK(Rand_MathsEngine);

static void Rand_Maths(benchmark::State &state) {
    for (auto _: state) std::ignore = Maths::Random::UniformNormalised();
}

BENCHMARK(Rand_Maths);

static void Rand_MathsManual(benchmark::State &state) {
    for (auto _: state) std::ignore = mathsGen(mathsEngine);
}

BENCHMARK(Rand_MathsManual);

static void Rand_ERand48(benchmark::State &state) {
    thread_local static unsigned short Xi[3] = {0x330E, 0xFDF3, static_cast<unsigned short>(randomDevice())};
    for (auto _: state) std::ignore = erand48(Xi);
}

BENCHMARK(Rand_ERand48);

static void Rand_DRand48(benchmark::State &state) {
    for (auto _: state) std::ignore = drand48();
}

BENCHMARK(Rand_DRand48);

BENCHMARK_MAIN();
