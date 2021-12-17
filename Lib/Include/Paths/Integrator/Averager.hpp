#pragma once

#include "Integrator.hpp"

namespace Paths {

struct IntegratorAverager final : public Integrator {
    explicit IntegratorAverager(std::unique_ptr<Integrator> integrator);

    ~IntegratorAverager() noexcept override;

    void set_camera(Camera c) noexcept override;

    void set_scene(Scene *s) noexcept override { m_integrator->set_scene(s); }

    void do_render() noexcept override;

    [[nodiscard]] Image::ImageView get_image() noexcept override;

private:
    std::unique_ptr<Integrator> m_integrator { nullptr };
    Image::Image<> m_image_sum {};
    Real m_sample_count = 0;
    Image::Image<> m_image_average {};

    struct WorkItem {
        IntegratorAverager &m_self;
        std::size_t m_start, m_end;
    };

    static void avg_worker_fn(WorkItem &&item) noexcept;

    static void sum_worker_fn(WorkItem &&item) noexcept;

    std::thread m_summer_thread;
    Utils::WorkerPoolWaitGroup<decltype(&IntegratorAverager::avg_worker_fn), WorkItem, ProgramConfig::default_spin>
        m_summer_pool { &IntegratorAverager::sum_worker_fn, ProgramConfig::preferred_thread_count };
    std::thread m_averager_thread;
    Utils::WorkerPoolWaitGroup<decltype(&IntegratorAverager::avg_worker_fn), WorkItem, ProgramConfig::default_spin>
        m_averager_pool { &IntegratorAverager::avg_worker_fn, ProgramConfig::preferred_thread_count };

    void start_threads();
};

}