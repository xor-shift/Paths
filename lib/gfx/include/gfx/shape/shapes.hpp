#pragma once

#include "shape.hpp"

#include "aabox.hpp"
#include "plane.hpp"
#include "sphere.hpp"
#include "triangle.hpp"

#include <variant>

namespace Gfx::Shape {

typedef std::variant<AABox, Plane, Triangle, Parallelogram, Sphere> Shape;
typedef std::variant<AABox, Triangle, Parallelogram, Sphere> BoundableShape;

}
