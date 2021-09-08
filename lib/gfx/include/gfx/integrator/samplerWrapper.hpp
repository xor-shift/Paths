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

template<Concepts::Sampler T>
class SamplerWrapperIntegrator {
  public:
    typedef T sampler_type;

    explicit SamplerWrapperIntegrator(sampler_type &&sampler) requires std::is_move_constructible_v<sampler_type>
      : sampler(std::move(sampler))
        , workerPool(&Worker, 16) {
        workerThread = std::thread([this] {
            const auto conc = std::thread::hardware_concurrency();
            workerPool.Work(conc * 3 / 4);
        });
    }

    ~SamplerWrapperIntegrator() {
        workerPool.Close();
        if (workerThread.joinable())
            workerThread.join();
    }

    void Setup(Math::Vector<size_t, 2> dimensions) {
        frontBuffer = Gfx::Image(dimensions);
        backBuffer = Gfx::Image(dimensions);
    }

    void SetScene(std::shared_ptr<Scene> newScene) {
        scene = std::move(newScene);
    }

    void SetRenderOptions(RenderOptions opts) {
        renderOptions = opts;
    }

    Gfx::Image GetRender() {
        workerPool.SplitWork(backBuffer.Height(), 8, [this](size_t start, size_t end) {
            return WorkItem{
              .sampler = *this,
              .startRow = start,
              .endRow = end,
            };
        });

        workerPool.WGWait();

        return backBuffer;
    }

  private:
    sampler_type sampler;

    RenderOptions renderOptions{};
    std::shared_ptr<Scene> scene{nullptr};

    std::mutex frontBufferMutex{};
    Gfx::Image frontBuffer{};
    Gfx::Image backBuffer{};

    struct WorkItem {
        SamplerWrapperIntegrator<sampler_type> &sampler;
        size_t startRow, endRow;
    };

    static void Worker(WorkItem &&item) {
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

                backBuffer.At({{x, row}}) = sampler.Sample(*scene, ray);
            }
        }
    }

    WorkerPoolWG<decltype(&SamplerWrapperIntegrator<sampler_type>::Worker), WorkItem> workerPool;
    std::thread workerThread;
};

}
