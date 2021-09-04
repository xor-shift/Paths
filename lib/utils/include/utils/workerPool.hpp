#pragma once

#include <functional>
#include <future>
#include <shared_mutex>
#include <thread>
#include <utility>

#include "bufChan.hpp"
#include "waitGroup.hpp"

/*
template<typename Fn, typename WorkItem, typename PromisedType = void>
class WorkerPoolFuture {
    typedef std::pair<WorkItem, std::promise<PromisedType>> ChanType;

  public:
    explicit WorkerPoolFuture(Fn &&fn, size_t bufSize)
      : fn(fn)
        , workItemChan(bufSize) {}

    std::future<PromisedType> QueueWork(WorkItem item) {
        ChanType workItem{
          std::move(item),
          std::promise<PromisedType>{},
        };

        auto future = workItem.second.get_future();

        if (!workItemChan.EmplaceBack(std::move(workItem))) return {};

        return future;
    }

    void Work(size_t nThreads = 1) {
        std::vector<std::thread> workerThreads(nThreads);

        for (auto &thread : workerThreads) {
            thread = std::thread([this] {
                for (;;) {
                    auto workOpt = workItemChan.Get();
                    if (!workOpt) break;

                    auto &&[work, future] = *workOpt;

                    if constexpr (std::is_same_v<PromisedType, void>) {
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

    void Close() {
        workItemChan.Close();
    }

    template<typename Callable>
    std::vector<std::future<PromisedType>> SplitWork(size_t workSize, size_t workDivide, Callable &&wiGen) requires(std::is_same_v<std::invoke_result_t<Callable, size_t, size_t>, WorkItem>) {
        const size_t nSegments = workSize / workDivide;
        const size_t excessSegments = workSize & workDivide;

        std::vector<std::future<PromisedType>> futures(nSegments);

        for (size_t i = 0; i < nSegments; i++) {
            const size_t start = i * workDivide;
            const size_t end = start + workDivide + ((i + 1 == nSegments) ? excessSegments : 0);
            futures.at(i) = std::move(QueueWork(wiGen(start, end)));
        }

        return futures;
    }

  private:
    Fn fn;
    BufChan<ChanType> workItemChan;
};*/

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
        std::vector<std::thread> workerThreads(nThreads);

        for (auto &thread : workerThreads) {
            thread = std::thread([this] {
                for (;;) {
                    auto workOpt = workItemChan.Get();
                    if (!workOpt) break;

                    auto work = std::move(*workOpt);
                    fn(std::move(work));
                    wg.Done();
                }
            });
        }

        for (auto &thread : workerThreads) {
            if (thread.joinable())
                thread.join();
        }
    }

    void Close() {
        workItemChan.Close();
    }

    template<typename Callable>
    void SplitWork(size_t workSize, size_t workDivide, Callable &&wiGen) requires(std::is_same_v<std::invoke_result_t<Callable, size_t, size_t>, WorkItem>) {
        const size_t nSegments = workSize / workDivide;
        const size_t excessSegments = workSize & workDivide;

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
