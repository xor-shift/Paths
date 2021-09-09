#pragma once

#include "definitions.hpp"

namespace Gfx {

struct Ray {
    constexpr Ray(Point origin, Point direction)
      : origin(origin)
        , direction(direction)
        , directionReciprocals(Math::Reciprocal(direction)) {}

    Point origin;
    Point direction;
    Point directionReciprocals;
};

struct Intersection {
    constexpr Intersection(const Ray &ray, size_t matIndex, Real distance, Point normal = {{0}}, Math::Vector<Real, 2> uv = {{0, 0}})
    :matIndex(matIndex)
    ,distance(distance)
    ,intersectionPoint(ray.origin + ray.direction * distance)
    ,normal(normal)
    ,uv(uv) {}

    std::size_t matIndex{};

    Real distance{};
    Point intersectionPoint;
    Point normal{};

    Math::Vector<Real, 2> uv{{0, 0}};

    //constexpr Point ComputeIntersectionPoint(const Ray &ray) const noexcept { return ray.origin + ray.direction * distance; }
};

}
