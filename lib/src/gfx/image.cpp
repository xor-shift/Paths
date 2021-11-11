#include "gfx/image.hpp"

#include <tinyexr.h>
#include <lodepng.h>

#include <gfx/camera/filter.hpp>

namespace Gfx {

void Image::ExportPNG(const std::string &filename, Real min, Real max) const {
    auto filter = Gfx::Filters::Basic::ChainedUnaryFilter{
      Gfx::Filters::Basic::InvLERP(min, max),
      Gfx::Filters::Basic::Clamp{0, 1},
      Gfx::Filters::Basic::Oper([](Gfx::RGBSpectrum c) { return c * 255.; }),
    };

    std::vector<unsigned char> imageData(Width() * Height() * 4);
    for (size_t y = 0; y < Height(); y++) {
        for (size_t x = 0; x < Width(); x++) {
            const auto color = filter(At(x, y));

            const auto idx = (x + y * Width()) * 4;
            imageData[idx + 0] = static_cast<uint8_t>(color[0]);
            imageData[idx + 1] = static_cast<uint8_t>(color[1]);
            imageData[idx + 2] = static_cast<uint8_t>(color[2]);
            imageData[idx + 3] = 255;
        }
    }

    lodepng::encode(filename, imageData, Width(), Height(), LCT_RGBA, 8);
}

void Image::ExportEXR(const std::string &filename, size_t type) const {
    std::array<std::vector<float>, 3> channels;

    auto ForEachChan = [&channels](auto &&cb) { for (size_t i = 0; auto &c: channels) std::invoke(cb, i++, c); };

    ForEachChan([&](auto i, auto &c) {
        c = std::vector<float>(Width() * Height());
    });

    for (size_t i = 0; i < Width() * Height(); i++) {
        ForEachChan([&](auto j, auto &c) {
            c[i] = static_cast<float>(Data()[i][j]);
        });
    }

    float *channelsPtr[3];
    ForEachChan([&channelsPtr](auto i, auto &c) { channelsPtr[2 - i] = c.data(); });

    EXRHeader exrHeader;
    InitEXRHeader(&exrHeader);

    EXRImage exrImage;
    InitEXRImage(&exrImage);

    exrImage.images = reinterpret_cast<unsigned char **>(channelsPtr);
    exrImage.width = static_cast<int>(Width());
    exrImage.height = static_cast<int>(Height());

    exrHeader.num_channels = 3;

    std::vector<EXRChannelInfo> headerChannels(exrHeader.num_channels);
    exrHeader.channels = headerChannels.data();
    exrHeader.channels[0].name[0] = 'B';
    exrHeader.channels[1].name[0] = 'G';
    exrHeader.channels[2].name[0] = 'R';
    exrHeader.channels[0].name[1] = '\0';
    exrHeader.channels[1].name[1] = '\0';
    exrHeader.channels[2].name[1] = '\0';

    std::vector<int> headerPixelTypes(exrHeader.num_channels);
    exrHeader.pixel_types = headerPixelTypes.data();
    std::vector<int> headerRequestedPixelTypes(exrHeader.num_channels);
    exrHeader.requested_pixel_types = headerRequestedPixelTypes.data();

    constexpr int typeCorrespondence[] = {
      TINYEXR_PIXELTYPE_HALF,
      TINYEXR_PIXELTYPE_FLOAT,
      TINYEXR_PIXELTYPE_UINT
    };

    for (int i = 0; i < exrHeader.num_channels; i++) {
        exrHeader.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; //narrowed format for Gfx::Image
        exrHeader.requested_pixel_types[i] = typeCorrespondence[std::clamp<size_t>(type, 0, 2)];
    }

    const char *err{nullptr};
    if (int res = SaveEXRImageToFile(&exrImage, &exrHeader, filename.c_str(), &err); res != TINYEXR_SUCCESS) {
        fmt::print("saving EXR image failed with code {}\n", res);
        if (err) fmt::print("additional information: {}\n", err);
        else fmt::print("no additional information provided\n");
    }
}

}
