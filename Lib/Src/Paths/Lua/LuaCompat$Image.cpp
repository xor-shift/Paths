#include "Paths/Lua/LuaCompat.hpp"

#include "Paths/Image/Image.hpp"

#include "Paths/Image/Exporters/EXRExporter.hpp"
#include "Paths/Image/Exporters/PNGExporter.hpp"

namespace Paths::Lua::Detail {

extern void add_image_view_to_lua(sol::state &state) {
    auto image_view_compat = state.new_usertype<Paths::Image::ImageView>("imageView", sol::no_constructor);

    image_view_compat["export"] = [](Paths::Image::ImageView self, const std::string &file, const std::string &type) {
        if (type == "exrf16")
            return Paths::Image::Exporter<Paths::Image::EXRExporterF16>::export_to(file, self);
        if (type == "exrf32")
            return Paths::Image::Exporter<Paths::Image::EXRExporterF32>::export_to(file, self);
        if (type == "png")
            return Paths::Image::Exporter<Paths::Image::PNGExporter>::export_to(file, self);

        return false;
    };

    image_view_compat["getAt"]
        = [](const Paths::Image::ImageView &self, std::size_t row, std::size_t col) { return self.at(col, row); };
}

extern void add_image_to_lua(sol::state &state) {
    typedef Paths::Image::Image<> self_t;

    const auto image_constructors = sol::constructors<Paths::Image::Image<>(),
        Paths::Image::Image<>(std::size_t, std::size_t), Paths::Image::Image<>(Paths::Image::ImageView)>();
    auto image_compat = state.new_usertype<Paths::Image::Image<>>("image", image_constructors);

    image_compat[sol::meta_function::length] = [](const self_t &self) { return self.size(); };

    image_compat[sol::meta_function::index]
        = [](const self_t &self, std::size_t index) { return self.m_impl[index - 1]; };

    image_compat[sol::meta_function::new_index]
        = [](self_t &self, std::size_t index, self_t::value_type val) { self.m_impl[index - 1] = val; };

    image_compat["getView"]
        = [](const self_t &self) -> Paths::Image::ImageView { return static_cast<Paths::Image::ImageView>(self); };

    image_compat["clear"] = [](self_t &self) { self.resize(0, 0); };
}

}