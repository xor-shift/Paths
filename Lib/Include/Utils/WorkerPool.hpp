#pragma once

#include <functional>
#include <future>
#include <shared_mutex>
#include <thread>
#include <utility>

#include "BufferedChannel.hpp"
#include "WaitGroup.hpp"

namespace Utils {

/// WaitGroup based worker pool
/// \tparam Fn
/// \tparam WorkItem
/// \tparam spin
template<typename Fn, typename WorkItem, bool spin = false> class WorkerPoolWaitGroup {
    typedef WorkItem ChanType;

public:
    /**
     * Constructs a worker pool, does not start working
     * @param fn
     * @param buf_size
     */
    explicit WorkerPoolWaitGroup(Fn &&fn, size_t buf_size)
        : m_worker_fn(fn)
        , m_work_item_chan(buf_size) { }

    void queue_work(WorkItem item) {
        if (!m_work_item_chan.push(std::move(item))) { }
    }

    void do_work(size_t n_threads = 1) {
        auto lambda = [this] {
            for (;;) {
                auto work_opt = m_work_item_chan.get();
                if (!work_opt)
                    break;

                WorkItem work = *work_opt;
                std::invoke(m_worker_fn, std::move(work));
                m_wg.done();
            }
        };

        if (n_threads == 1) {
            lambda();
        } else {
            std::vector<std::thread> worker_threads(n_threads);

            for (auto &thread : worker_threads) {
                thread = std::thread(lambda);
            }

            for (auto &thread : worker_threads) {
                if (thread.joinable())
                    thread.join();
            }
        }
    }

    void close() { m_work_item_chan.close(); }

    template<typename Callable>
    void split_work(size_t work_size, size_t work_divide, Callable &&work_item_generator) requires(
        std::is_same_v<std::invoke_result_t<Callable, size_t, size_t>, WorkItem>) {
        const size_t n_segments = work_size / work_divide;
        const size_t excess_segments = work_size % work_divide;

        wg_delta(n_segments);

        for (size_t i = 0; i < n_segments; i++) {
            const size_t start = i * work_divide;
            const size_t end = start + work_divide + ((i + 1 == n_segments) ? excess_segments : 0);
            queue_work(std::invoke(work_item_generator, start, end));
        }
    }

    void wg_delta(size_t delta) { m_wg.add(delta); }

    void wg_wait() { m_wg.wait(); }

private:
    Fn m_worker_fn;
    BufChan<ChanType> m_work_item_chan;
    WaitGroup<spin> m_wg {};

    std::vector<std::thread> m_threads;
};

}
