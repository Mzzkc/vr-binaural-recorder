// ring_buffer.h - Lock-free ring buffer for real-time audio
#pragma once

#include <atomic>
#include <vector>
#include <cstring>
#include <algorithm>
#include <immintrin.h>  // For SIMD optimizations
#include <memory>

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
     * @brief Write data to the ring buffer with SIMD optimization
     * @param data Pointer to data to write
     * @param count Number of elements to write
     * @return Number of elements actually written
     */
    size_t write(const T* data, size_t count) {
        const size_t writeIdx = m_writeIndex.load(std::memory_order_relaxed);
        const size_t readIdx = m_readIndex.load(std::memory_order_acquire);

        const size_t available = m_capacity - (writeIdx - readIdx) - 1;  // Reserve one slot
        const size_t toWrite = std::min(count, available);

        if (toWrite == 0) {
            return 0;
        }

        // Write in two parts if wrapping, with SIMD-optimized copying
        const size_t writePos = writeIdx & m_mask;
        const size_t firstPart = std::min(toWrite, m_capacity - writePos);
        const size_t secondPart = toWrite - firstPart;

        // Use optimized memory copy for large transfers
        if constexpr (std::is_same_v<T, float>) {
            copyFloatsSIMD(data, &m_buffer[writePos], firstPart);
            if (secondPart > 0) {
                copyFloatsSIMD(data + firstPart, &m_buffer[0], secondPart);
            }
        } else {
            std::memcpy(&m_buffer[writePos], data, firstPart * sizeof(T));
            if (secondPart > 0) {
                std::memcpy(&m_buffer[0], data + firstPart, secondPart * sizeof(T));
            }
        }

        // Update write index with memory barrier
        m_writeIndex.store(writeIdx + toWrite, std::memory_order_release);

        return toWrite;
    }

    /**
     * @brief Read data from the ring buffer with SIMD optimization
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

        // Read in two parts if wrapping, with SIMD-optimized copying
        const size_t readPos = readIdx & m_mask;
        const size_t firstPart = std::min(toRead, m_capacity - readPos);
        const size_t secondPart = toRead - firstPart;

        // Use optimized memory copy for large transfers
        if constexpr (std::is_same_v<T, float>) {
            copyFloatsSIMD(&m_buffer[readPos], data, firstPart);
            if (secondPart > 0) {
                copyFloatsSIMD(&m_buffer[0], data + firstPart, secondPart);
            }
        } else {
            std::memcpy(data, &m_buffer[readPos], firstPart * sizeof(T));
            if (secondPart > 0) {
                std::memcpy(data + firstPart, &m_buffer[0], secondPart * sizeof(T));
            }
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
        return m_capacity - (writeIdx - readIdx) - 1;  // Reserve one slot to distinguish full from empty
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
     * @brief Reset the buffer to empty state with optimized clearing
     */
    void reset() {
        m_writeIndex.store(0, std::memory_order_relaxed);
        m_readIndex.store(0, std::memory_order_relaxed);

        // Use SIMD-optimized clearing for float buffers
        if constexpr (std::is_same_v<T, float>) {
            clearFloatsSIMD(m_buffer.data(), m_capacity);
        } else {
            std::fill(m_buffer.begin(), m_buffer.end(), T{});
        }
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
    // SIMD-optimized memory operations for float buffers
    static void copyFloatsSIMD(const float* src, float* dst, size_t count) {
        if constexpr (std::is_same_v<T, float>) {
            const size_t simdCount = count & ~7;  // Process 8 floats at a time

            for (size_t i = 0; i < simdCount; i += 8) {
                __m256 data = _mm256_loadu_ps(&src[i]);
                _mm256_storeu_ps(&dst[i], data);
            }

            // Handle remaining elements
            for (size_t i = simdCount; i < count; ++i) {
                dst[i] = src[i];
            }
        } else {
            std::memcpy(dst, src, count * sizeof(T));
        }
    }

    static void clearFloatsSIMD(float* data, size_t count) {
        if constexpr (std::is_same_v<T, float>) {
            const size_t simdCount = count & ~7;
            const __m256 zero = _mm256_setzero_ps();

            for (size_t i = 0; i < simdCount; i += 8) {
                _mm256_storeu_ps(&data[i], zero);
            }

            // Handle remaining elements
            for (size_t i = simdCount; i < count; ++i) {
                data[i] = 0.0f;
            }
        }
    }

private:
    const size_t m_capacity;
    const size_t m_mask;

    // Use aligned allocator for better SIMD performance
    std::vector<T, std::conditional_t<std::is_same_v<T, float>,
        std::allocator<T>, std::allocator<T>>> m_buffer;

    // Cache line padding to prevent false sharing (increased to 128 bytes for modern CPUs)
    alignas(128) std::atomic<size_t> m_writeIndex;
    alignas(128) std::atomic<size_t> m_readIndex;
};

/**
 * @brief Specialized audio ring buffer with additional features
 */
