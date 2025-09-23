// utils.cpp - Implementation of utility functions
#include "utils.h"
#include <cstring>
#include <cstdlib>
#include <cmath>

#ifndef _MSC_VER
#include <unordered_map>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <malloc.h>
#else
#include <stdlib.h>
#endif

namespace vrb {

namespace AudioUtils {

void applyWindow(float* buffer, size_t size, WindowType windowType) {
    if (!buffer || size == 0) return;

    if (windowType == WindowType::Rectangular) {
        return;  // No modification needed
    }

    const float N = static_cast<float>(size - 1);
    const float twoPI_N = 2.0f * MathUtils::PI / N;
    const float fourPI_N = 4.0f * MathUtils::PI / N;

    // Scalar windowing implementation
    for (size_t i = 0; i < size; ++i) {
        float window = 1.0f;
        const float pos = static_cast<float>(i);

        switch (windowType) {
        case WindowType::Hann: {
            float angle = pos * twoPI_N;
            window = 0.5f * (1.0f - cosf(angle));
            break;
        }
        case WindowType::Hamming: {
            float angle = pos * twoPI_N;
            window = 0.54f - 0.46f * cosf(angle);
            break;
        }
        case WindowType::Blackman: {
            float angle1 = pos * twoPI_N;
            float angle2 = pos * fourPI_N;
            window = 0.42f - 0.5f * cosf(angle1) + 0.08f * cosf(angle2);
            break;
        }
        case WindowType::Tukey: {
            // Tukey window with 10% fade regions
            const float alpha = 0.1f;
            const float fadeLength = alpha * N * 0.5f;

            if (pos <= fadeLength) {
                window = 0.5f * (1.0f + cosf(MathUtils::PI * (pos / fadeLength - 1.0f)));
            } else if (pos >= N - fadeLength) {
                window = 0.5f * (1.0f + cosf(MathUtils::PI * ((pos - N + fadeLength) / fadeLength)));
            } else {
                window = 1.0f;
            }
            break;
        }
        default:
            window = 1.0f;
            break;
        }

        buffer[i] *= window;
    }
}

void applyGainWithFade(float* buffer, size_t size, float startGain, float endGain) {
    simd::applyGainWithFade(buffer, size, startGain, endGain);
}

void clearBuffer(float* buffer, size_t size, bool fadeOut) {
    if (!buffer || size == 0) return;

    if (fadeOut && size > 64) {
        // Apply fade out to last 64 samples to prevent clicks
        const size_t fadeLength = 64;
        const size_t fadeStart = size - fadeLength;

        applyGainWithFade(buffer + fadeStart, fadeLength, 1.0f, 0.0f);
    }

    // SIMD-optimized clearing
#ifdef __AVX2__
    const __m256 zero = _mm256_setzero_ps();
    const size_t simdSize = size & ~7;

    for (size_t i = 0; i < simdSize; i += 8) {
        _mm256_storeu_ps(&buffer[i], zero);
    }

    // Clear remaining samples
    for (size_t i = simdSize; i < size; ++i) {
        buffer[i] = 0.0f;
    }
#else
    std::memset(buffer, 0, size * sizeof(float));
#endif
}

} // namespace AudioUtils

// MemoryPool implementation
MemoryPool::MemoryPool(size_t poolSize)
    : m_poolSize(poolSize)
    , m_used(0) {

    // Allocate aligned memory pool using our utility function
    m_pool = PerfUtils::alignedAlloc(poolSize, PerfUtils::CACHE_LINE_SIZE);

    if (!m_pool) {
        throw std::runtime_error("Failed to allocate memory pool of size " +
                                std::to_string(poolSize) + " bytes");
    }

    // Clear the pool efficiently
#ifdef __AVX2__
    const __m256i zero = _mm256_setzero_si256();
    const size_t simdSize = poolSize & ~31;  // 32-byte alignment

    for (size_t i = 0; i < simdSize; i += 32) {
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(static_cast<char*>(m_pool) + i), zero);
    }

    // Clear remaining bytes
    std::memset(static_cast<char*>(m_pool) + simdSize, 0, poolSize - simdSize);
#else
    std::memset(m_pool, 0, poolSize);
#endif
}

MemoryPool::~MemoryPool() {
    if (m_pool) {
        PerfUtils::alignedFree(m_pool);
        m_pool = nullptr;
    }
}

void* MemoryPool::allocate(size_t size, size_t alignment) {
    if (size == 0) return nullptr;

    // Align size to requested alignment
    size_t alignedSize = (size + alignment - 1) & ~(alignment - 1);

    std::lock_guard<std::mutex> lock(m_mutex);

    size_t currentUsed = m_used.load(std::memory_order_relaxed);
    if (currentUsed + alignedSize > m_poolSize) {
        return nullptr;  // Pool exhausted
    }

    // Find aligned position
    uintptr_t poolAddr = reinterpret_cast<uintptr_t>(m_pool);
    uintptr_t allocAddr = poolAddr + currentUsed;
    uintptr_t alignedAddr = (allocAddr + alignment - 1) & ~(alignment - 1);

    size_t actualUsed = (alignedAddr - poolAddr) + size;
    if (actualUsed > m_poolSize) {
        return nullptr;
    }

    m_used.store(actualUsed, std::memory_order_relaxed);
    return reinterpret_cast<void*>(alignedAddr);
}

void MemoryPool::deallocate(void* /*ptr*/) {
    // Simple pool doesn't support individual deallocation
    // Memory is freed when pool is reset or destroyed
}

void MemoryPool::reset() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_used.store(0, std::memory_order_relaxed);

    // Optionally clear memory for security
#ifdef CLEAR_MEMORY_ON_RESET
    std::memset(m_pool, 0, m_poolSize);
#endif
}

} // namespace vrb