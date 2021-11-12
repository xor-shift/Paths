#pragma once

#include <atomic>

#include <immintrin.h>

namespace Utils {

struct Spinlock {
    std::atomic<int> impl = 0;

    void Lock() {
        for (;;) {
            if (!impl.exchange(1, std::memory_order_acquire))
                return;
            while (impl.load(std::memory_order_relaxed))
                _mm_pause();
        }
    }

    void Unlock() {
        impl.store(0, std::memory_order_release);
    }
};

}