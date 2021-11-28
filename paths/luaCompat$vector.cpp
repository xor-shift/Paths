#include "luaCompat.hpp"

#include <gfx/common.hpp>

namespace Utils::LUA::Detail {

template<typename T, std::size_t n>
static void AddVectorSpecializationGeneric(sol::state &lua, const std::string &name) {
    using point_type = Maths::Vector<T, n>;

    auto specCompat = lua.new_usertype<point_type>(
      name,
      "new", sol::overload(
        []() -> point_type { return {}; },
        [](const sol::table &table) -> point_type {
            point_type p{};
            for (std::size_t i = 0; i < std::min(point_type::vectorSize, table.size()); i++)
                p[i] = table[i + 1];
            return p;
        }
      ),

      sol::meta_function::index, [](const point_type &self, int index) -> T { return self[index - 1]; },
      sol::meta_function::new_index, [](point_type &self, int index, T v) -> T { return self[index - 1] = v; },
      sol::meta_function::addition, [](const point_type &lhs, const point_type &rhs) -> point_type { return lhs + rhs; },
      sol::meta_function::subtraction, [](const point_type &lhs, const point_type &rhs) -> point_type { return lhs - rhs; },
      sol::meta_function::multiplication, sol::overload(
        [](const point_type &lhs, T rhs) -> point_type { return lhs * rhs; },
        [](const point_type &lhs, const point_type &rhs) -> T { return Maths::Dot(lhs, rhs); }
      ),
      sol::meta_function::division, [](const point_type &lhs, T rhs) -> point_type { return lhs / rhs; },

      "cross", [](const point_type &lhs, const point_type &rhs) -> point_type { return Maths::Dot(lhs, rhs); },
      "dot", [](const point_type &lhs, const point_type &rhs) -> T { return Maths::Dot(lhs, rhs); },
      sol::meta_function::length, [](const point_type &self) -> T { return Maths::Magnitude(self); },
      "magnitude", [](const point_type &self) -> T { return Maths::Magnitude(self); },
      "normalized", [](const point_type &self) -> point_type { return Maths::Normalized(self); }
    );
}

extern void AddVectorSpecializations(sol::state &lua) {
    Detail::AddVectorSpecializationGeneric<Gfx::Real, 3>(lua, "point");
    Detail::AddVectorSpecializationGeneric<std::size_t, 2>(lua, "dim2d");
}

}