#pragma once

#include "math/math.hpp"

#define LIBGFX_ENABLE_NORMAL_CHECKS

namespace Gfx {

typedef long double Real;

typedef Math::Vector<Real, 3> Point;

static constexpr Real Epsilon = static_cast<Real>(0.0001);

}

#ifdef LIBGFX_ENABLE_NORMAL_CHECKS

#include <cassert>

#define LIBGFX_NORMAL_CHECK(vec) assert((vec).IsNormalized())

#else

#define LIBGFX_NORMAL_CHECK(vec) void(0)

#endif
