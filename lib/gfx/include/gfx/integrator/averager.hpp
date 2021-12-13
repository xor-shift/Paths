#pragma once

#include "integrator.hpp"

namespace Gfx {

struct IntegratorAverager final : public Integrator {
    explicit IntegratorAverager(std::unique_ptr<Integrator> integrator);

    ~IntegratorAverager() noexcept override;

    void SetCamera(Camera c) noexcept override;

    void SetScene(Scene *s) noexcept override { integrator->SetScene(s); }

    void Render() noexcept override;

    [[nodiscard]] Image::ImageView GetImage() noexcept override;

  private:
    std::unique_ptr<Integrator> integrator{nullptr};
    Image::Image<> imageSum{};
    Real sampleCount = 0;
    Image::Image<> imageAverage{};

    struct WorkItem { IntegratorAverager &self; std::size_t start, end; };

    static void AvgWorkerFn(WorkItem &&item) noexcept;

    static void SumWorkerFn(WorkItem &&item) noexcept;

    std::thread summerThread;
    Utils::WorkerPoolWG<decltype(&IntegratorAverager::AvgWorkerFn), WorkItem, ProgramConfig::defaultSpin> summerPool{&IntegratorAverager::SumWorkerFn,
                                                                                                                     ProgramConfig::preferredThreadCount};
    std::thread averagerThread;
    Utils::WorkerPoolWG<decltype(&IntegratorAverager::AvgWorkerFn), WorkItem, ProgramConfig::defaultSpin> averagerPool{&IntegratorAverager::AvgWorkerFn,
                                                                                                                       ProgramConfig::preferredThreadCount};

    void StartThreads();
};

}