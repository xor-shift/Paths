#pragma once

//#define LIBGFX_SWRP_SINGLE_THREAD
#define LIBGFX_PREFER_SPIN

#define LIBGFX_EMBED_RAY_STATS

#define LIBGFX_ENABLE_ASSERT
#define LIBGFX_USE_CASSERT
#define LIBGFX_ENABLE_NORMAL_CHECKS

#if defined LIBGFX_USE_CASSERT and defined LIBGFX_ENABLE_ASSERT
# include <cassert>
#endif

#include <cmath>
#include <thread>

#include <maths/matrix.hpp>
#include <maths/vector.hpp>

namespace Gfx {

#ifdef LIBGFX_ENABLE_ASSERT
namespace Detail {
template<typename... Ts>
inline void Assert(bool b, fmt::format_string<Ts...> fmt, Ts &&...args) {
    if (!b) {
        std::fputs(fmt::format("assertion failed: {}", fmt::format(fmt, std::forward<Ts>(args)...)).c_str(), stderr);
#ifdef LIBGFX_USE_CASSERT
        assert(b);
#endif
        std::abort();
    }
}
}
#else

template<typename... Ts>
inline void Assert(bool b, fmt::format_string<Ts...> fmt, Ts &&...args) {}
#endif

namespace ProgramConfig {

static constexpr bool EmbedRayStats = true;
static constexpr bool VisualiseRayStats = false;

}

typedef std::float_t Float;
typedef std::double_t Double;
typedef Double Real;

static constexpr Real inf = std::numeric_limits<Real>::infinity();
static constexpr Real eps = std::numeric_limits<Real>::epsilon();
static constexpr Real sensibleInf = static_cast<Real>(16777215); //2^24-1
static constexpr Real sensibleEps = static_cast<Real>(1e-7);

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

#ifdef LIBGFX_ENABLE_ASSERT
# define LIBGFX_ASSERT(b) Gfx::Detail::Assert(b, #b)
# define LIBGFX_ASSERT_MSG(b, ...) Gfx::Detail::Assert(b, __VA_ARGS__)
#else
# define LIBGFX_ASSERT(...) void(0)
# define LIBGFX_ASSERT_MSG(...) void(0)
#endif

#ifdef LIBGFX_ENABLE_NORMAL_CHECKS
# define LIBGFX_NORMAL_CHECK(vec) LIBGFX_ASSERT_MSG(Maths::IsNormalized(vec), "vector not normalised")
#else
# define LIBGFX_NORMAL_CHECK(vec) void(0)
#endif