#pragma once

#include <optional>

#include "definitions.hpp"

namespace Gfx {

#ifdef LIBGFX_EMBED_RAY_STATS

struct RayStats {
    size_t traversals{0};
};

#endif

struct Ray {
    constexpr Ray(Point origin, Point direction)
      : origin(origin)
        , direction(direction)
        , directionReciprocals(Math::Reciprocal(direction))
        , majorDirection(MajorDirection(direction)) {}

    Point origin;
    Point direction;
    Point directionReciprocals;
    size_t majorDirection;

  private:
    static constexpr size_t MajorDirection(const Point &direction) noexcept {
        const Point abs = Math::Abs(direction);

        size_t majorAxis;

        if (abs[0] > abs[1]) {
            if (abs[0] > abs[2]) majorAxis = 0;
            else majorAxis = 2;
        } else {
            if (abs[1] > abs[2]) majorAxis = 1;
            else majorAxis = 2;
        }

        bool neg = direction[majorAxis] < 0;
        majorAxis *= 2;
        majorAxis += neg;

        return majorAxis;
    }
};

struct Intersection {
    constexpr Intersection(const Ray &ray, size_t matIndex, Real distance, Point normal = {{0}}, Math::Vector<Real, 2> uv = {{0, 0}}
#ifdef LIBGFX_EMBED_RAY_STATS
      , RayStats stats = {}
#endif
    )
      : matIndex(matIndex)
        , distance(distance)
        , intersectionPoint(ray.origin + ray.direction * distance)
        , normal(normal)
        , uv(uv)
#ifdef LIBGFX_EMBED_RAY_STATS
    , stats(stats)
#endif
    {}

    std::size_t matIndex{};

    Real distance{};
    Point intersectionPoint;
    Point normal{};

    Math::Vector<Real, 2> uv{{0, 0}};


#ifdef LIBGFX_EMBED_RAY_STATS
    RayStats stats;
#endif

    //constexpr Point ComputeIntersectionPoint(const Ray &ray) const noexcept { return ray.origin + ray.direction * distance; }

    static constexpr bool Replace(std::optional<Intersection> &toReplace, std::optional<Intersection> &&replaceWith) noexcept {
        if (!replaceWith || replaceWith->distance < 0) return false;
        if (!toReplace || toReplace->distance > replaceWith->distance) {
            toReplace = std::move(replaceWith);
            return true;
        }
        return false;
    }
};

}
