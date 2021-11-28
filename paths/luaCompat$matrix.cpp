#include "luaCompat.hpp"

#include <gfx/common.hpp>

namespace Utils::LUA::Detail {

extern void AddMatrixToLUA(sol::state &lua) {
    auto matrixCompat = lua.new_usertype<Gfx::Matrix>(
      "matrix",
      sol::constructors<
        Gfx::Matrix(),
        Gfx::Matrix(Gfx::Real, Gfx::Real, Gfx::Real, Gfx::Real, Gfx::Real, Gfx::Real, Gfx::Real, Gfx::Real, Gfx::Real)
      >(),
      sol::meta_function::multiplication, sol::overload([](const Gfx::Matrix &lhs, const Gfx::Matrix &rhs) -> Gfx::Matrix { return lhs * rhs; },
                                                        [](const Gfx::Matrix &lhs, Gfx::Real rhs) -> Gfx::Matrix { return lhs * rhs; })
    );

    matrixCompat["newIdentity"] = []() -> Gfx::Matrix { return Maths::Identity<Gfx::Real, 3>(); };
    matrixCompat["newRadRotation"] = [](Gfx::Real yaw, Gfx::Real pitch, Gfx::Real roll = 0) -> Gfx::Matrix {
        return Gfx::Matrix::Rotation(yaw, pitch, roll);
    };
    matrixCompat["newDegRotation"] = [](Gfx::Real yaw, Gfx::Real pitch, Gfx::Real roll = 0) -> Gfx::Matrix {
        constexpr const Gfx::Real k = M_PI / 180.;
        return Gfx::Matrix::Rotation(yaw * k, pitch * k, roll * k);
    };
    matrixCompat["mulMatrices"] = [](Gfx::Matrix lhs, Gfx::Matrix rhs) -> Gfx::Matrix {
        return static_cast<Gfx::Matrix>(lhs * rhs);
    };

    matrixCompat["mulByMatrix"] = [](Gfx::Matrix &self, Gfx::Matrix m) -> Gfx::Matrix { return static_cast<Gfx::Matrix>(self * m); };
    matrixCompat["scale"] = [](Gfx::Matrix &self, Gfx::Real k) -> Gfx::Matrix { return static_cast<Gfx::Matrix>(self * k); };
    matrixCompat["setAt"] = [](Gfx::Matrix &self, std::size_t i, std::size_t j, Gfx::Real v) -> void { self.At(i, j) = v; };
    matrixCompat["getAt"] = [](const Gfx::Matrix &self, std::size_t i, std::size_t j) -> Gfx::Real { return self.At(i, j); };
}

}