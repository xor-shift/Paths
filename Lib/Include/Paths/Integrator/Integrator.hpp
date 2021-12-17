#pragma once

#include "Maths/Maths.hpp"
#include "Paths/Camera.hpp"
#include "Paths/Image/Image.hpp"
#include "Paths/Ray.hpp"
#include "Paths/Scene/Scene.hpp"
#include "Utils/WorkerPool.hpp"

namespace Paths {

namespace Concepts {

template<typename T>
concept Integrator = requires(const T &ci, T &i, Camera camera, Scene *scene, const std::string &str) {
    { i.set_camera(camera) } -> std::same_as<void>;
    { i.set_scene(scene) } -> std::same_as<void>;
    { i.do_render() } -> std::same_as<void>;
    { i.get_image() } -> std::convertible_to<Image::ImageView>;
};

}

class Integrator {
public:
    virtual ~Integrator() noexcept = default;

    virtual void set_camera(Camera) noexcept = 0;

    virtual void set_scene(Scene *) noexcept = 0;

    virtual void do_render() noexcept = 0;

    virtual Image::ImageView get_image() noexcept = 0;
};

}