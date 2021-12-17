#include "Paths/Lua/LuaCompat.hpp"

#include "Paths/Common.hpp"

namespace Paths::Lua::Detail {

template<typename T, std::size_t n>
static void add_vector_specialization_generic(sol::state &lua, const std::string &name) {
    using point_type = Maths::Vector<T, n>;

    auto spec_compat = lua.new_usertype<point_type>(name);

    spec_compat["new"] = sol::overload([]() -> point_type { return {}; },
        [](const sol::table &table) -> point_type {
            point_type p {};
            for (std::size_t i = 0; i < std::min(point_type::m_vector_size, table.size()); i++)
                p[i] = table[i + 1];
            return p;
        });

    spec_compat[sol::meta_function::index] = [](const point_type &self, int index) -> T { return self[index - 1]; };

    spec_compat[sol::meta_function::new_index]
        = [](point_type &self, int index, T v) -> T { return self[index - 1] = v; };

    spec_compat[sol::meta_function::addition]
        = [](const point_type &lhs, const point_type &rhs) -> point_type { return lhs + rhs; };

    spec_compat[sol::meta_function::subtraction]
        = [](const point_type &lhs, const point_type &rhs) -> point_type { return lhs - rhs; };

    spec_compat[sol::meta_function::multiplication]
        = sol::overload([](const point_type &lhs, T rhs) -> point_type { return lhs * rhs; },
            [](const point_type &lhs, const point_type &rhs) -> T { return Maths::dot(lhs, rhs); });

    spec_compat[sol::meta_function::division] = [](const point_type &lhs, T rhs) -> point_type { return lhs / rhs; };

    spec_compat[sol::meta_function::length] = [](const point_type &self) -> T { return Maths::Magnitude(self); };

    spec_compat["cross"]
        = [](const point_type &lhs, const point_type &rhs) -> point_type { return Maths::dot(lhs, rhs); };

    spec_compat["dot"] = [](const point_type &lhs, const point_type &rhs) -> T { return Maths::dot(lhs, rhs); };

    spec_compat["magnitude"] = [](const point_type &self) -> T { return Maths::Magnitude(self); };

    spec_compat["normalized"] = [](const point_type &self) -> point_type { return Maths::normalized(self); };
}

extern void add_vector_specializations(sol::state &lua) {
    Detail::add_vector_specialization_generic<Real, 3>(lua, "point");
    Detail::add_vector_specialization_generic<std::size_t, 2>(lua, "dim2d");
}

}