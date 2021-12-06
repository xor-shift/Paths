#pragma once

#include <atomic>
#include <condition_variable>
#include <list>
#include <optional>
#include <stdexcept>

#include "circular_buffer.hpp"

namespace Utils {

/**
 * A class that imitates golang's buffered channels
 * @tparam T The type to buffer
 */
template<typename T>
class BufChan {
  public:
    /**
     * Creates a buffered channel
     * @param maxBufferSize The buffer size
     */
    explicit BufChan(size_t size)
      : circBuffer(size) {}

    ~BufChan() {
        Close();
    }

    BufChan &operator<<(T data) {
        if (!EmplaceBack(std::move(data))) {
            //throw std::runtime_error("Sent data to closed channel");
        }

        return *this;
    }

    [[nodiscard]] bool Push(T data) {
        return EmplaceBack(std::move(data));
    }

    [[nodiscard]] bool EmplaceBack(T &&data) {
        std::unique_lock bufferLock(bufferMutex);

        if (closed) return false;

        if (Full()) {
            outputCV.wait(bufferLock, [this] { return !Full() || closed; });
        }

        if (closed) return false;

        circBuffer.push_back(std::move(data));
        inputCV.notify_one();

        return true;
    }

    std::optional<T> Get() {
        std::unique_lock bufferLock(bufferMutex);

        if (closed) return std::nullopt;

        if (Empty()) {
            inputCV.wait(bufferLock, [this] { return !Empty() || closed; });
        }

        if (closed) return std::nullopt;

        T val = std::move(circBuffer.front());
        circBuffer.pop_front();
        outputCV.notify_one();

        return val;
    }

    [[nodiscard]] bool Full() const {
        return circBuffer.full();
    }

    [[nodiscard]] bool Empty() const {
        return circBuffer.empty();
    }

    void Close() {
        std::unique_lock bufferLock(bufferMutex);
        closed = true;
        outputCV.notify_all();
        inputCV.notify_all();
    }

  private:
    std::atomic_bool closed{false};
    std::mutex bufferMutex{};
    CircularBuffer<T> circBuffer;

    std::condition_variable inputCV{};
    std::condition_variable outputCV{};
};

}