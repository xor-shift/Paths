#pragma once

#include <sol/state.hpp>
#include <gfx/integrator/integrator.hpp>

namespace Utils::LUA::Detail {

struct store_t { std::shared_ptr<Gfx::ShapeStore> impl{nullptr}; };
struct scene_t { std::shared_ptr<Gfx::Scene> impl{std::make_shared<Gfx::Scene>()}; };
struct integrator_t { std::unique_ptr<Gfx::Integrator> impl{nullptr}; };

extern void AddVectorSpecializations(sol::state &lua);

extern void AddMatrixToLUA(sol::state &lua);

extern void AddCameraToLUA(sol::state &lua);

extern void AddStoreToLUA(sol::state &lua);

extern void AddSceneToLUA(sol::state &lua);

extern void AddIntegratorToLUA(sol::state &lua);

}
