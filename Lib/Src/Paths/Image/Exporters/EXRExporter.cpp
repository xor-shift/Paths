#include "Paths/Image/Exporters/EXRExporter.hpp"

#include "tinyexr.h"

#include <array>
#include <functional>
#include <vector>

namespace Paths::Image {

template<typename Callback> static void for_each_channel(std::array<std::vector<float>, 3> &channels, Callback &&cb) {
    for (std::size_t i = 0; auto &channel : channels)
        std::invoke(cb, i++, channel);
}

template<std::size_t type> static inline bool export_impl(const std::string &filename, ImageView image) noexcept {
    std::array<std::vector<float>, 3> channels;

    for_each_channel(channels, [&](auto, auto &c) { c = std::vector<float>(image.size()); });

    for (size_t i = 0; i < image.size(); i++) {
        for_each_channel(
            channels, [&](std::size_t j, std::vector<float> &c) { c[i] = static_cast<float>(image.data()[i][j]); });
    }

    float *channels_ptr[3];
    for_each_channel(channels, [&channels_ptr](auto i, auto &c) { channels_ptr[2 - i] = c.data(); });

    EXRHeader exr_header;
    InitEXRHeader(&exr_header);

    EXRImage exr_image;
    InitEXRImage(&exr_image);

    exr_image.images = reinterpret_cast<unsigned char **>(channels_ptr);
    exr_image.width = static_cast<int>(image.m_width);
    exr_image.height = static_cast<int>(image.m_height);

    exr_header.num_channels = 3;

    std::vector<EXRChannelInfo> header_channels(exr_header.num_channels);
    exr_header.channels = header_channels.data();
    exr_header.channels[0].name[0] = 'B';
    exr_header.channels[1].name[0] = 'G';
    exr_header.channels[2].name[0] = 'R';
    exr_header.channels[0].name[1] = '\0';
    exr_header.channels[1].name[1] = '\0';
    exr_header.channels[2].name[1] = '\0';

    std::vector<int> header_pixel_types(exr_header.num_channels);
    exr_header.pixel_types = header_pixel_types.data();
    std::vector<int> header_requested_pixel_types(exr_header.num_channels);
    exr_header.requested_pixel_types = header_requested_pixel_types.data();

    constexpr int types[] = { TINYEXR_PIXELTYPE_HALF, TINYEXR_PIXELTYPE_FLOAT, TINYEXR_PIXELTYPE_UINT };

    for (int i = 0; i < exr_header.num_channels; i++) {
        exr_header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // narrowed format for Gfx::Image
        exr_header.requested_pixel_types[i] = types[type];
    }

    const char *err { nullptr };
    const int res = SaveEXRImageToFile(&exr_image, &exr_header, filename.c_str(), &err);

    return res == TINYEXR_SUCCESS;

    /*if (res != TINYEXR_SUCCESS) {
        // fmt::print("saving EXR image failed with code {}\n", res);
        // if (err) fmt::print("additional information: {}\n", err);
        // else fmt::print("no additional information provided\n");
        return false;
    }

    return true;*/
}

bool Exporter<EXRExporterF16>::export_to(const std::string &filename, ImageView image) {
    return export_impl<0>(filename, image);
}

bool Exporter<EXRExporterF32>::export_to(const std::string &filename, ImageView image) {
    return export_impl<1>(filename, image);
}

bool Exporter<EXRExporterU32>::export_to(const std::string &filename, ImageView image) {
    return export_impl<2>(filename, image);
}

}
