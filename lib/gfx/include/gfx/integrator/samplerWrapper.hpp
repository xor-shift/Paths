#pragma once

#include <condition_variable>
#include <future>
#include <list>
#include <thread>

#include <gfx/integrator/integrator.hpp>
#include <gfx/image.hpp>
#include <gfx/scene/scene.hpp>
#include <maths/matvec.hpp>
#include <gfx/sampler/sampler.hpp>
#include <utils/workerPool.hpp>

namespace Gfx {

template<Concepts::Sampler T, bool takeAverage>
class SamplerWrapperIntegrator {
    void StartThreads() {
#ifdef LIBGFX_SWRP_SINGLE_THREAD
        const auto conc = 1;
#else
        const auto conc = std::thread::hardware_concurrency() * 3 / 4;
#endif

        rendererWorkerThread = std::thread([this, conc] { rendererWorkerPool.Work(conc); });
        btfWorkerThread = std::thread([this, conc] { btfWorkerPool.Work(conc); });
    }

  public:
    typedef T sampler_type;

    explicit SamplerWrapperIntegrator(sampler_type &&sampler) requires std::is_move_constructible_v<sampler_type>
      : sampler(std::move(sampler)) { StartThreads(); }

    SamplerWrapperIntegrator() requires std::is_default_constructible_v<sampler_type>
      : sampler({}) { StartThreads(); }

    ~SamplerWrapperIntegrator() {
        rendererWorkerPool.Close();
        if (rendererWorkerThread.joinable())
            rendererWorkerThread.join();

        btfWorkerPool.Close();
        if (btfWorkerThread.joinable())
            btfWorkerThread.join();
    }

    void Setup(Math::Vector<size_t, 2> dimensions) {
        frontBuffer = Gfx::Image(dimensions);
        backBuffer = Gfx::Image(dimensions);
    }

    void SetScene(std::shared_ptr<Scene> newScene) {
        scene = std::move(newScene);
    }

    void SetRenderOptions(RenderOptions opts) {
        ResetBackBuffer();
        renderOptions = opts;
    }

    Gfx::Image GetRender() {
        auto workProducer = [this](size_t start, size_t end) {
            return RendererWorkItem{
              .sampler = *this,
              .startRow = start,
              .endRow = end,
            };
        };

        rendererWorkerPool.SplitWork(backBuffer.Height(), 8, workProducer);
        rendererWorkerPool.WGWait();

        framesRendered += 1.;

        btfWorkerPool.SplitWork(backBuffer.Height(), 8, workProducer);
        btfWorkerPool.WGWait();

        return frontBuffer;
    }

  private:
    sampler_type sampler;

    RenderOptions renderOptions{};
    std::shared_ptr<Scene> scene{nullptr};

    Gfx::Image frontBuffer{};
    Gfx::Image backBuffer{};
    Real framesRendered = 0;

    void ResetBackBuffer() {
        framesRendered = 0.;
        backBuffer = Gfx::Image(frontBuffer.dimensions);
    }

    struct RendererWorkItem {
        SamplerWrapperIntegrator<sampler_type, takeAverage> &sampler;
        size_t startRow, endRow;
    };

    static void BackToFrontWorker(RendererWorkItem &&item) noexcept {
        for (size_t row = item.startRow; row != item.endRow; row++) {
            for (size_t col = 0; col < item.sampler.backBuffer.Width(); col++) {
                if constexpr (!takeAverage) {
                    item.sampler.frontBuffer.At({{col, row}}) = item.sampler.backBuffer.At({{col, row}});
                    item.sampler.backBuffer.At({{col, row}}) = {{0}};
                } else {
                    item.sampler.frontBuffer.At({{col, row}}) = item.sampler.backBuffer.At({{col, row}}) / item.sampler.framesRendered;
                }
            }
        }
    }

    static void RendererWorker(RendererWorkItem &&item) noexcept {
        const auto &renderOptions = item.sampler.renderOptions;
        const auto &sampler = item.sampler.sampler;
        const auto &scene = item.sampler.scene;
        auto &backBuffer = item.sampler.backBuffer;

        const Real d = (static_cast<Real>(backBuffer.Width()) / 2) / std::tan(renderOptions.fovWidth / 2.);

        for (size_t row = item.startRow; row != item.endRow; row++) {
            for (std::size_t x = 0; x < backBuffer.Width(); x++) {
                const Real rayX = static_cast<Real>(x)
                                  - static_cast<Real>(backBuffer.Width()) / 2.;

                const Real rayY = static_cast<Real>(backBuffer.Height() - row)
                                  - static_cast<Real>(backBuffer.Height()) / 2.;

                const Ray ray(renderOptions.position, renderOptions.rotation * Math::Normalized(Point{{rayX, rayY, d}}));

                backBuffer.At({{x, row}}) += sampler.Sample(*scene, ray);
            }
        }
    }

    WorkerPoolWG<decltype(&SamplerWrapperIntegrator<sampler_type, takeAverage>::RendererWorker), RendererWorkItem> rendererWorkerPool{&RendererWorker, 16};
    std::thread rendererWorkerThread;
    WorkerPoolWG<decltype(&SamplerWrapperIntegrator<sampler_type, takeAverage>::BackToFrontWorker), RendererWorkItem> btfWorkerPool{&BackToFrontWorker, 16};
    std::thread btfWorkerThread;
};

}
