#pragma once

#include <atomic>
#include <condition_variable>
#include <list>
#include <optional>
#include <stdexcept>

#include "CircularBuffer.hpp"

namespace Utils {

/**
 * A class that imitates golang's buffered channels
 * @tparam T The type to buffer
 */
template<typename T> class BufChan {
public:
    /**
     * Creates a buffered channel
     * @param maxBufferSize The buffer size
     */
    explicit BufChan(size_t size)
        : m_circ_buffer(size) { }

    ~BufChan() { close(); }

    BufChan &operator<<(T data) {
        if (!push(std::move(data))) {
            // throw std::runtime_error("Sent data to closed channel");
        }

        return *this;
    }

    [[nodiscard]] bool push(const T &data) { return push(std::move(T(data))); }

    [[nodiscard]] bool push(T &&data) {
        std::unique_lock buffer_lock(m_buffer_mutex);

        if (m_closed)
            return false;

        if (full()) {
            m_output_cv.wait(buffer_lock, [this] { return !full() || m_closed; });
        }

        if (m_closed)
            return false;

        m_circ_buffer.push_back(std::move(data));
        m_input_cv.notify_one();

        return true;
    }

    std::optional<T> get() {
        std::unique_lock buffer_lock(m_buffer_mutex);

        if (m_closed)
            return std::nullopt;

        if (empty()) {
            m_input_cv.wait(buffer_lock, [this] { return !empty() || m_closed; });
        }

        if (m_closed)
            return std::nullopt;

        T val = std::move(m_circ_buffer.front());
        m_circ_buffer.pop_front();
        m_output_cv.notify_one();

        return val;
    }

    [[nodiscard]] bool full() const { return m_circ_buffer.full(); }

    [[nodiscard]] bool empty() const { return m_circ_buffer.empty(); }

    void close() {
        std::unique_lock buffer_lock(m_buffer_mutex);
        m_closed = true;
        m_output_cv.notify_all();
        m_input_cv.notify_all();
    }

private:
    std::atomic_bool m_closed { false };
    std::mutex m_buffer_mutex {};
    CircularBuffer<T> m_circ_buffer;

    std::condition_variable m_input_cv {};
    std::condition_variable m_output_cv {};
};

}