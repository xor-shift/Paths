#include "luaCompat.hpp"

namespace Utils::LUA::Detail {

extern void AddRayToLUA(sol::state &lua) {
    auto rayCompat = lua.new_usertype<Gfx::Ray>(
      "ray", sol::constructor_list<Gfx::Ray(Gfx::Point, Gfx::Point)>{},

      LuaProperty(Gfx::Ray, origin),
      LuaProperty(Gfx::Ray, direction)
    );

    auto isectionCompat = lua.new_usertype<Gfx::Intersection>(
      "intersection", sol::constructor_list<
        Gfx::Intersection(const Gfx::Ray &, size_t, Gfx::Real),
        Gfx::Intersection(const Gfx::Ray &, size_t, Gfx::Real, Gfx::Point),
        Gfx::Intersection(const Gfx::Ray &, size_t, Gfx::Real, Gfx::Point, Maths::Vector<Gfx::Real, 2>)
      >{},

      LuaProperty(Gfx::Intersection, matIndex),
      LuaProperty(Gfx::Intersection, distance),
      LuaProperty(Gfx::Intersection, intersectionPoint),
      LuaProperty(Gfx::Intersection, normal),
      LuaProperty(Gfx::Intersection, goingIn),
      LuaProperty(Gfx::Intersection, orientedNormal),
      LuaProperty(Gfx::Intersection, uv)
    );
}

}