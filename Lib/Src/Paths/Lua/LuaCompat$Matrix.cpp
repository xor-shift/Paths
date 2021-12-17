#include "Paths/Lua/LuaCompat.hpp"

#include "Paths/Common.hpp"

namespace Paths::Lua::Detail {

extern void add_matrix_to_lua(sol::state &lua) {
    const auto constructors = sol::constructors<Paths::Matrix(),
        Paths::Matrix(Paths::Real, Paths::Real, Paths::Real, Paths::Real, Paths::Real, Paths::Real, Paths::Real,
            Paths::Real, Paths::Real)>();
    auto matrix_compat = lua.new_usertype<Paths::Matrix>("matrix", constructors);

    matrix_compat["newIdentity"] = []() -> Paths::Matrix { return Maths::identity_matrix<Paths::Real, 3>(); };

    matrix_compat["newRadRotation"] = [](Paths::Real yaw, Paths::Real pitch, Paths::Real roll = 0) -> Paths::Matrix {
        return Paths::Matrix::rotation(yaw, pitch, roll);
    };

    matrix_compat["newDegRotation"] = [](Paths::Real yaw, Paths::Real pitch, Paths::Real roll = 0) -> Paths::Matrix {
        constexpr const Paths::Real k = M_PI / 180.;
        return Paths::Matrix::rotation(yaw * k, pitch * k, roll * k);
    };

    matrix_compat["mulMatrices"]
        = [](Paths::Matrix lhs, Paths::Matrix rhs) -> Paths::Matrix { return static_cast<Paths::Matrix>(lhs * rhs); };

    matrix_compat[sol::meta_function::multiplication]
        = sol::overload([](const Paths::Matrix &lhs, const Paths::Matrix &rhs) -> Paths::Matrix { return lhs * rhs; },
            [](const Paths::Matrix &lhs, Paths::Real rhs) -> Paths::Matrix { return lhs * rhs; });

    matrix_compat["mulByMatrix"]
        = [](Paths::Matrix &self, Paths::Matrix m) -> Paths::Matrix { return static_cast<Paths::Matrix>(self * m); };

    matrix_compat["scale"]
        = [](Paths::Matrix &self, Paths::Real k) -> Paths::Matrix { return static_cast<Paths::Matrix>(self * k); };

    matrix_compat["setAt"]
        = [](Paths::Matrix &self, std::size_t i, std::size_t j, Paths::Real v) -> void { self.at(i, j) = v; };

    matrix_compat["getAt"]
        = [](const Paths::Matrix &self, std::size_t i, std::size_t j) -> Paths::Real { return self.at(i, j); };
}

}