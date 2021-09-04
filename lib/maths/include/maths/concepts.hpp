#pragma once

namespace Math {

template<typename T, typename U>
concept Addable = requires(T x, U y) { x + y; x += y; };

template<typename T, typename U>
concept Subtractable = requires(T x, U y) { x - y; x -= y; };

template<typename T, typename U>
concept Multipliable = requires(T x, U y) { x * y; x *= y; };

template<typename T, typename U>
concept Divisible = requires(T x, U y) { x / y; x /= y; };

}
