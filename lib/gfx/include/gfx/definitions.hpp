#pragma once

#include "maths/maths.hpp"

#define LIBGFX_ENABLE_NORMAL_CHECKS

namespace Gfx {

typedef double Real;
static constexpr Real inf = std::numeric_limits<Real>::infinity();
static constexpr Real eps = std::numeric_limits<Real>::epsilon();
static constexpr Real sensibleInf = static_cast<Real>(1e20);
static constexpr Real sensibleEps = static_cast<Real>(0.000001);

typedef Math::Vector<Real, 3> Point;
static constexpr Point EpsilonVector({sensibleEps, sensibleEps, sensibleEps});

}

#ifdef LIBGFX_ENABLE_NORMAL_CHECKS

#include <cassert>

#define LIBGFX_NORMAL_CHECK(vec) assert(Math::IsNormalized(vec))

#else

#define LIBGFX_NORMAL_CHECK(vec) void(0)

#endif
