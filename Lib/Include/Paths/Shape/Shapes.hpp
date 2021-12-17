#pragma once

#include "Shape.hpp"

#include "AxisAlignedBox.hpp"
#include "Disc.hpp"
#include "Plane.hpp"
#include "Sphere.hpp"
#include "Triangle.hpp"

#include <variant>

namespace Paths::Shape {

using Shape = std::variant<AxisAlignedBox, Disc, Parallelogram, Plane, Sphere, Triangle>;
using BoundableShape = std::variant<AxisAlignedBox, Disc, Parallelogram, Sphere, Triangle>;

/// TODO: rename
template<typename ShapeT = void>
using BoundableShapeT = std::conditional_t<std::is_same_v<ShapeT, void>, BoundableShape, ShapeT>;

/// TODO: rename
template<typename ShapeT = void> using ShapeT = std::conditional_t<std::is_same_v<ShapeT, void>, Shape, ShapeT>;

constexpr auto apply(const auto &shape, auto &&cb) {
    using ShapeT = std::decay_t<decltype(shape)>;

    if constexpr (std::is_same_v<ShapeT, Shape> || std::is_same_v<ShapeT, BoundableShape>)
        return std::visit(cb, shape);
    else
        return std::invoke(cb, shape);
}

constexpr auto apply(auto &&shape, auto &&cb) {
    using ShapeT = std::decay_t<decltype(shape)>;

    if constexpr (std::is_same_v<ShapeT, Shape> || std::is_same_v<ShapeT, BoundableShape>)
        return std::visit(cb, std::forward<ShapeT>(shape));
    else
        return std::invoke(cb, std::forward<ShapeT>(shape));
}

template<typename It> std::optional<Intersection> intersect_linear(Ray ray, It begin, It end) {
    std::optional<Intersection> best = std::nullopt;

    for (It it = begin; it < end; it++) {
        apply(*it, [ray, &best]<Concepts::Shape T>(const T &s) {
            Intersection::replace(best, std::move(s.intersect_ray(ray)));
        });
    }

    return best;
}

template<typename ShapeT, typename From> std::vector<BoundableShapeT<ShapeT>> convert_shapes_vector(const From &store) {
    std::vector<Paths::Shape::BoundableShapeT<ShapeT>> extracted;

    for (const auto &s : store) {
        Paths::Shape::apply(s, [&extracted](auto &&s) {
            using T = std::decay_t<decltype(s)>;

            if constexpr (std::is_same_v<ShapeT, void>) {
                if constexpr (Paths::Concepts::Boundable<T>)
                    extracted.emplace_back(s);
            } else {
                if constexpr (std::is_same_v<ShapeT, T>)
                    extracted.emplace_back(s);
            }
        });
    }

    return extracted;
}

}
