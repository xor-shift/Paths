#pragma once

#include <execution>
#include <numeric>
#include "gcem.hpp"

#include "definitions.hpp"

namespace Gfx {

namespace Impl {

constexpr Real spectrumStart = 390, spectrumEnd = 710;

//range is 390..710, 320nm wide
//64 samples -> a sample every 5nm
static constexpr std::size_t nSpectrumSamples = 64;

}

typedef Math::Vector<Real, Impl::nSpectrumSamples> Spectrum;
typedef Math::Vector<Real, 3> RGBSpectrum;

namespace Impl {

constexpr Real SpectrumGaussian(Real lambda, Real lhp, Real rhp) {
    return gcem::pow<Real, Real>(static_cast<Real>(M_E), -(
        (lambda - (rhp + lhp) / 2) /
        ((rhp - lhp) / 2)
    ));
}

template<typename Fn>
constexpr Spectrum SpectrumFromLambdaUnary(Fn &&cb) requires std::is_same_v<std::invoke_result_t<Fn, Real>, Real> {
    Spectrum s{};

    Real d = (spectrumEnd - spectrumStart) / (static_cast<Real>(Impl::nSpectrumSamples));
    for (std::size_t i = 0; i < nSpectrumSamples; i++) {
        Real l = spectrumStart + d * static_cast<Real>(i);
        s[i] = std::invoke(cb, l);
    }

    return s;
}

//peak at 575
constexpr Real SpectrumGaussianRed(Real lambda) { return SpectrumGaussian(lambda, 520, 630); }

//peak at 545
constexpr Real SpectrumGaussianGreen(Real lambda) { return SpectrumGaussian(lambda, 500, 590); }

//peak at 445
constexpr Real SpectrumGaussianBlue(Real lambda) { return SpectrumGaussian(lambda, 410, 480); }


constexpr const std::array<const Spectrum, 3> colorWeights {
    SpectrumFromLambdaUnary(SpectrumGaussianRed),
    SpectrumFromLambdaUnary(SpectrumGaussianGreen),
    SpectrumFromLambdaUnary(SpectrumGaussianBlue)};

constexpr static const Spectrum &redWeights = colorWeights[0];
constexpr static const Spectrum &greenWeights = colorWeights[1];
constexpr static const Spectrum &blueWeights = colorWeights[2];

constexpr const auto spectrumReductionPolicy = []() {
    if constexpr (nSpectrumSamples >= 32) return std::execution::par;
    else return std::execution::seq;
}();

}

constexpr RGBSpectrum SpectrumToRGB(const Spectrum &spectrum) {
    RGBSpectrum ret{{0, 0, 0}};

    for (std::size_t i = 0; i < Impl::colorWeights.size(); i++) {
        const Spectrum tempSpectrum = spectrum * Impl::colorWeights[i];
        ret[i] = std::reduce(Impl::spectrumReductionPolicy, tempSpectrum.data(), tempSpectrum.data(), static_cast<Real>(1), std::multiplies<>());
    }

    return ret;
}

constexpr Spectrum RGBToSpectrum([[maybe_unused]] const RGBSpectrum &rgb) { return {}; }

}
