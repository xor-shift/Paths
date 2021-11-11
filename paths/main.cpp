#include <csignal>
#include <fmt/format.h>

#include <gfx/camera/filter.hpp>
#include <gfx/obj/obj.hpp>

#include "cameraSetup.hpp"

void PrintHelp(const char *argv0) {
    fmt::print(
      "Usage:    ./{0} [camera setup file] <resume file>\n"
      "Examples: ./{0} conf.json\n", argv0);
    //"          ./{0} conf.json resume.json", argv0);
}

void Output(const std::string &filename, OutConfig::FileType type, const Gfx::Image &image) {
    switch (type) {
        case OutConfig::FileType::PNG:
            image.ExportPNG(filename, 0, 1);
            break;
        case OutConfig::FileType::EXR16F:
        case OutConfig::FileType::EXR32F:
        case OutConfig::FileType::EXR32I:
            image.ExportEXR(filename, std::to_underlying(type) - 1);
            break;
        default:
            break;
    }
}

static volatile std::sig_atomic_t sigVar = 0;

void SIGHandler(int signum) { sigVar = 1; }

int main(int argc, char *const *argv) {
    std::signal(SIGINT, SIGHandler);
    std::signal(SIGTERM, SIGHandler);

    PreparedCamera camera;

    if (argc != 2) {
        PrintHelp(argv[0]);
        return 1;
    } else {
        camera = PrepareCameraFromJSON(argv[1]);
    }

    for (size_t i = 0; i < camera.outConfig.nSamples; i++) {
        fmt::print("Rendering sample {} out of {}\n", i + 1, camera.outConfig.nSamples);

        camera.integrator->DoRender();

        bool interrupted = sigVar == 1;

        bool output = false;
        output |= (camera.outConfig.outputEvery != 0) &&
                  ((i + 1) % camera.outConfig.outputEvery == 0);
        output |= i + 1 == camera.outConfig.nSamples;
        output |= interrupted;

        if (output) {
            const auto &image = camera.integrator->GetRender();
            Output(camera.outConfig.outFileName, camera.outConfig.outFileType, image);
        }

        if (interrupted) break;
    }

    fmt::print("Saving resume to \"{}\"", camera.outConfig.resumeFileName);
    auto[image, nSamples] = camera.integrator->GetBackbuffer();
    SaveResume(camera.outConfig.resumeFileName, image, nSamples);

    return 0;
}
