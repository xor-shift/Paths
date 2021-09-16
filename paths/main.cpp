#include <csignal>
#include <fmt/format.h>

#include <gfx/camera/filter.hpp>

#include "cameraSetup.hpp"
#include "lodepng.h"

void PrintHelp(const char *argv0) {
    fmt::print(
      "Usage:    ./{0} [camera setup file] <resume file>\n"
      "Examples: ./{0} conf.json\n", argv0);
      //"          ./{0} conf.json resume.json", argv0);
}

template<Gfx::Concepts::BasicFilterUnary T>
void Output(const std::string &filename, const Gfx::Image &image, const T &filter) {
    std::vector<unsigned char> imageData(image.Width() * image.Height() * 4);
    for (size_t y = 0; y < image.Height(); y++) {
        for (size_t x = 0; x < image.Width(); x++) {
            const auto color = filter(image.At(x, y));

            const auto idx = (x + y * image.Width()) * 4;
            imageData[idx + 0] = static_cast<uint8_t>(color[0]);
            imageData[idx + 1] = static_cast<uint8_t>(color[1]);
            imageData[idx + 2] = static_cast<uint8_t>(color[2]);
            imageData[idx + 3] = 255;
        }
    }

    lodepng::encode(filename, imageData, image.Width(), image.Height(), LCT_RGBA, 8);
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
            Output(camera.outConfig.outFileName, image, Gfx::Filters::Basic::ChainedUnaryFilter{
              Gfx::Filters::Basic::InvLERP(0, 1),
              Gfx::Filters::Basic::Clamp{0, 1},
              Gfx::Filters::Basic::Oper([](Gfx::RGBSpectrum c) { return c * 255.; }),
            });
        }

        if (interrupted) break;
    }

    fmt::print("Saving resume to \"{}\"", camera.outConfig.resumeFileName);
    auto[image, nSamples] = camera.integrator->GetBackbuffer();
    SaveResume(camera.outConfig.resumeFileName, image, nSamples);

    return 0;
}