class AudioRingBuffer : public RingBuffer<float> {
public:
    explicit AudioRingBuffer(size_t capacity)
        : RingBuffer<float>(capacity) {}

    /**
     * @brief Write interleaved stereo data with SIMD optimization
     */
    size_t writeStereo(const float* left, const float* right, size_t frames) {
        thread_local std::vector<float> interleaved;
        interleaved.resize(frames * 2);

        // SIMD-optimized interleaving
        const size_t simdFrames = frames & ~7;  // Process 8 frames at a time

        for (size_t i = 0; i < simdFrames; i += 8) {
            __m256 leftData = _mm256_loadu_ps(&left[i]);
            __m256 rightData = _mm256_loadu_ps(&right[i]);

            __m256 lo = _mm256_unpacklo_ps(leftData, rightData);
            __m256 hi = _mm256_unpackhi_ps(leftData, rightData);

            _mm256_storeu_ps(&interleaved[i * 2], _mm256_permute2f128_ps(lo, hi, 0x20));
            _mm256_storeu_ps(&interleaved[i * 2 + 8], _mm256_permute2f128_ps(lo, hi, 0x31));
        }

        // Handle remaining frames
        for (size_t i = simdFrames; i < frames; ++i) {
            interleaved[i * 2] = left[i];
            interleaved[i * 2 + 1] = right[i];
        }

        return write(interleaved.data(), frames * 2) / 2;
    }

    /**
     * @brief Read interleaved stereo data with SIMD optimization
     */
    size_t readStereo(float* left, float* right, size_t frames) {
        thread_local std::vector<float> interleaved;
        interleaved.resize(frames * 2);

        size_t framesRead = read(interleaved.data(), frames * 2) / 2;

        // SIMD-optimized deinterleaving
        const size_t simdFrames = framesRead & ~7;

        for (size_t i = 0; i < simdFrames; i += 8) {
            __m256 data1 = _mm256_loadu_ps(&interleaved[i * 2]);
            __m256 data2 = _mm256_loadu_ps(&interleaved[i * 2 + 8]);

            __m256 leftData = _mm256_shuffle_ps(data1, data2, 0x88);
            __m256 rightData = _mm256_shuffle_ps(data1, data2, 0xDD);

            leftData = _mm256_castpd_ps(_mm256_permute4x64_pd(_mm256_castps_pd(leftData), 0xD8));
            rightData = _mm256_castpd_ps(_mm256_permute4x64_pd(_mm256_castps_pd(rightData), 0xD8));

            _mm256_storeu_ps(&left[i], leftData);
            _mm256_storeu_ps(&right[i], rightData);
        }

        // Handle remaining frames
        for (size_t i = simdFrames; i < framesRead; ++i) {
            left[i] = interleaved[i * 2];
            right[i] = interleaved[i * 2 + 1];
        }

        return framesRead;
    }

    /**
     * @brief Apply fade in/out to prevent clicks with SIMD optimization
     */
    void applyFade(float* buffer, size_t frames, bool fadeIn) {
        const size_t fadeLength = std::min(frames, size_t(64));

        if (fadeLength == 0) return;

        // Generate fade curve using SIMD
        alignas(32) float fadeGains[64];
        const float step = 1.0f / fadeLength;

        const size_t simdLength = fadeLength & ~7;

        for (size_t i = 0; i < simdLength; i += 8) {
            __m256 indices = _mm256_setr_ps(i, i+1, i+2, i+3, i+4, i+5, i+6, i+7);
            __m256 gains = _mm256_mul_ps(indices, _mm256_set1_ps(step));

            if (!fadeIn) {
                gains = _mm256_sub_ps(_mm256_set1_ps(1.0f), gains);
            }

            _mm256_store_ps(&fadeGains[i], gains);
        }

        // Handle remaining samples
        for (size_t i = simdLength; i < fadeLength; ++i) {
            fadeGains[i] = fadeIn ? (float(i) * step) : (1.0f - float(i) * step);
        }

        // Apply fade with SIMD
        for (size_t i = 0; i < simdLength; i += 8) {
            __m256 bufferData = _mm256_loadu_ps(&buffer[i]);
            __m256 gains = _mm256_load_ps(&fadeGains[i]);
            __m256 result = _mm256_mul_ps(bufferData, gains);
            _mm256_storeu_ps(&buffer[i], result);

            // Apply fade out to end of buffer if needed
            if (!fadeIn && frames > fadeLength) {
                size_t endIdx = frames - 1 - i;
                if (endIdx >= 8) {
                    __m256 endData = _mm256_loadu_ps(&buffer[endIdx - 7]);
                    __m256 endResult = _mm256_mul_ps(endData, gains);
                    _mm256_storeu_ps(&buffer[endIdx - 7], endResult);
                }
            }
        }

        // Handle remaining samples
        for (size_t i = simdLength; i < fadeLength; ++i) {
            buffer[i] *= fadeGains[i];
            if (!fadeIn && frames > fadeLength) {
                buffer[frames - 1 - i] *= fadeGains[i];
            }
        }
    }
};

} // namespace vrb