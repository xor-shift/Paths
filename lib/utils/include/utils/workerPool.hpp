#pragma once

#include <functional>
#include <future>
#include <thread>
#include <utility>

#include "bufChan.hpp"

/**
 * A worker pool of sorts
 * @tparam WorkItem The argument type that the worker function takes in
 * @tparam PromiseType The return type of the worker function, will be wrapped in a future
 */
template<typename Fn, typename WorkItem, typename PromiseType = void>
class WorkerPool {
    typedef std::pair<WorkItem, std::promise<PromiseType>> ChanType;

  public:
    /**
     * Creates a worker pool, no workers will be launched, see Work
     * @tparam Callable
     * @param fn The worker function, will be stored in a std::function
     * @param bufSize The work queue size, don't set this less than the number of threads you intend to create
     */
    explicit WorkerPool(Fn &&fn, size_t bufSize)
      : fn(fn)
        , workItemChan(bufSize) {}

    /**
     * Queues some work, will block if the queue is full
     * @param item The item to queue
     * @return The future to the return data
     */
    std::future<PromiseType> QueueWork(WorkItem item) {
        ChanType workItem{
          std::move(item),
          std::promise<PromiseType>{},
        };

        auto future = workItem.second.get_future();

        if (!workItemChan.EmplaceBack(std::move(workItem))) return {};

        return future;
    }

    /**
     * Starts worker threads and immediately joins them
     * Can be called multiple times to create multiple thread pools
     * @param nThreads The number of threads to launch
     */
    void Work(size_t nThreads = 1) {
        std::vector<std::thread> workerThreads(nThreads);

        for (auto &thread : workerThreads) {
            thread = std::thread([this] {
                for (;;) {
                    auto workOpt = workItemChan.Get();
                    if (!workOpt) break;

                    auto &&[work, future] = *workOpt;

                    if constexpr (std::is_same_v<PromiseType, void>) {
                        fn(std::forward<WorkItem>(work));
                        future.set_value();
                    } else {
                        auto res = fn(std::forward<WorkItem>(work));
                        future.set_value(res);
                    }
                }
            });
        }

        for (auto &thread : workerThreads) {
            if (thread.joinable())
                thread.join();
        }
    }

    /**
     * Closes the work queue channel, effectively shutting down the pool, do not call while there is work queued, may result in UB
     */
    void Close() {
        workItemChan.Close();
    }

  private:
    Fn fn;
    BufChan<ChanType> workItemChan;
};
