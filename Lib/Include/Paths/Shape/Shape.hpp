#pragma once

#include <concepts>
#include <optional>
#include <utility>

#include "Paths/Ray.hpp"

namespace Paths::Concepts {

template<typename T>
concept Shape = requires(const T &s, const Ray &ray) {
    { s.intersect_ray(ray) } -> std::convertible_to<std::optional<Intersection>>;
}
&&requires(T &s) { true; };

template<typename T>
concept Boundable = requires(const T &s) {
    { s.m_extents } -> std::convertible_to<std::pair<Point, Point>>;
    { s.m_center } -> std::convertible_to<Point>;
};

}
