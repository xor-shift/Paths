#pragma once

#include <gfx/definitions.hpp>
#include <gfx/spectrum.hpp>
#include <gfx/image.hpp>

namespace Gfx {

namespace Concepts {

template<typename T>
concept BasicFilterUnary = requires(const T &f, RGBSpectrum s) {
    { f(s) } -> std::convertible_to<RGBSpectrum>;
};

template<typename T>
concept BasicFilterBinary = requires(const T &f, RGBSpectrum s0, RGBSpectrum s1) {
    { f(s0, s1) } -> std::convertible_to<RGBSpectrum>;
};

template<typename T, size_t N>
concept BasicFilterNary = requires(const T &f, std::array<RGBSpectrum, N> s) {
    { f < N > (s) } -> std::convertible_to<RGBSpectrum>;
};

}

namespace Filters::Basic {

struct InvLERP {
    InvLERP(Real a, Real b)
      : r(1. / (b - a))
        , k(-a * r) {}

    RGBSpectrum operator()(RGBSpectrum s) const noexcept {
        for (auto &v : s) v = v * r + k;
        return s;
    }

    //lerp(t) = a+t(b-a) = v
    //lerp^-1(v) = (v-a)/(b-a) = t
    //1/(b-a) = r; -ar = k
    //t = vr+k, single FMA instruction :^)
    const Real r, k;
};

struct Clamp {
    const Real min, max;

    RGBSpectrum operator()(RGBSpectrum s) const noexcept {
        for (auto &v : s) v = std::clamp(v, min, max);
        return s;
    }
};

template<Concepts::BasicFilterUnary... Ts>
struct ChainedUnaryFilter {
    typedef std::tuple<Ts...> TupleType;

    explicit ChainedUnaryFilter(TupleType &&fs)
      : fs(std::forward<TupleType>(fs)) {}

    ChainedUnaryFilter(Ts &&...fs)
      : fs(std::make_tuple(std::forward<Ts>(fs)...)) {}

    RGBSpectrum operator()(RGBSpectrum s) const noexcept { return std::apply([s](auto &&...f) mutable -> RGBSpectrum { return ((s = f(s)), ...); }, fs); }

  private:
    const TupleType fs;
};

}


}
