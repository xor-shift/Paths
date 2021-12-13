#include <fmt/format.h>
#include <maths/rand.hpp>
#include <fstream>

struct DoubleHistogram {
    DoubleHistogram(double min, double max, size_t n)
      : min(min)
        , max(max)
        , hist(n, 0)
        , segmentSize((max - min) / static_cast<double>(n))
        , initSegment(RawSegment(min)) {}

    void Push(double n) {
        if (n < min || n > max) return;
        hist[ActualSegment(n)]++;
    }

    double min, max;
    std::vector<size_t> hist;

  private:
    const double segmentSize;
    const ptrdiff_t initSegment;

    [[nodiscard]] ptrdiff_t RawSegment(double v) const { return static_cast<ptrdiff_t>(std::floor(v / segmentSize)); }

    [[nodiscard]] size_t ActualSegment(double v) const { return RawSegment(v) - initSegment; }
};

void PrintHistograms() {
    const size_t nDist = 1024 * 64;

    DoubleHistogram hist0(-2, 2, 32);
    for (size_t i = 0; i < nDist; i++) {
        auto asd = Maths::Random::NormalPair();
        hist0.Push(asd[0]);
        hist0.Push(asd[1]);
    }

    for (const auto &v : hist0.hist) fmt::print("{},", v);
    fmt::print("\n");
}

template<Math::Concepts::V2HPSampler T>
void Sample2(size_t n, std::ofstream &ofs, T &&s) {
    for (size_t i = 0; i < n; i++) {
        auto samp = s();
        ofs << samp[0] << ", " << samp[1] << ", ";
    }
    ofs << std::endl;
}

template<Math::Concepts::V2HPSampler T>
void Bench2(const std::string &name, size_t n, T &&s) {
    auto t0 = std::chrono::system_clock::now();
    for (size_t i = 0; i < n; i++) s();
    auto t1 = std::chrono::system_clock::now();

    auto k = 1000000000.L;
    auto t = static_cast<long double>(std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count());
    fmt::print("Bench2 for {}: {}\n", name, k * static_cast<long double>(n) / t);
}

int main() {
    const size_t nSamples = 1024 * 4;

    const size_t nUniform = nSamples * 64;
    const size_t nNormal = nSamples * 64;
    const size_t nSphere = 1024;

    std::ofstream ofs("dists", std::fstream::out);

    Sample2(nUniform, ofs, &Maths::Random::UnitSquare);
    Sample2(nNormal, ofs, &Maths::Random::NormalPair);

    for (size_t i = 0; i < nSphere; i++) { //this is heavy to display in MPL
        auto samp = Math::D3RandomUnitVector();
        ofs << samp[0] << ", " << samp[1] << ", " << samp[2] << ", ";
    }
    ofs << std::endl;

    ofs.close();

    Bench2("UnitSquare", 1024 * 1024, &Maths::Random::UnitSquare);
    Bench2("NormalPair", 1024 * 1024, &Maths::Random::NormalPair);

    return 0;
}