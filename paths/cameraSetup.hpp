#pragma once

#include <memory>
#include <string>

#include <gfx/scene/scene.hpp>
#include <gfx/image.hpp>
#include <gfx/integrator/integrator.hpp>

struct OutConfig {
    enum class FileType: int {
        PNG = 0,
        EXR16F = 1,
        EXR32F = 2,
        EXR32I = 3,
    };

    //sample the integrator nSamples times
    size_t nSamples;

    //per outputEvery samples, output the rendered image. a value of 0 or >nSamples means that it should only be output when sampling is complete
    size_t outputEvery;

    FileType outFileType;
    std::string outFileName;

    std::string resumeFileName;
};

struct PreparedCamera {
    std::shared_ptr<Gfx::Scene> scenePtr{nullptr};
    std::unique_ptr<Gfx::Integrator> integrator{nullptr};
    OutConfig outConfig;
};

[[nodiscard]] extern PreparedCamera PrepareCameraFromJSON(const std::string &filename);

[[nodiscard]] extern std::pair<Gfx::Image, size_t> LoadResume(const std::string &filename);

extern void SaveResume(const std::string &filename, const Gfx::Image &image, size_t nSamples);
