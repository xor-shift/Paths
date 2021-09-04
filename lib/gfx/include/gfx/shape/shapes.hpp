#pragma once

#include "shape.hpp"

#include <variant>

#include <gfx/shape/aabox.hpp>
#include <gfx/shape/plane.hpp>
#include <gfx/shape/triangle.hpp>

namespace Gfx::Shape {

typedef std::variant<AABox, Plane, Triangle> Shape;

}
