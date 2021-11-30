#include "luaCompat.hpp"

namespace Utils::LUA::Detail {

extern void AddSceneToLUA(sol::state &lua) {
    auto sceneCompat = lua.new_usertype<scene_t>(
      "scene", sol::default_constructor,
      "getStoreReference", [](scene_t &self) -> store_t { return {self.impl}; },
      "addMaterial", [](scene_t &self, const std::string &name, const sol::table &arguments) -> std::size_t {
          auto albedo = arguments.get<sol::optional<Gfx::Point>>("albedo");
          auto emittance = arguments.get<sol::optional<Gfx::Point>>("emittance");
          auto ior = arguments.get<sol::optional<Gfx::Real>>("ior");
          auto reflectance = arguments.get<sol::optional<Gfx::Real>>("reflectance");

          Gfx::Material mat{
            .reflectance = reflectance ? *reflectance : Gfx::Real{0},
            .ior = ior ? *ior : Gfx::Real{1.003},
            .albedo = albedo ? *albedo : Gfx::Point{},
            .emittance = emittance ? *emittance : Gfx::Point{},
          };

          self.impl->InsertMaterial(mat, name);

          return 0;
      },
      "resolveMaterial", [](const scene_t &self, const std::string &material) { return self.impl->ResolveMaterial(material); },
      "clear", [](scene_t &self) { self.impl = nullptr; }
    );
}

}
