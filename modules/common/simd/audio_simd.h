// audio_simd.h - Consolidated SIMD optimizations for audio processing
// Consolidates SIMD operations from utils.h and other audio components

#pragma once

#include <cstddef>
#include <cstring>
#include <cmath>

#ifdef __AVX2__
#include <immintrin.h>
#elif defined(__SSE2__)
#include <emmintrin.h>
#endif

namespace vrb {
namespace simd {

/**
 * @brief SIMD-optimized RMS calculation
 */
inline float calculateRMS(const float* buffer, size_t size) {
    if (size == 0) return 0.0f;

#ifdef __AVX2__
    __m256 sum = _mm256_setzero_ps();
    const size_t simdSize = size & ~7;

    for (size_t i = 0; i < simdSize; i += 8) {
        __m256 samples = _mm256_loadu_ps(&buffer[i]);
        sum = _mm256_fmadd_ps(samples, samples, sum);
    }

    // Horizontal sum
    __m128 sum128 = _mm_add_ps(_mm256_extractf128_ps(sum, 0), _mm256_extractf128_ps(sum, 1));
    sum128 = _mm_hadd_ps(sum128, sum128);
    sum128 = _mm_hadd_ps(sum128, sum128);

    float result = _mm_cvtss_f32(sum128);

    // Handle remaining elements
    for (size_t i = simdSize; i < size; ++i) {
        result += buffer[i] * buffer[i];
    }

    return std::sqrt(result / size);

#elif defined(__SSE2__)
    __m128 sum = _mm_setzero_ps();
    const size_t simdSize = size & ~3;

    for (size_t i = 0; i < simdSize; i += 4) {
        __m128 samples = _mm_loadu_ps(&buffer[i]);
        sum = _mm_add_ps(sum, _mm_mul_ps(samples, samples));
    }

    // Horizontal sum
    sum = _mm_hadd_ps(sum, sum);
    sum = _mm_hadd_ps(sum, sum);

    float result = _mm_cvtss_f32(sum);

    // Handle remaining elements
    for (size_t i = simdSize; i < size; ++i) {
        result += buffer[i] * buffer[i];
    }

    return std::sqrt(result / size);

#else
    float sum = 0.0f;
    for (size_t i = 0; i < size; ++i) {
        sum += buffer[i] * buffer[i];
    }
    return std::sqrt(sum / size);
#endif
}

/**
 * @brief SIMD-optimized peak calculation
 */
inline float calculatePeak(const float* buffer, size_t size) {
    if (size == 0) return 0.0f;

#ifdef __AVX2__
    __m256 maxValues = _mm256_setzero_ps();
    const size_t simdSize = size & ~7;

    for (size_t i = 0; i < simdSize; i += 8) {
        __m256 samples = _mm256_loadu_ps(&buffer[i]);
        __m256 absSamples = _mm256_andnot_ps(_mm256_set1_ps(-0.0f), samples);
        maxValues = _mm256_max_ps(maxValues, absSamples);
    }

    // Horizontal max
    __m128 max128 = _mm_max_ps(_mm256_extractf128_ps(maxValues, 0), _mm256_extractf128_ps(maxValues, 1));
    max128 = _mm_max_ps(max128, _mm_shuffle_ps(max128, max128, _MM_SHUFFLE(2, 3, 0, 1)));
    max128 = _mm_max_ps(max128, _mm_shuffle_ps(max128, max128, _MM_SHUFFLE(1, 0, 3, 2)));

    float result = _mm_cvtss_f32(max128);

    // Handle remaining elements
    for (size_t i = simdSize; i < size; ++i) {
        result = std::max(result, std::abs(buffer[i]));
    }

    return result;

#elif defined(__SSE2__)
    __m128 maxValues = _mm_setzero_ps();
    const size_t simdSize = size & ~3;

    for (size_t i = 0; i < simdSize; i += 4) {
        __m128 samples = _mm_loadu_ps(&buffer[i]);
        __m128 absSamples = _mm_andnot_ps(_mm_set1_ps(-0.0f), samples);
        maxValues = _mm_max_ps(maxValues, absSamples);
    }

    // Horizontal max
    maxValues = _mm_max_ps(maxValues, _mm_shuffle_ps(maxValues, maxValues, _MM_SHUFFLE(2, 3, 0, 1)));
    maxValues = _mm_max_ps(maxValues, _mm_shuffle_ps(maxValues, maxValues, _MM_SHUFFLE(1, 0, 3, 2)));

    float result = _mm_cvtss_f32(maxValues);

    // Handle remaining elements
    for (size_t i = simdSize; i < size; ++i) {
        result = std::max(result, std::abs(buffer[i]));
    }

    return result;

#else
    float maxVal = 0.0f;
    for (size_t i = 0; i < size; ++i) {
        maxVal = std::max(maxVal, std::abs(buffer[i]));
    }
    return maxVal;
#endif
}

/**
 * @brief SIMD-optimized buffer mixing
 */
inline void mixBuffers(float* destination, const float* source, size_t size, float gain = 1.0f) {
#ifdef __AVX2__
    const __m256 gainVector = _mm256_set1_ps(gain);
    const size_t simdSize = size & ~7;

    for (size_t i = 0; i < simdSize; i += 8) {
        __m256 dest = _mm256_loadu_ps(&destination[i]);
        __m256 src = _mm256_loadu_ps(&source[i]);
        __m256 scaled = _mm256_mul_ps(src, gainVector);
        __m256 result = _mm256_add_ps(dest, scaled);
        _mm256_storeu_ps(&destination[i], result);
    }

    // Handle remaining elements
    for (size_t i = simdSize; i < size; ++i) {
        destination[i] += source[i] * gain;
    }

#elif defined(__SSE2__)
    const __m128 gainVector = _mm_set1_ps(gain);
    const size_t simdSize = size & ~3;

    for (size_t i = 0; i < simdSize; i += 4) {
        __m128 dest = _mm_loadu_ps(&destination[i]);
        __m128 src = _mm_loadu_ps(&source[i]);
        __m128 scaled = _mm_mul_ps(src, gainVector);
        __m128 result = _mm_add_ps(dest, scaled);
        _mm_storeu_ps(&destination[i], result);
    }

    // Handle remaining elements
    for (size_t i = simdSize; i < size; ++i) {
        destination[i] += source[i] * gain;
    }

#else
    for (size_t i = 0; i < size; ++i) {
        destination[i] += source[i] * gain;
    }
#endif
}

/**
 * @brief SIMD-optimized buffer copying
 */
inline void copyBuffer(float* destination, const float* source, size_t size) {
#ifdef __AVX2__
    const size_t simdSize = size & ~7;

    for (size_t i = 0; i < simdSize; i += 8) {
        __m256 data = _mm256_loadu_ps(&source[i]);
        _mm256_storeu_ps(&destination[i], data);
    }

    // Handle remaining elements
    std::memcpy(&destination[simdSize], &source[simdSize], (size - simdSize) * sizeof(float));

#elif defined(__SSE2__)
    const size_t simdSize = size & ~3;

    for (size_t i = 0; i < simdSize; i += 4) {
        __m128 data = _mm_loadu_ps(&source[i]);
        _mm_storeu_ps(&destination[i], data);
    }

    // Handle remaining elements
    std::memcpy(&destination[simdSize], &source[simdSize], (size - simdSize) * sizeof(float));

#else
    std::memcpy(destination, source, size * sizeof(float));
#endif
}

/**
 * @brief SIMD-optimized gain application with fade
 */
inline void applyGainWithFade(float* buffer, size_t size, float startGain, float endGain) {
    if (size == 0) return;

    const float gainStep = (endGain - startGain) / static_cast<float>(size - 1);

#ifdef __AVX2__
    const size_t simdSize = size & ~7;
    const __m256 stepVector = _mm256_set_ps(
        7 * gainStep, 6 * gainStep, 5 * gainStep, 4 * gainStep,
        3 * gainStep, 2 * gainStep, 1 * gainStep, 0 * gainStep
    );
    const __m256 incrementVector = _mm256_set1_ps(8 * gainStep);

    __m256 currentGain = _mm256_set1_ps(startGain);
    currentGain = _mm256_add_ps(currentGain, stepVector);

    for (size_t i = 0; i < simdSize; i += 8) {
        __m256 samples = _mm256_loadu_ps(&buffer[i]);
        __m256 result = _mm256_mul_ps(samples, currentGain);
        _mm256_storeu_ps(&buffer[i], result);
        currentGain = _mm256_add_ps(currentGain, incrementVector);
    }

    // Handle remaining elements
    for (size_t i = simdSize; i < size; ++i) {
        float gain = startGain + gainStep * static_cast<float>(i);
        buffer[i] *= gain;
    }

#else
    for (size_t i = 0; i < size; ++i) {
        float gain = startGain + gainStep * static_cast<float>(i);
        buffer[i] *= gain;
    }
#endif
}

} // namespace simd
} // namespace vrb