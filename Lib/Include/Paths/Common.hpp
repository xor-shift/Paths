#pragma once

//#define LIBGFX_SWRP_SINGLE_THREAD
#define LIBGFX_USE_CASSERT

#ifndef NDEBUG
#    define LIBGFX_ENABLE_ASSERT
#    define LIBGFX_ENABLE_NORMAL_CHECKS
#endif

#if defined LIBGFX_USE_CASSERT and defined LIBGFX_ENABLE_ASSERT
#    include <cassert>
#endif

#include <cmath>
#include <thread>

#include "Maths/Matrix.hpp"
#include "Maths/Vector.hpp"

namespace Paths {

#ifdef LIBGFX_ENABLE_ASSERT
namespace Detail {
template<typename... Ts> inline void do_assert(bool b, fmt::format_string<Ts...> fmt, Ts &&...args) {
    if (!b) {
        std::fputs(fmt::format("assertion failed: {}", fmt::format(fmt, std::forward<Ts>(args)...)).c_str(), stderr);
#    ifdef LIBGFX_USE_CASSERT
        assert(b);
#    endif
        std::abort();
    }
}
}
#else
template<typename... Ts> inline void do_assert(bool b, fmt::format_string<Ts...> fmt, Ts &&...args) { }
#endif

namespace ProgramConfig {

static constexpr const bool single_thread = false;
static constexpr const bool default_spin = true;

static constexpr const bool embed_ray_stats = true;

static const size_t preferred_thread_count = single_thread ? 1 : std::thread::hardware_concurrency();

}

using Real = std::double_t;

static constexpr Real inf = std::numeric_limits<Real>::infinity();
static constexpr Real eps = std::numeric_limits<Real>::epsilon();
static constexpr Real sensible_inf = static_cast<Real>(16777215); // 2^24-1
static constexpr Real sensible_eps = static_cast<Real>(1e-7);

using Point = Maths::Vector<Real, 3>;
static constexpr Point epsilon_point({ sensible_eps, sensible_eps, sensible_eps });
using Matrix = Maths::Matrix<Real, 3, 3>;

}

#ifdef LIBGFX_ENABLE_ASSERT
#    define LIBGFX_ASSERT(b) Paths::Detail::do_assert(b, #    b)
#    define LIBGFX_ASSERT_MSG(b, ...) Paths::Detail::do_assert(b, __VA_ARGS__)
#else
#    define LIBGFX_ASSERT(...) void(0)
#    define LIBGFX_ASSERT_MSG(...) void(0)
#endif

#ifdef LIBGFX_ENABLE_NORMAL_CHECKS
#    define LIBGFX_NORMAL_CHECK(vec) LIBGFX_ASSERT_MSG(Maths::is_normalized(vec), "vector not normalised")
#else
#    define LIBGFX_NORMAL_CHECK(vec) void(0)
#endif

#define NODISCARD_LEAK [[nodiscard("ignoring return leaks resources")]]
#define NODISCARD_PURE [[nodiscard("ignoring the return value of a function with no side effects")]]
