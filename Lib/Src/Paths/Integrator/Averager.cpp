#include "Paths/Integrator/Averager.hpp"

namespace Paths {

IntegratorAverager::IntegratorAverager(std::unique_ptr<Integrator> integrator)
    : m_integrator(std::move(integrator)) {
    start_threads();
}

IntegratorAverager::~IntegratorAverager() noexcept {
    m_summer_pool.close();
    if (m_summer_thread.joinable())
        m_summer_thread.join();

    m_averager_pool.close();
    if (m_averager_thread.joinable())
        m_averager_thread.join();
}

void IntegratorAverager::set_camera(Camera c) noexcept {
    m_integrator->set_camera(c);
    m_image_sum.resize(c.m_resolution[0], c.m_resolution[1]);
    m_image_average.resize(c.m_resolution[0], c.m_resolution[1]);
}

void IntegratorAverager::do_render() noexcept {
    m_integrator->do_render();

    m_summer_pool.split_work(
        m_image_sum.m_height, ProgramConfig::preferred_thread_count, [this](size_t start, size_t end) {
            return WorkItem {
                .m_self = *this,
                .m_start = start,
                .m_end = end,
            };
        });
    m_summer_pool.wg_wait();

    m_sample_count += 1;
}

[[nodiscard]] Image::ImageView IntegratorAverager::get_image() noexcept {
    m_averager_pool.split_work(
        m_image_average.m_height, ProgramConfig::preferred_thread_count, [this](size_t start, size_t end) {
            return WorkItem {
                .m_self = *this,
                .m_start = start,
                .m_end = end,
            };
        });
    m_averager_pool.wg_wait();

    return static_cast<Image::ImageView>(m_image_average);
}

void IntegratorAverager::avg_worker_fn(IntegratorAverager::WorkItem &&item) noexcept {
    for (std::size_t y = item.m_start; y < item.m_end; y++) {
        const auto offset = y * item.m_self.m_image_sum.m_width;
        const auto *src = item.m_self.m_image_sum.cbegin() + offset;
        auto *dst = item.m_self.m_image_average.begin() + offset;

        for (std::size_t i = 0; i < item.m_self.m_image_sum.m_width; i++) {
            dst[i] = src[i] / item.m_self.m_sample_count;
        }
    }
}

void IntegratorAverager::sum_worker_fn(IntegratorAverager::WorkItem &&item) noexcept {
    auto view = item.m_self.m_integrator->get_image();

    for (std::size_t y = item.m_start; y < item.m_end; y++) {
        const auto offset = y * item.m_self.m_image_sum.m_width;
        const auto *src = view.cbegin() + offset;
        auto *dst = item.m_self.m_image_sum.begin() + offset;

        for (std::size_t i = 0; i < item.m_self.m_image_sum.m_width; i++)
            dst[i] = dst[i] + src[i];
    }
}

void IntegratorAverager::start_threads() {
    m_summer_thread = std::thread([this] { m_summer_pool.do_work(ProgramConfig::preferred_thread_count); });
    m_averager_thread = std::thread([this] { m_averager_pool.do_work(ProgramConfig::preferred_thread_count); });
}

}
