#include <gfx/image/exporters/exr.hpp>

#include <tinyexr.h>

#include <array>
#include <functional>
#include <vector>

namespace Gfx::Image {

template<std::size_t type>
static inline bool ExportImpl(const std::string &filename, ImageView image) noexcept {
    std::array<std::vector<float>, 3> channels;

    auto ForEachChan = [&channels](auto &&cb) { for (size_t i = 0; auto &c: channels) std::invoke(cb, i++, c); };

    ForEachChan([&](auto i, auto &c) {
        c = std::vector<float>(image.size());
    });

    for (size_t i = 0; i < image.size(); i++) {
        ForEachChan([&](std::size_t j, std::vector<float> &c) {
            c[i] = static_cast<float>(image.data()[i][j]);
        });
    }

    float *channelsPtr[3];
    ForEachChan([&channelsPtr](auto i, auto &c) { channelsPtr[2 - i] = c.data(); });

    EXRHeader exrHeader;
    InitEXRHeader(&exrHeader);

    EXRImage exrImage;
    InitEXRImage(&exrImage);

    exrImage.images = reinterpret_cast<unsigned char **>(channelsPtr);
    exrImage.width = static_cast<int>(image.width);
    exrImage.height = static_cast<int>(image.height);

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

    constexpr int types[] = {
      TINYEXR_PIXELTYPE_HALF,
      TINYEXR_PIXELTYPE_FLOAT,
      TINYEXR_PIXELTYPE_UINT
    };

    for (int i = 0; i < exrHeader.num_channels; i++) {
        exrHeader.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; //narrowed format for Gfx::Image
        exrHeader.requested_pixel_types[i] = types[type];
    }

    const char *err{nullptr};
    if (int res = SaveEXRImageToFile(&exrImage, &exrHeader, filename.c_str(), &err); res != TINYEXR_SUCCESS) {
        //fmt::print("saving EXR image failed with code {}\n", res);
        //if (err) fmt::print("additional information: {}\n", err);
        //else fmt::print("no additional information provided\n");
        return false;
    }

    return true;
}

bool Exporter<EXRExporterF16>::Export(const std::string &filename, ImageView image) { return ExportImpl<0>(filename, image); }

bool Exporter<EXRExporterF32>::Export(const std::string &filename, ImageView image) { return ExportImpl<1>(filename, image); }

bool Exporter<EXRExporterU32>::Export(const std::string &filename, ImageView image) { return ExportImpl<2>(filename, image); }

}