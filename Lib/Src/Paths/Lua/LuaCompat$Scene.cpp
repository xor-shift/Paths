#include "Paths/Lua/LuaCompat.hpp"

namespace Paths::Lua::Detail {

extern void add_scene_to_lua(sol::state &lua) {
    auto scene_compat = lua.new_usertype<SceneWrapper>("scene", sol::default_constructor);

    scene_compat["getStoreReference"] = [](SceneWrapper &self) -> StoreWrapper { return { self.m_impl }; };

    scene_compat["addMaterial"]
        = [](SceneWrapper &self, const std::string &name, const sol::table &arguments) -> std::size_t {
        auto albedo = arguments.get<sol::optional<Paths::Point>>("albedo");
        auto emittance = arguments.get<sol::optional<Paths::Point>>("emittance");
        auto ior = arguments.get<sol::optional<Paths::Real>>("ior");
        auto reflectance = arguments.get<sol::optional<Paths::Real>>("reflectance");

        Paths::Material mat {
            .m_reflectance = reflectance ? *reflectance : Paths::Real { 0 },
            .m_ior = ior ? *ior : Paths::Real { 1.003 },
            .m_albedo = albedo ? *albedo : Paths::Point {},
            .m_emittance = emittance ? *emittance : Paths::Point {},
        };

        self.m_impl->insert_material(mat, name);

        return 0;
    };

    scene_compat["resolveMaterial"]
        = [](const SceneWrapper &self, const std::string &material) { return self.m_impl->resolve_material(material); };

    scene_compat["clear"] = [](SceneWrapper &self) { self.m_impl = nullptr; };

    scene_compat["castRay"] = [](const SceneWrapper &self, Paths::Ray ray) -> std::optional<Paths::Intersection> {
        std::size_t _;
        return self.m_impl->intersect_ray(ray, _, _);
    };
}

}
