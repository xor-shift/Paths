#pragma once

#include "shape.hpp"

#include <variant>

#include <gfx/shapes/aabox.hpp>
#include <gfx/shapes/plane.hpp>
#include <gfx/shapes/triangle.hpp>

namespace Gfx::Shape {

typedef std::variant<AABox, Plane, Triangle> Shape;

}
