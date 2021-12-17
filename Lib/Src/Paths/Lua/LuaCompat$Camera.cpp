#include "Paths/Lua/LuaCompat.hpp"

#include "Paths/Camera.hpp"

namespace Paths::Lua::Detail {

extern void add_camera_to_lua(sol::state &lua) {
    auto camera_compat = lua.new_usertype<Paths::Camera>("camera", sol::default_constructor);

    camera_compat["position"] = SOL_PROPERTY(Paths::Camera, m_position);
    camera_compat["fovHint"] = SOL_PROPERTY(Paths::Camera, m_fov_hint);
    camera_compat["resolution"] = SOL_PROPERTY(Paths::Camera, m_resolution);
    camera_compat["rayTransform"] = SOL_PROPERTY(Paths::Camera, m_ray_transform);
    camera_compat["apertureDiameter"] = SOL_PROPERTY(Paths::Camera, m_aperture_diameter);
    camera_compat["focalDistance"] = SOL_PROPERTY(Paths::Camera, m_focal_distance);
    camera_compat["setLookDeg"] = &Paths::Camera::set_look_deg;
    camera_compat["setLookRad"] = &Paths::Camera::set_look_rad;
    camera_compat["setLookAt"] = &Paths::Camera::set_look_at;
}

}
