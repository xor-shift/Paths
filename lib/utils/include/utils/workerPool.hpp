#pragma once

#include <functional>
#include <future>
#include <shared_mutex>
#include <thread>
#include <utility>

#include "bufChan.hpp"
#include "waitGroup.hpp"

namespace Utils {

/**
 * A WaitGroup based worker pool
 * @tparam Fn The callback to call for working
 * @tparam WorkItem The data type to work with
 */
template<typename Fn, typename WorkItem>
class WorkerPoolWG {
    typedef WorkItem ChanType;
  public:
    /**
     * Constructs a worker pool, does not start working
     * @param fn
     * @param bufSize
     */
    explicit WorkerPoolWG(Fn &&fn, size_t bufSize)
      : fn(fn)
        , workItemChan(bufSize) {}

    void QueueWork(WorkItem item) {
        if (!workItemChan.EmplaceBack(std::move(item))) {}
    }

    void Work(size_t nThreads = 1) {
        auto lambda = [this] {
            for (;;) {
                auto workOpt = workItemChan.Get();
                if (!workOpt) break;

                WorkItem work = *workOpt;
                std::invoke(fn, std::move(work));
                wg.Done();
            }
        };

        if (nThreads == 1) {
            lambda();
        } else {
            std::vector<std::thread> workerThreads(nThreads);

            for (auto &thread: workerThreads) {
                thread = std::thread(lambda);
            }

            for (auto &thread: workerThreads) {
                if (thread.joinable())
                    thread.join();
            }
        }
    }

    void Close() {
        workItemChan.Close();
    }

    template<typename Callable>
    void SplitWork(size_t workSize, size_t workDivide, Callable &&wiGen) requires(std::is_same_v<std::invoke_result_t<Callable, size_t, size_t>, WorkItem>) {
        const size_t nSegments = workSize / workDivide;
        const size_t excessSegments = workSize % workDivide;

        WGDelta(nSegments);

        for (size_t i = 0; i < nSegments; i++) {
            const size_t start = i * workDivide;
            const size_t end = start + workDivide + ((i + 1 == nSegments) ? excessSegments : 0);
            QueueWork(wiGen(start, end));
        }
    }

    void WGDelta(size_t delta) { wg.Add(delta); }

    void WGWait() { wg.Wait(); }

  private:
    Fn fn;
    BufChan<ChanType> workItemChan;
    WaitGroup wg{};
};

}
