#pragma once

#include <atomic>

#include <immintrin.h>

namespace Utils {

struct Spinlock {
    std::atomic<int> m_impl = 0;

    void lock() {
        for (;;) {
            if (!m_impl.exchange(1, std::memory_order_acquire))
                return;
            while (m_impl.load(std::memory_order_relaxed))
                _mm_pause();
        }
    }

    void unlock() { m_impl.store(0, std::memory_order_release); }
};

}