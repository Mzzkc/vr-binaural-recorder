// utils.cpp - Implementation of utility functions
#include "utils.h"
#include <cstring>
#include <cstdlib>

#ifndef _MSC_VER
#include <unordered_map>
#endif

namespace vrb {

namespace AudioUtils {

void applyWindow(float* buffer, size_t size, WindowType windowType) {
    if (!buffer || size == 0) return;

    const float N = static_cast<float>(size - 1);

    for (size_t i = 0; i < size; ++i) {
        float window = 1.0f;
        const float n = static_cast<float>(i);

        switch (windowType) {
        case WindowType::Rectangular:
            window = 1.0f;
            break;

        case WindowType::Hann:
            window = 0.5f * (1.0f - std::cos(2.0f * MathUtils::PI * n / N));
            break;

        case WindowType::Hamming:
            window = 0.54f - 0.46f * std::cos(2.0f * MathUtils::PI * n / N);
            break;

        case WindowType::Blackman:
            window = 0.42f - 0.5f * std::cos(2.0f * MathUtils::PI * n / N) +
                    0.08f * std::cos(4.0f * MathUtils::PI * n / N);
            break;
        }

        buffer[i] *= window;
    }
}

void applyGainWithFade(float* buffer, size_t size, float startGain, float endGain) {
    if (!buffer || size == 0) return;

    const float gainDelta = (endGain - startGain) / static_cast<float>(size - 1);

    for (size_t i = 0; i < size; ++i) {
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

        for (size_t i = fadeStart; i < size; ++i) {
            const float fade = static_cast<float>(size - i) / static_cast<float>(fadeLength);
            buffer[i] *= fade;
        }
    }

    std::memset(buffer, 0, size * sizeof(float));
}

} // namespace AudioUtils

// MemoryPool implementation
MemoryPool::MemoryPool(size_t poolSize)
    : m_poolSize(poolSize)
    , m_used(0) {

    // Allocate aligned memory pool
#ifdef _WIN32
    m_pool = _aligned_malloc(poolSize, 64);
#else
    if (posix_memalign(&m_pool, 64, poolSize) != 0) {
        m_pool = nullptr;
    }
#endif

    if (!m_pool) {
        throw std::runtime_error("Failed to allocate memory pool");
    }

    // Clear the pool
    std::memset(m_pool, 0, poolSize);
}

MemoryPool::~MemoryPool() {
    if (m_pool) {
#ifdef _WIN32
        _aligned_free(m_pool);
#else
        free(m_pool);
#endif
    }
}

void* MemoryPool::allocate(size_t size, size_t alignment) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // Align the requested size
    size_t alignedSize = (size + alignment - 1) & ~(alignment - 1);

    // Check if we have enough space
    if (m_used + alignedSize > m_poolSize) {
        return nullptr; // Pool exhausted
    }

    // Calculate aligned pointer
    uintptr_t poolAddr = reinterpret_cast<uintptr_t>(m_pool);
    uintptr_t currentAddr = poolAddr + m_used;
    uintptr_t alignedAddr = (currentAddr + alignment - 1) & ~(alignment - 1);

    // Update used memory (including alignment padding)
    size_t padding = alignedAddr - currentAddr;
    m_used += padding + alignedSize;

    return reinterpret_cast<void*>(alignedAddr);
}

void MemoryPool::deallocate(void* ptr) {
    // Simple pool - deallocate does nothing, use reset() to clear all
    (void)ptr; // Suppress unused parameter warning
}

void MemoryPool::reset() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_used = 0;
    std::memset(m_pool, 0, m_poolSize);
}

} // namespace vrb