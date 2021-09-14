#pragma once

#include <condition_variable>
#include <future>
#include <list>
#include <thread>

#include <gfx/integrator/integrator.hpp>
#include <gfx/image.hpp>
#include <gfx/scene/scene.hpp>
#include <maths/matvec.hpp>
#include <maths/rand.hpp>
#include <gfx/sampler/samplers.hpp>
#include <utils/workerPool.hpp>

namespace Gfx {

namespace Impl {

enum class FilterType {
    Box,
    Gaussian,
};

struct SWRPConfig {
    bool singleThreaded;
    bool averageFrames;

    FilterType filterType;
    std::array<size_t, 4> gaussianParams{0, 1, 3, 5};
};

template<Concepts::Sampler T, SWRPConfig configuration>
class SamplerWrapper : public Integrator {
  public:
    typedef T sampler_type;

    explicit SamplerWrapper(sampler_type &&sampler) requires std::is_move_constructible_v<sampler_type>
      : sampler(std::move(sampler)) { StartThreads(); }

    SamplerWrapper() requires std::is_default_constructible_v<sampler_type>
      : sampler({}) { StartThreads(); }

    ~SamplerWrapper() {
        rendererWorkerPool.Close();
        if (rendererThread.joinable())
            rendererThread.join();

        btfWorkerPool.Close();
        if (bufferCopyThread.joinable())
            bufferCopyThread.join();
    }

    void SetSize(size_t width, size_t height) override {
        frontBuffer = Gfx::Image(width, height);
        backBuffer = Gfx::Image(width, height);
    }

    void SetScene(std::shared_ptr<Scene> newScene) override {
        scene = std::move(newScene);
    }

    void SetCameraOptions(CameraOptions opts) override {
        ResetBuffers();
        renderOptions = opts;
    }

    [[nodiscard]] const Gfx::Image &GetRender() override {
        auto workProducer = [this](size_t start, size_t end) {
            return WorkItem{
              .self = *this,
              .start = start,
              .end = end,
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

    std::shared_ptr<Scene> scene{nullptr};
    CameraOptions renderOptions;

    Gfx::Image frontBuffer{};
    Gfx::Image backBuffer{};
    Real framesRendered = 0;

    struct WorkItem {
        SamplerWrapper<T, configuration> &self;
        size_t start, end;
    };

    static void BufferCopyWorker(WorkItem &&item) noexcept {
        for (size_t row = item.start; row != item.end; row++) {
            for (size_t col = 0; col < item.self.backBuffer.Width(); col++) {
                if constexpr (configuration.averageFrames) {
                    item.self.frontBuffer.At(col, row) = item.self.backBuffer.At(col, row) / item.self.framesRendered;
                } else {
                    item.self.frontBuffer.At(col, row) = item.self.backBuffer.At(col, row);
                    item.self.backBuffer.At(col, row) = {{0}};
                }
            }
        }
    }

    static std::pair<Real, Real> GetPixXY(size_t width, size_t height, size_t x, size_t y) {
        const Real pixX = static_cast<Real>(x) - static_cast<Real>(width) / 2.;
        const Real pixY = static_cast<Real>(height - y) - static_cast<Real>(height) / 2.;

        if constexpr (configuration.filterType == FilterType::Box) {
            return {pixX, pixY};
        } else if constexpr (configuration.filterType == FilterType::Gaussian) {
            const auto samples = Math::SampleNormalMPCE<
              configuration.gaussianParams[0],
              configuration.gaussianParams[1],
              configuration.gaussianParams[2],
              configuration.gaussianParams[3]>();
            return {pixX + samples[0], pixY + samples[1]};
        }

    }

    static void RendererWorker(WorkItem &&item) noexcept {
        const auto &renderOptions = item.self.renderOptions;
        const auto &sampler = item.self.sampler;
        const auto &scene = item.self.scene;
        auto &backBuffer = item.self.backBuffer;

        const Real d = (static_cast<Real>(backBuffer.Width()) / 2) / std::tan(renderOptions.fovWidth / 2.);

        for (size_t row = item.start; row != item.end; row++) {
            for (std::size_t x = 0; x < backBuffer.Width(); x++) {
                const auto xy = GetPixXY(item.self.frontBuffer.Width(), item.self.frontBuffer.Height(), x, row);

                const Ray ray(renderOptions.position, renderOptions.rotation * Math::Normalized(Point{{xy.first, xy.second, d}}));

                backBuffer.At(x, row) += sampler.Sample(*scene, ray);
            }
        }
    }

    std::thread bufferCopyThread;
    Utils::WorkerPoolWG<decltype(&SamplerWrapper<sampler_type, configuration>::BufferCopyWorker), WorkItem> btfWorkerPool{&BufferCopyWorker, 16};
    std::thread rendererThread;
    Utils::WorkerPoolWG<decltype(&SamplerWrapper<sampler_type, configuration>::RendererWorker), WorkItem> rendererWorkerPool{&RendererWorker, 16};

    void StartThreads() {
        rendererThread = std::thread([this] { rendererWorkerPool.Work(preferredThreadCount); });
        bufferCopyThread = std::thread([this] { btfWorkerPool.Work(preferredThreadCount); });
    }

    void ResetBuffers() {
        if constexpr (configuration.averageFrames) {
            framesRendered = 0.;
            backBuffer.Fill({{0, 0, 0}});
        }
    }
};

}

typedef Impl::SamplerWrapper<Sampler::Whitted, Impl::SWRPConfig{
  .singleThreaded = false,
  .averageFrames = false,
  .filterType = Impl::FilterType::Box,
}> WhittedIntegrator;

typedef Impl::SamplerWrapper<Sampler::PT, Impl::SWRPConfig{
  .singleThreaded = false,
  .averageFrames = true,
  .filterType = Impl::FilterType::Gaussian,
}> MonteCarloIntegrator;

}
