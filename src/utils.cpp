// utils.cpp - Implementation of utility functions
#include "utils.h"
#include <cstring>
#include <cstdlib>
#include <immintrin.h>  // For SIMD optimizations

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

    // SIMD-optimized windowing for common cases
#ifdef __AVX2__
    const size_t simdSize = size & ~7;

    for (size_t i = 0; i < simdSize; i += 8) {
        __m256 indices = _mm256_setr_ps(i, i+1, i+2, i+3, i+4, i+5, i+6, i+7);
        __m256 samples = _mm256_loadu_ps(&buffer[i]);
        __m256 window;

        switch (windowType) {
        case WindowType::Hann: {
            __m256 angle = _mm256_mul_ps(indices, _mm256_set1_ps(twoPI_N));
            __m256 cosVal = _mm256_cos_ps(angle);
            window = _mm256_mul_ps(_mm256_set1_ps(0.5f),
                                  _mm256_sub_ps(_mm256_set1_ps(1.0f), cosVal));
            break;
        }
        case WindowType::Hamming: {
            __m256 angle = _mm256_mul_ps(indices, _mm256_set1_ps(twoPI_N));
            __m256 cosVal = _mm256_cos_ps(angle);
            window = _mm256_sub_ps(_mm256_set1_ps(0.54f),
                                  _mm256_mul_ps(_mm256_set1_ps(0.46f), cosVal));
            break;
        }
        case WindowType::Blackman: {
            __m256 angle1 = _mm256_mul_ps(indices, _mm256_set1_ps(twoPI_N));
            __m256 angle2 = _mm256_mul_ps(indices, _mm256_set1_ps(fourPI_N));
            __m256 cos1 = _mm256_cos_ps(angle1);
            __m256 cos2 = _mm256_cos_ps(angle2);
            window = _mm256_add_ps(_mm256_set1_ps(0.42f),
                _mm256_add_ps(_mm256_mul_ps(_mm256_set1_ps(-0.5f), cos1),
                             _mm256_mul_ps(_mm256_set1_ps(0.08f), cos2)));
            break;
        }
        case WindowType::Tukey: {
            // Tukey window with 10% fade regions
            const float alpha = 0.1f;
            const float fadeLength = alpha * N * 0.5f;

            __m256 window = _mm256_set1_ps(1.0f);
            for (int j = 0; j < 8; ++j) {
                float n = i + j;
                if (n < fadeLength) {
                    float fade = 0.5f * (1.0f + std::cos(MathUtils::PI * (n / fadeLength - 1.0f)));
                    window[j] = fade;
                } else if (n > N - fadeLength) {
                    float fade = 0.5f * (1.0f + std::cos(MathUtils::PI * ((n - N + fadeLength) / fadeLength)));
                    window[j] = fade;
                }
            }
            break;
        }
        default:
            window = _mm256_set1_ps(1.0f);
            break;
        }

        __m256 result = _mm256_mul_ps(samples, window);
        _mm256_storeu_ps(&buffer[i], result);
    }

    // Process remaining samples
    for (size_t i = simdSize; i < size; ++i) {
#else
    for (size_t i = 0; i < size; ++i) {
#endif
        float window = 1.0f;
        const float n = static_cast<float>(i);

        switch (windowType) {
        case WindowType::Hann:
            window = 0.5f * (1.0f - std::cos(twoPI_N * n));
            break;
        case WindowType::Hamming:
            window = 0.54f - 0.46f * std::cos(twoPI_N * n);
            break;
        case WindowType::Blackman:
            window = 0.42f - 0.5f * std::cos(twoPI_N * n) + 0.08f * std::cos(fourPI_N * n);
            break;
        case WindowType::Tukey: {
            const float alpha = 0.1f;
            const float fadeLength = alpha * N * 0.5f;
            if (n < fadeLength) {
                window = 0.5f * (1.0f + std::cos(MathUtils::PI * (n / fadeLength - 1.0f)));
            } else if (n > N - fadeLength) {
                window = 0.5f * (1.0f + std::cos(MathUtils::PI * ((n - N + fadeLength) / fadeLength)));
            }
            break;
        }
        default:
            break;
        }

        buffer[i] *= window;
    }
}

void applyGainWithFade(float* buffer, size_t size, float startGain, float endGain) {
    if (!buffer || size == 0) return;

    const float gainDelta = (endGain - startGain) / static_cast<float>(size - 1);

#ifdef __AVX2__
    const size_t simdSize = size & ~7;
    const __m256 startGainVec = _mm256_set1_ps(startGain);
    const __m256 gainDeltaVec = _mm256_set1_ps(gainDelta);

    for (size_t i = 0; i < simdSize; i += 8) {
        __m256 indices = _mm256_setr_ps(i, i+1, i+2, i+3, i+4, i+5, i+6, i+7);
        __m256 gains = _mm256_fmadd_ps(indices, gainDeltaVec, startGainVec);
        __m256 samples = _mm256_loadu_ps(&buffer[i]);
        __m256 result = _mm256_mul_ps(samples, gains);
        _mm256_storeu_ps(&buffer[i], result);
    }

    // Process remaining samples
    for (size_t i = simdSize; i < size; ++i) {
#else
    for (size_t i = 0; i < size; ++i) {
#endif
        const float gain = startGain + gainDelta * static_cast<float>(i);
        buffer[i] *= gain;
    }
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