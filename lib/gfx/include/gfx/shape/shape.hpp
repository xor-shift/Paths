#pragma once

#include <concepts>
#include <optional>
#include <utility>

#include <gfx/ray.hpp>

namespace Gfx::Concepts {

template<typename T>
concept Shape = requires(const T &s, const Ray &ray) {
    { s.Intersect(ray) } -> std::convertible_to<std::optional<Intersection>>;
} && requires(T &s) {
    true;
};

template<typename T>
concept Boundable = requires(const T &s) {
    { s.extents } -> std::convertible_to<std::pair<Point, Point>>;
    { s.center } -> std::convertible_to<Point>;
};

}
