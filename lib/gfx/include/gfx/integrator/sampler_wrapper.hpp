#pragma once

#include "integrator.hpp"

namespace Gfx {

class SamplerWrapperIntegrator : public Integrator {
  public:
    SamplerWrapperIntegrator() { StartThreads(); }

    ~SamplerWrapperIntegrator() noexcept override {
        rendererPool.Close();
        if (rendererThread.joinable())
            rendererThread.join();
    }

    void SetCamera(Camera c) noexcept override {
        camera = c;
        camera.Prepare();
        backBuffer.Resize(c.resolution[0], c.resolution[1]);
    }

    void SetScene(Scene *s) noexcept override {
        scene = s;
    }

    void Render() noexcept override {
        rendererPool.SplitWork(camera.resolution[1], ProgramConfig::preferredThreadCount, [this](size_t start, size_t end) {
            return WorkItem{
              .self = *this,
              .start = start,
              .end = end,
            };
        });
        rendererPool.WGWait();
    }

    [[nodiscard]] Image::ImageView GetImage() noexcept override { return static_cast<Image::ImageView>(backBuffer); }

  protected:
    [[nodiscard]] virtual Color Sample(Ray ray, Scene &s) const noexcept { return {}; };

  private:
    Scene *scene{nullptr};
    Camera camera{};
    Image::Image<> backBuffer{};

    struct WorkItem {
        SamplerWrapperIntegrator &self;
        std::size_t start, end;
    };

    static void WorkerFn(WorkItem &&item) noexcept {
        for (std::size_t i = item.start; i < item.end; i++)
            item.self.IntegrateLine(i);
    }

    std::thread rendererThread;
    Utils::WorkerPoolWG<decltype(&SamplerWrapperIntegrator::WorkerFn), WorkItem, ProgramConfig::defaultSpin> rendererPool{&SamplerWrapperIntegrator::WorkerFn,
                                                                                                                          ProgramConfig::preferredThreadCount};

    void StartThreads() {
        rendererThread = std::thread([this] { rendererPool.Work(ProgramConfig::preferredThreadCount); });
        //bufferCopyThread = std::thread([this] { btfWorkerPool.Work(preferredThreadCount); });
    }

    void IntegrateLine(std::size_t y) noexcept {
        for (std::size_t x = 0; x < camera.resolution[0]; x++) {
            const auto ray = camera.MakeRay(x, y);
            backBuffer.At(x, y) = Sample(ray, *scene);
        }
    }
};

}