#pragma once

#include <concepts>

#include <gfx/ray.hpp>
#include <gfx/scene/scene.hpp>
#include <gfx/spectrum.hpp>

namespace Gfx {

struct CameraOptions {
    Real fovWidth = M_PI / 2; //in radians
    Point position{{0, 0, 0}};
    Math::Matrix<Real, 3, 3> rotation{{
                                        1, 0, 0,
                                        0, 1, 0,
                                        0, 0, 1
                                      }};
    //Point rotation{0, 0, 0};
};

class Integrator {
  public:
    virtual ~Integrator() = default;

    virtual void SetSize(size_t width, size_t height) = 0;

    virtual void SetScene(std::shared_ptr<Scene> newScene) = 0;

    virtual void SetCameraOptions(CameraOptions opts) = 0;

    virtual const Image &GetRender() = 0;
};

}


//integrators do not benefit from compile time polymorphism as much as for example shapes
/*namespace Gfx::Concepts {

template<typename T>
concept Integrator = requires(const T &i) {
    true;
};

}*/
