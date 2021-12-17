#include "Paths/Lua/LuaCompat.hpp"

namespace Paths::Lua::Detail {

extern void add_ray_to_lua(sol::state &lua) {
    auto ray_compat
        = lua.new_usertype<Paths::Ray>("ray", sol::constructor_list<Paths::Ray(Paths::Point, Paths::Point)> {});
    ray_compat["origin"] = SOL_PROPERTY(Paths::Ray, m_origin);
    ray_compat["direction"] = SOL_PROPERTY(Paths::Ray, m_direction);

    const auto isection_constructors = sol::constructor_list<Paths::Intersection(
                                                                 const Paths::Ray &, size_t, Paths::Real),
        Paths::Intersection(const Paths::Ray &, size_t, Paths::Real, Paths::Point),
        Paths::Intersection(const Paths::Ray &, size_t, Paths::Real, Paths::Point, Maths::Vector<Paths::Real, 2>)> {};
    auto isection_compat = lua.new_usertype<Paths::Intersection>("intersection", isection_constructors);

    isection_compat["matIndex"] = SOL_PROPERTY(Paths::Intersection, m_mat_index);
    isection_compat["distance"] = SOL_PROPERTY(Paths::Intersection, m_distance);
    isection_compat["intersectionPoint"] = SOL_PROPERTY(Paths::Intersection, m_intersection_point);
    isection_compat["normal"] = SOL_PROPERTY(Paths::Intersection, m_normal);
    isection_compat["goingIn"] = SOL_PROPERTY(Paths::Intersection, m_going_in);
    isection_compat["orientedNormal"] = SOL_PROPERTY(Paths::Intersection, m_oriented_normal);
    isection_compat["uv"] = SOL_PROPERTY(Paths::Intersection, m_uv);
}

}