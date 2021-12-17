#pragma once

#include "Paths/Integrator/Integrator.hpp"

namespace Paths {

class SamplerWrapperIntegrator : public Integrator {
public:
    SamplerWrapperIntegrator() { start_threads(); }

    ~SamplerWrapperIntegrator() noexcept override {
        m_renderer_pool.close();
        if (m_renderer_thread.joinable())
            m_renderer_thread.join();
    }

    void set_camera(Camera c) noexcept override {
        m_camera = c;
        m_camera.prepare();
        m_back_buffer.resize(c.m_resolution[0], c.m_resolution[1]);
    }

    void set_scene(Scene *s) noexcept override { m_scene = s; }

    void do_render() noexcept override {
        m_renderer_pool.split_work(
            m_camera.m_resolution[1], ProgramConfig::preferred_thread_count, [this](size_t start, size_t end) {
                return WorkItem {
                    .m_self = *this,
                    .m_start = start,
                    .m_end = end,
                };
            });
        m_renderer_pool.wg_wait();
    }

    [[nodiscard]] Image::ImageView get_image() noexcept override {
        return static_cast<Image::ImageView>(m_back_buffer);
    }

protected:
    [[nodiscard]] virtual Color sample(Ray, Scene &) const noexcept { return {}; };

private:
    Scene *m_scene { nullptr };
    Camera m_camera {};
    Image::Image<> m_back_buffer {};

    struct WorkItem {
        SamplerWrapperIntegrator &m_self;
        std::size_t m_start, m_end;
    };

    static void worker_fn(WorkItem &&item) noexcept {
        for (std::size_t i = item.m_start; i < item.m_end; i++)
            item.m_self.integrate_line(i);
    }

    std::thread m_renderer_thread;
    Utils::WorkerPoolWaitGroup<decltype(&SamplerWrapperIntegrator::worker_fn), WorkItem, ProgramConfig::default_spin>
        m_renderer_pool { &SamplerWrapperIntegrator::worker_fn, ProgramConfig::preferred_thread_count };

    void start_threads() {
        m_renderer_thread = std::thread([this] { m_renderer_pool.do_work(ProgramConfig::preferred_thread_count); });
        // bufferCopyThread = std::thread([this] { btfWorkerPool.Work(preferredThreadCount); });
    }

    void integrate_line(std::size_t y) noexcept {
        for (std::size_t x = 0; x < m_camera.m_resolution[0]; x++) {
            const auto ray = m_camera.make_ray(x, y);
            m_back_buffer.at(x, y) = sample(ray, *m_scene);
        }
    }
};

}