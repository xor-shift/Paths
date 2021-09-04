#pragma once

#include <condition_variable>
#include <future>
#include <list>
#include <thread>

#include <gfx/concepts/integrator.hpp>
#include <gfx/image.hpp>
#include <gfx/scene.hpp>
#include <utils/workerPool.hpp>

namespace Gfx {

template<Concepts::Sampler Sampler>
class SamplerWrapperIntegrator {
  public:
    explicit SamplerWrapperIntegrator(Sampler &&sampler) requires std::is_move_constructible_v<Sampler>
      : sampler(std::move(sampler))
        , workerPool(&Worker, 16) {
        workerThread = std::thread([this] { workerPool.Work(6); });
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
        const size_t newSegmentSize = 64;

        const size_t nSegments = backBuffer.Height() / newSegmentSize;
        const size_t excessSegments = backBuffer.Height() % newSegmentSize;

        std::vector<std::future<void>> futures(nSegments);

        for (size_t i = 0; i < nSegments; i++) {
            const size_t start = i * newSegmentSize;
            const size_t end = start + newSegmentSize + ((i + 1 == nSegments) ? excessSegments : 0);

            futures[i] = workerPool.QueueWork(WorkItem{
              .sampler = *this,
              .startRow = start,
              .endRow = end,
            });
        }

        for (auto &future : futures) future.wait();

        return backBuffer;
    }

  private:
    Sampler sampler;

    RenderOptions renderOptions{};
    std::shared_ptr<Scene> scene{nullptr};

    std::mutex frontBufferMutex{};
    Gfx::Image frontBuffer{};
    Gfx::Image backBuffer{};

    struct WorkItem {
        SamplerWrapperIntegrator<Sampler> &sampler;
        size_t startRow, endRow;
    };

    static void Worker(WorkItem &&item) {
        const auto &renderOptions = item.sampler.renderOptions;
        const auto &sampler = item.sampler.sampler;
        const auto &scene = item.sampler.scene;
        auto &backBuffer = item.sampler.backBuffer;

        const Real d = (static_cast<Real>(backBuffer.Width()) / 2) / std::tan(renderOptions.fovWidth / 2.);

        const auto ScanRow = [&](size_t row) {
            for (std::size_t x = 0; x < backBuffer.Width(); x++) {
                Real rayX = static_cast<Real>(x);
                rayX -= static_cast<Real>(backBuffer.Width()) / 2.;

                Real rayY = static_cast<Real>(backBuffer.Height() - row);
                rayY -= static_cast<Real>(backBuffer.Height()) / 2.;

                Ray ray(renderOptions.position,
                        Math::Ops::Vector::Normalized(Point{rayX, rayY, d}));

                backBuffer.At({x, row}) = sampler.Sample(*scene, ray);
            }
        };

        for (size_t row = item.startRow; row != item.endRow; row++) {
            ScanRow(row);
        }
    }

    WorkerPool<decltype(&SamplerWrapperIntegrator<Sampler>::Worker), WorkItem> workerPool;
    std::thread workerThread;
};

}
