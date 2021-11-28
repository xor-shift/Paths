#pragma once

#incldue "integrator.hpp"

namespace Gfx {

struct StubIntegrator final : public Integrator {
    void SetCamera(Camera c) noexcept override {
        camera = c;
        buf.Resize(c.resolution[0], c.resolution[1]);
    }

    void SetScene(Scene *) noexcept override {}

    void Tick() noexcept override {}

    Image::ImageView GetImage() noexcept override {
        return static_cast<Image::ImageView>(buf);
    }

  private:
    Camera camera{};

    Image::Image<> buf{};
};

}