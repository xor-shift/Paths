/**
 * This file contains a AABox shape, described by two points in space, representing an axis aligned box
 * The main purpose of this shape is use in BVHs
 */

#pragma once

#include <chrono>

#include <gfx/concepts/shape.hpp>
#include <math/math.hpp>

namespace Gfx::Shape {

struct AABox {
  private:
    template<bool getDistance>
    using IntersectionType = std::conditional_t<getDistance, std::pair<bool, Real>, bool>;

    template<bool getDistance>
    [[nodiscard]] static constexpr IntersectionType<getDistance> Func(Real tMin, Real tMax) noexcept {
        bool isect = tMax > std::max<Real>(tMin, 0.);

        if constexpr (getDistance) {
            return std::pair<bool, Real>{isect, tMin > 0. ? tMin : tMax};
        } else {
            return isect;
        }
    }

  public:

    template<bool getDistance = false>
    [[nodiscard]] constexpr auto Intersects0(const Ray &ray) const noexcept {
        Real
          tMin = std::numeric_limits<Real>::min(),
          tMax = std::numeric_limits<Real>::max();

        for (size_t i = 0; i < 3; ++i) {
            auto t1 = (min[i] - ray.origin[i]) * ray.directionReciprocals[i];
            auto t2 = (max[i] - ray.origin[i]) * ray.directionReciprocals[i];

            tMin = std::max(tMin, std::min(std::min(t1, t2), tMax));
            tMax = std::min(tMax, std::max(std::max(t1, t2), tMin));
        }

        return Func<getDistance>(tMin, tMax);
    }

    template<bool getDistance = false>
    [[nodiscard]] constexpr auto Intersects1(const Ray &ray) const noexcept {
        auto t1 = (min - ray.origin) * ray.directionReciprocals;
        auto t2 = (max - ray.origin) * ray.directionReciprocals;

        Real
          tMin = std::numeric_limits<Real>::min(),
          tMax = std::numeric_limits<Real>::max();

        for (std::size_t i = 0; i < 3; i++) {
            tMin = std::max(tMin, std::min(std::min(t1[i], t2[i]), tMax));
            tMax = std::min(tMax, std::max(std::max(t1[i], t2[i]), tMin));
        }

        return Func<getDistance>(tMin, tMax);
    }

    template<bool getDistance = false>
    [[nodiscard]] constexpr auto Intersects2(const Ray &ray) const noexcept {
        auto t1 = (min - ray.origin) * ray.directionReciprocals;
        auto t2 = (max - ray.origin) * ray.directionReciprocals;

        auto vMin = Math::Ops::Vector::MinPerElem(t1, t2);
        auto vMax = Math::Ops::Vector::MaxPerElem(t1, t2);

        Real
          tMin = std::numeric_limits<Real>::min(),
          tMax = std::numeric_limits<Real>::max();

        for (std::size_t i = 0; i < 3; i++) {
            tMin = std::max(tMin, std::min(vMin[i], tMax));
            tMax = std::min(tMax, std::max(vMax[i], tMin));
        }

        return Func<getDistance>(tMin, tMax);
    }

    template<bool getDistance = false>
    [[nodiscard]] constexpr auto Intersects(const Ray &ray) const noexcept { return Intersects0<getDistance>(ray); }

    static void PerfTest() {
        Gfx::Shape::AABox box{
          .min = Gfx::Point{2, 2, 2},
          .max = Gfx::Point{3, 3, 3},
        };

        Gfx::Ray ray({0, 0, 0}, {2.1, 2.1, 2.1});

        auto DoTest = [&box, &ray](const std::string &name, /*bool (Gfx::Shape::AABox::*fn)(const Gfx::Ray &) const noexcept*/ auto fn) {
            constexpr size_t nInvoc = 1024 * 1024;

            auto t0 = std::chrono::system_clock::now();
            for (size_t i = 0; i < nInvoc; i++) [[maybe_unused]] volatile auto res = (box.*fn)(ray);
            auto t1 = std::chrono::system_clock::now();

            auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();

            fmt::print("{}: {}ns, {}ns/op\n", name, ns, static_cast<long double>(ns) / nInvoc);
        };

        fmt::print("BB speed\n");
        DoTest("Intersects0", &Gfx::Shape::AABox::Intersects0<false>);
        DoTest("Intersects1", &Gfx::Shape::AABox::Intersects1<false>);
        DoTest("Intersects2", &Gfx::Shape::AABox::Intersects2<false>);
        DoTest("Intersects", &Gfx::Shape::AABox::Intersects<false>);
        fmt::print("\nrender speed\n");
        DoTest("Intersects0", &Gfx::Shape::AABox::Intersects0<true>);
        DoTest("Intersects1", &Gfx::Shape::AABox::Intersects1<true>);
        DoTest("Intersects2", &Gfx::Shape::AABox::Intersects2<true>);
        DoTest("Intersects", &Gfx::Shape::AABox::Intersects<true>);
    };

    [[nodiscard]] constexpr std::optional<Intersection> Intersect(const Ray &ray) const noexcept {
        if (const auto &[isect, dist] = Intersects<true>(ray); isect) {
            auto isection = Intersection{
              .theRay = ray,
              .distance = dist,
              .normal = Point{0, 0, 0},
              .matIndex = matIndex,
              .uv = {0, 0},
            };

            if (bool calcNormal = true; calcNormal) {
                isection.ComputeIntersectionPoint();

                auto p = isection.intersectionPoint - (min + max) * .5;
                auto d = (max - min) * .5;
                Real bias = 1.00000001;

                isection.normal = Math::Ops::Vector::Normalized(Point{
                  std::floor(p[0] / d[0] * bias),
                  std::floor(p[1] / d[1] * bias),
                  std::floor(p[2] / d[2] * bias),
                });
            }

            if (bool calcUV = true; calcUV) {

            }

            return isection;
        } else return std::nullopt;
    }

    Point min{}, max{};

    std::size_t matIndex{0};
    constexpr static bool boundable = true;
};

}
