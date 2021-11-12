#include <gfx/image/exporters/png.hpp>

#include <gfx/image/filter.hpp>
#include <maths/maths.hpp>

#include <lodepng.h>


namespace Gfx::Image {

bool Exporter<PNGExporter>::Export(const std::string &filename, const Image &image) {
    auto [minp, maxp] = std::minmax_element(image.cbegin(), image.cend(), [](const Color &lhs, const Color &rhs) {
        return Maths::Dot(lhs, lhs) < Maths::Dot(rhs, rhs);
    });

    auto filter = Gfx::Image::Filters::Unary::Sequence(
        Gfx::Image::Filters::Unary::InvLerp(Maths::Magnitude(*minp), Maths::Magnitude(*maxp)),
        Gfx::Image::Filters::Unary::Clamp(0, 1),
        Gfx::Image::Filters::Unary::Oper([](ChannelType v) { return v * 255.; }));

    std::vector<unsigned char> imageData(image.size() * 4);
    for (size_t y = 0; y < image.height; y++) {
        for (size_t x = 0; x < image.width; x++) {
            const auto &color = image.At(x, y);

            const auto idx = (x + y * image.width) * 4;
            imageData[idx + 0] = static_cast<uint8_t>(filter(color[0]));
            imageData[idx + 1] = static_cast<uint8_t>(filter(color[1]));
            imageData[idx + 2] = static_cast<uint8_t>(filter(color[2]));
            imageData[idx + 3] = 255;
        }
    }

    auto res = lodepng::encode(filename, imageData, image.width, image.height, LCT_RGBA, 8);

    return res == 0;
}

}