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

}
