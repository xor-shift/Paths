#include <gfx/integrator/averager.hpp>

namespace Gfx {

IntegratorAverager::IntegratorAverager(std::unique_ptr<Integrator> integrator)
  : integrator(std::move(integrator)) { StartThreads(); }

IntegratorAverager::~IntegratorAverager() noexcept {
    summerPool.Close();
    if (summerThread.joinable())
        summerThread.join();

    averagerPool.Close();
    if (averagerThread.joinable())
        averagerThread.join();
}

void IntegratorAverager::SetCamera(Camera c) noexcept {
    integrator->SetCamera(c);
    imageSum.Resize(c.resolution[0], c.resolution[1]);
    imageAverage.Resize(c.resolution[0], c.resolution[1]);
}

void IntegratorAverager::Render() noexcept {
    integrator->Render();

    summerPool.SplitWork(imageSum.height, preferredThreadCount, [this](size_t start, size_t end) {
        return WorkItem{
          .self = *this,
          .start = start,
          .end = end,
        };
    });
    summerPool.WGWait();

    sampleCount += 1;
}

[[nodiscard]] Image::ImageView IntegratorAverager::GetImage() noexcept {
    averagerPool.SplitWork(imageAverage.height, preferredThreadCount, [this](size_t start, size_t end) {
        return WorkItem{
          .self = *this,
          .start = start,
          .end = end,
        };
    });
    averagerPool.WGWait();

    return static_cast<Image::ImageView>(imageAverage);
}

void IntegratorAverager::AvgWorkerFn(IntegratorAverager::WorkItem &&item) noexcept {
    for (std::size_t y = item.start; y < item.end; y++) {
        const auto offset = y * item.self.imageSum.width;
        const auto *src = item.self.imageSum.cbegin() + offset;
        auto *dst = item.self.imageAverage.begin() + offset;

        for (std::size_t i = 0; i < item.self.imageSum.width; i++) {
            dst[i] = src[i] / item.self.sampleCount;
        }
    }
}

void IntegratorAverager::SumWorkerFn(IntegratorAverager::WorkItem &&item) noexcept {
    auto view = item.self.integrator->GetImage();

    for (std::size_t y = item.start; y < item.end; y++) {
        const auto offset = y * item.self.imageSum.width;
        const auto *src = view.cbegin() + offset;
        auto *dst = item.self.imageSum.begin() + offset;

        for (std::size_t i = 0; i < item.self.imageSum.width; i++)
            dst[i] = dst[i] + src[i];
    }
}

void IntegratorAverager::StartThreads() {
    summerThread = std::thread([this] { summerPool.Work(preferredThreadCount); });
    averagerThread = std::thread([this] { averagerPool.Work(preferredThreadCount); });
}

}
