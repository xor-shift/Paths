#pragma once

#include "Paths/Integrator/Integrator.hpp"
#include <sol/state.hpp>

namespace Paths::Lua::Detail {

struct StoreWrapper {
    std::shared_ptr<Paths::ShapeStore> m_impl { nullptr };
};

struct SceneWrapper {
    std::shared_ptr<Paths::Scene> m_impl { std::make_shared<Paths::Scene>() };
};

struct IntegratorWrapper {
    std::unique_ptr<Paths::Integrator> m_impl { nullptr };
};

extern void add_vector_specializations(sol::state &lua);

extern void add_matrix_to_lua(sol::state &lua);

extern void add_camera_to_lua(sol::state &lua);

extern void add_store_to_lua(sol::state &lua);

extern void add_scene_to_lua(sol::state &lua);

extern void add_integrator_to_lua(sol::state &lua);

extern void add_image_view_to_lua(sol::state &lua);

extern void add_image_to_lua(sol::state &lua);

extern void add_ray_to_lua(sol::state &lua);

}

#define SOL_PROPERTY(T, name)                                            \
    sol::property([](T &self, decltype(self.name) v) { self.name = v; }, \
        [](const T &self) -> decltype(self.name) { return self.name; })
