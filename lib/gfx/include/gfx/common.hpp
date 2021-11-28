#pragma once

#define LIBGFX_ENABLE_NORMAL_CHECKS
//#define LIBGFX_SWRP_SINGLE_THREAD
#define LIBGFX_PREFER_SPIN

#include <cmath>
#include <thread>

#include <maths/matrix.hpp>
#include <maths/vector.hpp>

namespace Gfx {

typedef std::float_t Float;
typedef std::double_t Double;
typedef Double Real;

static constexpr Real inf = std::numeric_limits<Real>::infinity();
static constexpr Real eps = std::numeric_limits<Real>::epsilon();
static constexpr Real sensibleInf = static_cast<Real>(16777215); //2^24-1
static constexpr Real sensibleEps = static_cast<Real>(1e-6);

typedef Maths::Vector<Real, 3> Point;
static constexpr Point epsilonPoint({sensibleEps, sensibleEps, sensibleEps});
typedef Maths::Matrix<Real, 3, 3> Matrix;

#ifdef LIBGFX_SWRP_SINGLE_THREAD
static const size_t preferredThreadCount = 1;
#else
static const size_t preferredThreadCount = std::thread::hardware_concurrency();
#endif

#ifdef LIBGFX_PREFER_SPIN
static constexpr bool preferredSpin = true;
#else
static constexpr bool preferredSpin = false;
#endif

}

#ifdef LIBGFX_ENABLE_NORMAL_CHECKS

#include <cassert>

#define LIBGFX_NORMAL_CHECK(vec) assert(Maths::IsNormalized(vec))

#else

#define LIBGFX_NORMAL_CHECK(vec) void(0)

#endif