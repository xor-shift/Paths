#include "luaCompat.hpp"

#include <gfx/image/image.hpp>

#include <gfx/image/exporters/exr.hpp>
#include <gfx/image/exporters/png.hpp>

namespace Utils::LUA::Detail {

extern void AddImageViewToLUA(sol::state &state) {
    auto imageViewCompat = state.new_usertype<Gfx::Image::ImageView>(
      "imageView", sol::no_constructor,
      "export", [](Gfx::Image::ImageView self, const std::string &file, const std::string &type) -> bool {
          if (type == "exrf16") return Gfx::Image::Exporter<Gfx::Image::EXRExporterF16>::Export(file, self);
          else if (type == "exrf32") return Gfx::Image::Exporter<Gfx::Image::EXRExporterF32>::Export(file, self);
          else if (type == "png") return Gfx::Image::Exporter<Gfx::Image::PNGExporter>::Export(file, self);

          return false;
      },
      "getAt", [](const Gfx::Image::ImageView &self, std::size_t row, std::size_t col) -> Gfx::Color {
          return self.At(col, row);
      }
    );
}

extern void AddImageToLUA(sol::state &state) {
    typedef Gfx::Image::Image<> self_t;

    auto imageCompat = state.new_usertype<Gfx::Image::Image<>>(
      "image", sol::constructors<
        Gfx::Image::Image<>(),
        Gfx::Image::Image<>(std::size_t, std::size_t),
        Gfx::Image::Image<>(Gfx::Image::ImageView)
      >(),
      sol::meta_function::length, [](const self_t &self) { return self.size(); },
      sol::meta_function::index, [](const self_t &self, std::size_t index) { return self.impl[index - 1]; },
      sol::meta_function::new_index, [](self_t &self, std::size_t index, self_t::value_type val) { self.impl[index - 1] = val; },
      "getView", [](const self_t &self) -> Gfx::Image::ImageView { return static_cast<Gfx::Image::ImageView>(self); },
      "clear", [](self_t &self) { self.Resize(0, 0); }
    );
}

}