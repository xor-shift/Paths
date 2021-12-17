#include <chrono>
#include <fmt/format.h>
#include <fstream>

#include "Maths/Random.hpp"

struct DoubleHistogram {
    DoubleHistogram(double min, double max, size_t n)
        : m_min(min)
        , m_max(max)
        , m_hist(n, 0)
        , m_segment_size((max - min) / static_cast<double>(n))
        , m_init_segment(raw_segment(min)) { }

    void push(double n) {
        if (n < m_min || n > m_max)
            return;
        m_hist[actual_segment(n)]++;
    }

    double m_min, m_max;
    std::vector<size_t> m_hist;

private:
    const double m_segment_size;
    const ptrdiff_t m_init_segment;

    [[nodiscard]] ptrdiff_t raw_segment(double v) const {
        return static_cast<ptrdiff_t>(std::floor(v / m_segment_size));
    }

    [[nodiscard]] size_t actual_segment(double v) const { return raw_segment(v) - m_init_segment; }
};

void print_histograms() {
    const size_t n_dist = 1024 * 64;

    DoubleHistogram hist_0(-2, 2, 32);
    for (size_t i = 0; i < n_dist; i++) {
        auto asd = Maths::Random::normal_pair();
        hist_0.push(asd[0]);
        hist_0.push(asd[1]);
    }

    for (const auto &v : hist_0.m_hist)
        fmt::print("{},", v);
    fmt::print("\n");
}

template<typename T> void sample_2(size_t n, std::ofstream &ofs, T &&s) {
    for (size_t i = 0; i < n; i++) {
        auto samp = s();
        ofs << samp[0] << ", " << samp[1] << ", ";
    }
    ofs << std::endl;
}

template<typename T> void bench_2(const std::string &name, size_t n, T &&s) {
    auto t_0 = std::chrono::system_clock::now();
    for (size_t i = 0; i < n; i++)
        s();
    auto t_1 = std::chrono::system_clock::now();

    auto k = 1000000000.L;
    auto t = static_cast<long double>(std::chrono::duration_cast<std::chrono::nanoseconds>(t_1 - t_0).count());
    fmt::print("bench_2 for {}: {}\n", name, k * static_cast<long double>(n) / t);
}

int main() {
    const size_t n_samples = 1024 * 4;

    const size_t n_uniform = n_samples * 64;
    const size_t n_normal = n_samples * 64;
    const size_t n_sphere = 1024;

    std::ofstream ofs("dists", std::fstream::out);

    sample_2(n_uniform, ofs, &Maths::Random::unit_square);
    sample_2(n_normal, ofs, &Maths::Random::normal_pair);

    for (size_t i = 0; i < n_sphere; i++) { // this is heavy to display in MPL
        auto samp = Maths::Random::unit_vector();
        ofs << samp[0] << ", " << samp[1] << ", " << samp[2] << ", ";
    }
    ofs << std::endl;

    ofs.close();

    bench_2("unit_square", 1024 * 1024, &Maths::Random::unit_square);
    bench_2("normal_pair", 1024 * 1024, &Maths::Random::normal_pair);

    return 0;
}