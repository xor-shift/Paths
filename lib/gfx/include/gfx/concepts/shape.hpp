#pragma once

#include <concepts>
#include <optional>

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
    { s.boundable } -> std::convertible_to<bool>;
    { s.matIndex } -> std::convertible_to<std::size_t>;
    { s.Intersect(ray) } -> std::same_as<std::optional<Intersection>>;
} && requires(T &s) {
    true;
};

}