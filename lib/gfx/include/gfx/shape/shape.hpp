#pragma once

#include <concepts>
#include <optional>
#include <utility>

#include <gfx/ray.hpp>

namespace Gfx::Concepts {

//true means that a property has to be calculated
//false means that it is not necessary to calculate a property, it might still get calculated if exceedingly cheap
struct IntersectionOptions {
    bool calcUV = false;
    bool calcNormal = false;
};

template<typename T>
concept Shape = requires(const T &s, const Ray &ray) {
    { s.matIndex } -> std::convertible_to<std::size_t>;
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
