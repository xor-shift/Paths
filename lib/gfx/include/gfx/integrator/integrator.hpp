#pragma once

#include <gfx/camera.hpp>
#include <gfx/image/image.hpp>
#include <gfx/ray.hpp>
#include <gfx/scene/scene.hpp>
#include <maths/maths.hpp>
#include <utils/worker_pool.hpp>

namespace Gfx {

namespace Concepts {

template<typename T>
concept Integrator = requires(const T &ci, T &i, Camera camera, Scene *scene, const std::string &str) {
    { i.SetCamera(camera) } -> std::same_as<void>;
    { i.SetScene(scene) } -> std::same_as<void>;
    { i.Render() } -> std::same_as<void>;
    { i.GetImage() } -> std::convertible_to<Image::ImageView>;
    { i.Reset() } -> std::same_as<void>;
    { i.ExportState(str) } -> std::same_as<void>;
    { i.ImportState(str) } -> std::same_as<void>;
};

}

class Integrator {
  public:
    virtual ~Integrator() noexcept = default;

    virtual void SetCamera(Camera) noexcept = 0;

    virtual void SetScene(Scene *) noexcept = 0;

    virtual void Render() noexcept = 0;

    virtual Image::ImageView GetImage() noexcept = 0;
};

}