#include "luaCompat.hpp"

#include <gfx/camera.hpp>

#define Property(name) LuaProperty(Gfx::Camera, name)

namespace Utils::LUA::Detail {

extern void AddCameraToLUA(sol::state &lua) {
    auto cameraCompat = lua.new_usertype<Gfx::Camera>(
      "camera", sol::default_constructor,
      Property(position),
      Property(fovHint),
      Property(resolution),
      Property(rayTransform),

      Property(apertureDiameter),
      Property(focalDistance),

      "setLookDeg", &Gfx::Camera::SetLookDeg,
      "setLookRad", &Gfx::Camera::SetLookRad,
      "setLookAt", &Gfx::Camera::SetLookAt
    );
}

}

#undef Property
