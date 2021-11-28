#pragma once

#include "shape.hpp"

#include "aabox.hpp"
#include "disc.hpp"
#include "plane.hpp"
#include "sphere.hpp"
#include "triangle.hpp"

#include <variant>

namespace Gfx::Shape {

typedef std::variant<AABox, Disc, Parallelogram, Plane, Sphere, Triangle> Shape;
typedef std::variant<AABox, Disc, Parallelogram, Sphere, Triangle> BoundableShape;

template<typename ShapeT = void>
using boundable_shape_t = std::conditional_t<std::is_same_v<ShapeT, void>, BoundableShape, ShapeT>;

template<typename ShapeT = void>
using shape_t = std::conditional_t<std::is_same_v<ShapeT, void>, Shape, ShapeT>;

constexpr auto Apply(const auto &shape, auto &&cb) {
    using ShapeT = std::decay_t<decltype(shape)>;

    if constexpr (std::is_same_v<ShapeT, Shape> || std::is_same_v<ShapeT, BoundableShape>)
        return std::visit(cb, shape);
    else
        return std::invoke(cb, shape);
}

constexpr auto Apply(auto &&shape, auto &&cb) {
    using ShapeT = std::decay_t<decltype(shape)>;

    if constexpr (std::is_same_v<ShapeT, Shape> || std::is_same_v<ShapeT, BoundableShape>)
        return std::visit(cb, std::forward<ShapeT>(shape));
    else
        return std::invoke(cb, std::forward<ShapeT>(shape));
}

template<typename It>
std::optional<Intersection> IntersectLinear(Ray ray, It begin, It end) {
    std::optional<Intersection> best = std::nullopt;

    for (It it = begin; it < end; it++) {
        Apply(*it, [ray, &best]<Concepts::Shape T>(const T &s) {
            Intersection::Replace(best, std::move(s.Intersect(ray)));
        });
    }

    return best;
}


}
