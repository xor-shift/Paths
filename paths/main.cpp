#include <csignal>
#include <fmt/format.h>
#include <tinyexr.h>

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
void OutputPNG(const std::string &filename, const Gfx::Image &image, const T &filter) {
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

void OutputEXR(const std::string &filename, const Gfx::Image &image, OutConfig::FileType type = OutConfig::FileType::EXR16F) {
    std::array<std::vector<float>, 3> channels;

    auto ForEachChan = [&channels](auto &&cb) { for (size_t i = 0; auto &c: channels) std::invoke(cb, i++, c); };

    ForEachChan([&](auto i, auto &c) {
        c = std::vector<float>(image.Width() * image.Height());
    });

    for (size_t i = 0; i < image.Width() * image.Height(); i++) {
        ForEachChan([&](auto j, auto &c) {
            c[i] = static_cast<float>(image.Data()[i][j]);
        });
    }

    float *channelsPtr[3];
    ForEachChan([&channelsPtr](auto i, auto &c) { channelsPtr[i] = c.data(); });

    EXRHeader exrHeader;
    InitEXRHeader(&exrHeader);

    EXRImage exrImage;
    InitEXRImage(&exrImage);

    exrImage.images = reinterpret_cast<unsigned char **>(channelsPtr);
    exrImage.width = static_cast<int>(image.Width());
    exrImage.height = static_cast<int>(image.Height());

    exrHeader.num_channels = 3;

    exrHeader.channels = static_cast<EXRChannelInfo *>(std::malloc(sizeof(EXRChannelInfo) * exrHeader.num_channels));
    exrHeader.channels[0].name[0] = 'B';
    exrHeader.channels[1].name[0] = 'G';
    exrHeader.channels[2].name[0] = 'R';
    exrHeader.channels[0].name[1] = '\0';
    exrHeader.channels[1].name[1] = '\0';
    exrHeader.channels[2].name[1] = '\0';

    exrHeader.pixel_types = static_cast<int *>(std::malloc(sizeof(int) * exrHeader.num_channels));
    exrHeader.requested_pixel_types = static_cast<int *>(std::malloc(sizeof(int) * exrHeader.num_channels));

    int requestedPixelType = 0;
    switch (type) {
        default:
        case OutConfig::FileType::EXR16F:
            requestedPixelType = TINYEXR_PIXELTYPE_HALF;
            break;
        case OutConfig::FileType::EXR32F:
            requestedPixelType = TINYEXR_PIXELTYPE_FLOAT;
            break;
        case OutConfig::FileType::EXR32I:
            requestedPixelType = TINYEXR_PIXELTYPE_UINT;
            break;
    }

    for (int i = 0; i < exrHeader.num_channels; i++) {
        exrHeader.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; //narrowed format for Gfx::Image
        exrHeader.requested_pixel_types[i] = requestedPixelType;
    }

    const char *err{nullptr};
    if (int res = SaveEXRImageToFile(&exrImage, &exrHeader, filename.c_str(), &err); res != TINYEXR_SUCCESS) {
        fmt::print("saving EXR image failed with code {}\n", res);
        if (err) fmt::print("additional information: {}\n", err);
        else fmt::print("no additional information provided\n");
    }

    std::free(exrHeader.channels);
    std::free(exrHeader.pixel_types);
    std::free(exrHeader.requested_pixel_types);
}

template<Gfx::Concepts::BasicFilterUnary T>
void Output(const std::string &filename, OutConfig::FileType type, const Gfx::Image &image, const T &filter) {
    switch (type) {
        case OutConfig::FileType::PNG:
            OutputPNG(filename, image, filter);
            break;
        case OutConfig::FileType::EXR16F:
        case OutConfig::FileType::EXR32F:
        case OutConfig::FileType::EXR32I:
            OutputEXR(filename, image, type);
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
            Output(camera.outConfig.outFileName, camera.outConfig.outFileType, image, Gfx::Filters::Basic::ChainedUnaryFilter{
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
