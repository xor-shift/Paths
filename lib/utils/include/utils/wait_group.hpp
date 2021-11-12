#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

#include <immintrin.h>

namespace Utils {

template<bool spin = false>
class WaitGroup {
  public:
    void Add(int delta) {
        n += delta;
    }

    void Wait() requires (!spin) {
        std::unique_lock lock(wgMutex);
        wgCV.wait(lock, [this] { return n == 0; });
    }

    void Wait() requires spin {
        while (n.load(std::memory_order_relaxed))
            _mm_pause();
    }

    void Done() requires (!spin) {
        n--;
        std::unique_lock lock(wgMutex);
        wgCV.notify_all();
    }

    void Done() requires spin {
        n--;
    }

  private:
    std::mutex wgMutex{};
    std::condition_variable wgCV{};
    std::atomic<int> n{0};
};

}
