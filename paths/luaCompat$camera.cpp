#include "luaCompat.hpp"

#include <gfx/camera.hpp>

namespace Utils::LUA::Detail {

extern void AddCameraToLUA(sol::state &lua) {
    auto cameraCompat = lua.new_usertype<Gfx::Camera>(
      "camera", sol::default_constructor,
      "position", sol::property(
        [](Gfx::Camera &self, Gfx::Point p) {
            self.position = p;
        }, [](const Gfx::Camera &self) -> Gfx::Point {
            return self.position;
        }),
      "fovHint", sol::property(
        [](Gfx::Camera &self, Gfx::Real v) {
            self.fovHint = v;
        }, [](const Gfx::Camera &self) {
            return self.fovHint;
        }),
      "resolution", sol::property(
        [](Gfx::Camera &self, Maths::Vector<std::size_t, 2> v) {
            self.resolution = v;
        }, [](const Gfx::Camera &self) {
            return self.resolution;
        }),
      "rayTransform", sol::property(
        [](Gfx::Camera &self, Gfx::Matrix v) {
            self.rayTransform = v;
        }, [](const Gfx::Camera &self) {
            return self.rayTransform;
        }),
      "setLookDeg", &Gfx::Camera::SetLookDeg,
      "setLookRad", &Gfx::Camera::SetLookRad,
      "setLookAt", &Gfx::Camera::SetLookAt
    );
}

}