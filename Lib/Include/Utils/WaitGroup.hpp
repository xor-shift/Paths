#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

#include <immintrin.h>

namespace Utils {

template<bool spin = false> class WaitGroup {
    std::mutex m_wg_mutex {};
    std::condition_variable m_wg_cv {};
    std::atomic<int> m_n { 0 };

public:
    void add(int delta) { m_n += delta; }

    void wait() requires(!spin) {
        std::unique_lock lock(m_wg_mutex);
        m_wg_cv.wait(lock, [this] { return m_n == 0; });
    }

    void wait() requires spin {
        while (m_n.load(std::memory_order_relaxed) > 0)
            _mm_pause();
    }

    void done() requires(!spin) {
        m_n--;
        std::unique_lock lock(m_wg_mutex);
        m_wg_cv.notify_all();
    }

    void done() requires spin { m_n--; }
};

}
