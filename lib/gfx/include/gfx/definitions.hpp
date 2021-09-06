#pragma once

#include "maths/maths.hpp"

#define LIBGFX_ENABLE_NORMAL_CHECKS

namespace Gfx {

typedef double Real;

typedef Math::Vector<Real, 3> Point;

static constexpr Real Epsilon = static_cast<Real>(0.0001);
static constexpr Point EpsilonVector({Epsilon, Epsilon, Epsilon});

}

#ifdef LIBGFX_ENABLE_NORMAL_CHECKS

#include <cassert>

#define LIBGFX_NORMAL_CHECK(vec) assert(Math::IsNormalized(vec))

#else

#define LIBGFX_NORMAL_CHECK(vec) void(0)

#endif
