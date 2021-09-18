#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

namespace Utils {

class WaitGroup {
  public:
    void Add(size_t delta) {
        std::unique_lock lock(wgMutex);
        n += delta;
    }

    void Wait() {
        std::unique_lock lock(wgMutex);
        wgCV.wait(lock, [this] { return n == 0; });
    }

    void Done() {
        std::unique_lock lock(wgMutex);
        n--;
        wgCV.notify_all();
    }

  private:
    std::mutex wgMutex{};
    std::condition_variable wgCV{};
    std::atomic_size_t n{0};
};

}
