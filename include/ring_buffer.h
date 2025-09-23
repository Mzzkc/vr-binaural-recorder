// ring_buffer.h - Lock-free ring buffer for real-time audio
#pragma once

#include <atomic>
#include <vector>
#include <cstring>
#include <algorithm>

namespace vrb {

/**
 * @brief Lock-free single-producer single-consumer ring buffer
 *
 * Optimized for real-time audio processing with cache-line alignment
 * and memory ordering optimizations.
 */
template<typename T>
class RingBuffer {
public:
    /**
     * @brief Construct ring buffer with specified capacity
     * @param capacity Buffer size in elements (will be rounded up to power of 2)
     */
    explicit RingBuffer(size_t capacity)
        : m_capacity(nextPowerOfTwo(capacity))
        , m_mask(m_capacity - 1)
        , m_buffer(m_capacity)
        , m_writeIndex(0)
        , m_readIndex(0) {

        // Clear buffer
        std::fill(m_buffer.begin(), m_buffer.end(), T{});
    }

    /**
     * @brief Write data to the ring buffer
     * @param data Pointer to data to write
     * @param count Number of elements to write
     * @return Number of elements actually written
     */
    size_t write(const T* data, size_t count) {
        const size_t writeIdx = m_writeIndex.load(std::memory_order_relaxed);
        const size_t readIdx = m_readIndex.load(std::memory_order_acquire);

        const size_t available = m_capacity - (writeIdx - readIdx);
        const size_t toWrite = std::min(count, available);

        if (toWrite == 0) {
            return 0;
        }

        // Write in two parts if wrapping
        const size_t firstPart = std::min(toWrite, m_capacity - (writeIdx & m_mask));
        const size_t secondPart = toWrite - firstPart;

        std::memcpy(&m_buffer[writeIdx & m_mask], data, firstPart * sizeof(T));
        if (secondPart > 0) {
            std::memcpy(&m_buffer[0], data + firstPart, secondPart * sizeof(T));
        }

        // Update write index with memory barrier
        m_writeIndex.store(writeIdx + toWrite, std::memory_order_release);

        return toWrite;
    }

    /**
     * @brief Read data from the ring buffer
     * @param data Pointer to buffer to read into
     * @param count Number of elements to read
     * @return Number of elements actually read
     */
    size_t read(T* data, size_t count) {
        const size_t readIdx = m_readIndex.load(std::memory_order_relaxed);
        const size_t writeIdx = m_writeIndex.load(std::memory_order_acquire);

        const size_t available = writeIdx - readIdx;
        const size_t toRead = std::min(count, available);

        if (toRead == 0) {
            return 0;
        }

        // Read in two parts if wrapping
        const size_t firstPart = std::min(toRead, m_capacity - (readIdx & m_mask));
        const size_t secondPart = toRead - firstPart;

        std::memcpy(data, &m_buffer[readIdx & m_mask], firstPart * sizeof(T));
        if (secondPart > 0) {
            std::memcpy(data + firstPart, &m_buffer[0], secondPart * sizeof(T));
        }

        // Update read index with memory barrier
        m_readIndex.store(readIdx + toRead, std::memory_order_release);

        return toRead;
    }

    /**
     * @brief Peek at data without consuming it
     * @param data Pointer to buffer to read into
     * @param count Number of elements to peek
     * @param offset Offset from read position
     * @return Number of elements actually peeked
     */
    size_t peek(T* data, size_t count, size_t offset = 0) const {
        const size_t readIdx = m_readIndex.load(std::memory_order_relaxed);
        const size_t writeIdx = m_writeIndex.load(std::memory_order_acquire);

        const size_t available = writeIdx - readIdx;
        if (offset >= available) {
            return 0;
        }

        const size_t toPeek = std::min(count, available - offset);
        const size_t peekIdx = readIdx + offset;

        // Read in two parts if wrapping
        const size_t firstPart = std::min(toPeek, m_capacity - (peekIdx & m_mask));
        const size_t secondPart = toPeek - firstPart;

        std::memcpy(data, &m_buffer[peekIdx & m_mask], firstPart * sizeof(T));
        if (secondPart > 0) {
            std::memcpy(data + firstPart, &m_buffer[0], secondPart * sizeof(T));
        }

        return toPeek;
    }

