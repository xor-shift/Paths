#pragma once

#include "definitions.hpp"

namespace Gfx {

struct Ray {
    Ray(Point origin, Point direction)
      : origin(origin)
        , direction(direction)
        , directionReciprocals{{1. / direction[0], 1. / direction[1], 1. / direction[2]}} {}

    Point origin;
    Point direction;
    Point directionReciprocals;
};

struct Intersection {
    const Ray &theRay;

    Real distance{};
    Point normal{};
    std::size_t matIndex{};

    Math::Vector<Real, 2> uv{{0, 0}};

    Point intersectionPoint{};

    Intersection &ComputeIntersectionPoint() {
        intersectionPoint = theRay.origin + theRay.direction * distance;
        return *this;
    }
};

}
