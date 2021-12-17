#include "Paths/Image/Exporters/PNGExporter.hpp"

#include "Maths/Maths.hpp"
#include "Paths/Image/Filter.hpp"

#include "lodepng.h"

namespace Paths::Image {

bool Exporter<PNGExporter>::export_to(const std::string &filename, ImageView image) {
    // min and max magnitude squared
    auto [min_l_sq, max_l_sq] = std::minmax_element(image.cbegin(), image.cend(),
        [](const Color &lhs, const Color &rhs) { return Maths::dot(lhs, lhs) < Maths::dot(rhs, rhs); });

    auto filter
        = Filters::Unary::sequence(Filters::Unary::inv_lerp(Maths::Magnitude(*min_l_sq), Maths::Magnitude(*max_l_sq)),
            Filters::Unary::clamp(0, 1), Filters::Unary::oper([](ColorChannelType v) { return v * 255.; }));

    std::vector<unsigned char> image_data(image.size() * 4);
    for (size_t y = 0; y < image.m_height; y++) {
        for (size_t x = 0; x < image.m_width; x++) {
            const auto &color = image.at(x, y);

            const auto idx = (x + y * image.m_width) * 4;
            image_data[idx + 0] = static_cast<uint8_t>(filter(color[0]));
            image_data[idx + 1] = static_cast<uint8_t>(filter(color[1]));
            image_data[idx + 2] = static_cast<uint8_t>(filter(color[2]));
            image_data[idx + 3] = 255;
        }
    }

    auto res = lodepng::encode(filename, image_data, image.m_width, image.m_height, LCT_RGBA, 8);

    return res == 0;
}

}