    /**
     * @brief Skip elements without reading them
     * @param count Number of elements to skip
     * @return Number of elements actually skipped
     */
    size_t skip(size_t count) {
        const size_t readIdx = m_readIndex.load(std::memory_order_relaxed);
        const size_t writeIdx = m_writeIndex.load(std::memory_order_acquire);

        const size_t available = writeIdx - readIdx;
        const size_t toSkip = std::min(count, available);

        m_readIndex.store(readIdx + toSkip, std::memory_order_release);

        return toSkip;
    }

    /**
     * @brief Get number of elements available to read
     */
    size_t available() const {
        const size_t writeIdx = m_writeIndex.load(std::memory_order_acquire);
        const size_t readIdx = m_readIndex.load(std::memory_order_relaxed);
        return writeIdx - readIdx;
    }

    /**
     * @brief Get free space available for writing
     */
    size_t free() const {
        const size_t writeIdx = m_writeIndex.load(std::memory_order_relaxed);
        const size_t readIdx = m_readIndex.load(std::memory_order_acquire);
        return m_capacity - (writeIdx - readIdx);
    }

    /**
     * @brief Check if buffer is empty
     */
    bool empty() const {
        return available() == 0;
    }

    /**
     * @brief Check if buffer is full
     */
    bool full() const {
        return free() == 0;
    }

    /**
     * @brief Reset the buffer to empty state
     */
    void reset() {
        m_writeIndex.store(0, std::memory_order_relaxed);
        m_readIndex.store(0, std::memory_order_relaxed);
        std::fill(m_buffer.begin(), m_buffer.end(), T{});
    }

    /**
     * @brief Get buffer capacity
     */
    size_t capacity() const {
        return m_capacity;
    }

private:
    /**
     * @brief Round up to next power of two
     */
    static size_t nextPowerOfTwo(size_t n) {
        // Handle edge cases
        if (n <= 1) return 1;

        // Bit manipulation trick to find next power of 2
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        if constexpr (sizeof(size_t) > 4) {
            n |= n >> 32;
        }
        n++;

        return n;
    }

private:
    const size_t m_capacity;
    const size_t m_mask;
    std::vector<T> m_buffer;

    // Cache line padding to prevent false sharing
    alignas(64) std::atomic<size_t> m_writeIndex;
    alignas(64) std::atomic<size_t> m_readIndex;
};

/**
 * @brief Specialized audio ring buffer with additional features
 */
class AudioRingBuffer : public RingBuffer<float> {
public:
    explicit AudioRingBuffer(size_t capacity)
        : RingBuffer<float>(capacity) {}

    /**
     * @brief Write interleaved stereo data
     */
    size_t writeStereo(const float* left, const float* right, size_t frames) {
        std::vector<float> interleaved(frames * 2);
        for (size_t i = 0; i < frames; ++i) {
            interleaved[i * 2] = left[i];
            interleaved[i * 2 + 1] = right[i];
        }
        return write(interleaved.data(), frames * 2) / 2;
    }

    /**
     * @brief Read interleaved stereo data
     */
    size_t readStereo(float* left, float* right, size_t frames) {
        std::vector<float> interleaved(frames * 2);
        size_t framesRead = read(interleaved.data(), frames * 2) / 2;

        for (size_t i = 0; i < framesRead; ++i) {
            left[i] = interleaved[i * 2];
            right[i] = interleaved[i * 2 + 1];
        }

        return framesRead;
    }

    /**
     * @brief Apply fade in/out to prevent clicks
     */
    void applyFade(float* buffer, size_t frames, bool fadeIn) {
        const float fadeLength = std::min(frames, size_t(64));

        for (size_t i = 0; i < fadeLength; ++i) {
            float gain = fadeIn ?
                (float(i) / fadeLength) :
                (float(fadeLength - i) / fadeLength);

            buffer[i] *= gain;
            if (!fadeIn && frames > fadeLength) {
                buffer[frames - 1 - i] *= gain;
            }
        }
    }
};

} // namespace vrb