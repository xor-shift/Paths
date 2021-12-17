#pragma once

#include <cmath>

namespace Maths {

template<typename T> constexpr T lerp(T a, T b, T p) noexcept { return std::lerp(a, b, p); }

template<typename T> constexpr T inv_lerp(T a, T b, T v) noexcept {
    return b == a ? std::is_floating_point_v<T> ? INFINITY : std::numeric_limits<T>::max()
                  : (v - a) / (b - a); // v/(b-a) + a/(b-a)
}

}