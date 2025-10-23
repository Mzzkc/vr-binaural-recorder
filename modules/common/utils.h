// utils.h - Utility functions and helper classes for VR Binaural Recorder
#pragma once

#include <chrono>
#include <thread>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include "../../core/include/vr_types.h"
#include "simd/audio_simd.h"

namespace vrb {

/**
 * @brief High-precision timer for performance measurements
 */
class Timer {
public:
    Timer() : m_start(std::chrono::high_resolution_clock::now()) {}

    void reset() {
        m_start = std::chrono::high_resolution_clock::now();
    }

    double elapsedSeconds() const {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_start);
        return duration.count() / 1e9;
    }

    double elapsedMilliseconds() const {
        return elapsedSeconds() * 1000.0;
    }

    double elapsedMicroseconds() const {
        return elapsedSeconds() * 1e6;
    }

private:
    std::chrono::high_resolution_clock::time_point m_start;
};

/**
 * @brief Moving average filter for smoothing values
 */
template<typename T>
class MovingAverage {
public:
    explicit MovingAverage(size_t windowSize)
        : m_windowSize(windowSize)
        , m_values(windowSize, T{})
        , m_index(0)
        , m_filled(false) {}

    T add(const T& value) {
        m_values[m_index] = value;
        m_index = (m_index + 1) % m_windowSize;

        if (!m_filled && m_index == 0) {
            m_filled = true;
        }

        return getAverage();
    }

    T getAverage() const {
        T sum = T{};
        size_t count = m_filled ? m_windowSize : m_index;

        for (size_t i = 0; i < count; ++i) {
            sum += m_values[i];
        }

        return count > 0 ? sum / static_cast<T>(count) : T{};
    }

    void reset() {
        std::fill(m_values.begin(), m_values.end(), T{});
        m_index = 0;
        m_filled = false;
    }

private:
    size_t m_windowSize;
    std::vector<T> m_values;
    size_t m_index;
    bool m_filled;
};

/**
 * @brief Thread-safe performance profiler
 */
class Profiler {
public:
    struct ProfileData {
        std::string name;
        double totalTime;
        double averageTime;
        double minTime;
        double maxTime;
        size_t sampleCount;
    };

    static Profiler& getInstance() {
        static Profiler instance;
        return instance;
    }

    void beginProfile(const std::string& name) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_activeProfiles[name] = std::chrono::high_resolution_clock::now();
    }

    void endProfile(const std::string& name) {
        auto endTime = std::chrono::high_resolution_clock::now();

        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_activeProfiles.find(name);
        if (it != m_activeProfiles.end()) {
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
                endTime - it->second).count() / 1e6; // Convert to milliseconds

            updateProfileData(name, duration);
            m_activeProfiles.erase(it);
        }
    }

    std::vector<ProfileData> getProfileData() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<ProfileData> result;

        for (const auto& pair : m_profileData) {
            result.push_back(pair.second);
        }

        return result;
    }

    void reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_profileData.clear();
        m_activeProfiles.clear();
    }

private:
    void updateProfileData(const std::string& name, double timeMs) {
        auto& data = m_profileData[name];
        data.name = name;
        data.totalTime += timeMs;
        data.sampleCount++;
        data.averageTime = data.totalTime / data.sampleCount;

        if (data.sampleCount == 1) {
            data.minTime = data.maxTime = timeMs;
        } else {
            data.minTime = std::min(data.minTime, timeMs);
            data.maxTime = std::max(data.maxTime, timeMs);
        }
    }

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, ProfileData> m_profileData;
    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> m_activeProfiles;
};

/**
 * @brief RAII profiler helper
 */
class ScopedProfiler {
public:
    explicit ScopedProfiler(const std::string& name) : m_name(name) {
        Profiler::getInstance().beginProfile(m_name);
    }

    ~ScopedProfiler() {
        Profiler::getInstance().endProfile(m_name);
    }

private:
    std::string m_name;
};

// Convenience macro for profiling
#define PROFILE_SCOPE(name) vrb::ScopedProfiler _prof(name)

/**
 * @brief Math utilities for spatial calculations
 */
namespace MathUtils {

    constexpr float PI = 3.14159265358979323846f;
    constexpr float TAU = 2.0f * PI;
    constexpr float DEG_TO_RAD = PI / 180.0f;
    constexpr float RAD_TO_DEG = 180.0f / PI;

    /**
     * @brief Linear interpolation
     */
    template<typename T>
    inline T lerp(const T& a, const T& b, float t) {
        return a + (b - a) * t;
    }

    /**
     * @brief Clamp value between min and max
     */
    template<typename T>
    inline T clamp(const T& value, const T& min, const T& max) {
        return std::max(min, std::min(max, value));
    }

    /**
     * @brief Convert dB to linear gain
     */
    inline float dbToLinear(float db) {
        return std::pow(10.0f, db / 20.0f);
    }

    /**
     * @brief Convert linear gain to dB
     */
    inline float linearToDb(float linear) {
        return 20.0f * std::log10(std::max(linear, 1e-8f));
    }

    /**
     * @brief Calculate distance between two 3D points
     */
    inline float distance(const Vec3& a, const Vec3& b) {
        return (a - b).length();
    }

    /**
     * @brief Calculate distance attenuation (inverse square law)
     */
    inline float distanceAttenuation(float distance, float referenceDistance = 1.0f) {
        return referenceDistance / std::max(distance, 0.01f);
    }

    /**
     * @brief Calculate angle between two vectors in radians
     */
    inline float angleBetween(const Vec3& a, const Vec3& b) {
        Vec3 na = a.normalized();
        Vec3 nb = b.normalized();
        float dotProduct = clamp(na.dot(nb), -1.0f, 1.0f);
        return std::acos(dotProduct);
    }

    /**
     * @brief Smooth exponential interpolation (good for real-time smoothing)
     */
    template<typename T>
    inline T exponentialSmooth(const T& current, const T& target, float smoothing) {
        return current + (target - current) * (1.0f - smoothing);
    }

    /**
     * @brief Calculate spherical coordinates from Cartesian
     */
    struct SphericalCoords {
        float radius;
        float azimuth;    // Angle around Y axis
        float elevation;  // Angle up from XZ plane
    };

    inline SphericalCoords cartesianToSpherical(const Vec3& cartesian) {
        SphericalCoords result;
        result.radius = cartesian.length();

        if (result.radius > 0.001f) {
            result.azimuth = std::atan2(cartesian.x, cartesian.z);
            result.elevation = std::asin(cartesian.y / result.radius);
        } else {
            result.azimuth = 0.0f;
            result.elevation = 0.0f;
        }

        return result;
    }

    /**
     * @brief Convert spherical coordinates to Cartesian
     */
    inline Vec3 sphericalToCartesian(const SphericalCoords& spherical) {
        float cosElevation = std::cos(spherical.elevation);
        return Vec3(
            spherical.radius * std::sin(spherical.azimuth) * cosElevation,
            spherical.radius * std::sin(spherical.elevation),
            spherical.radius * std::cos(spherical.azimuth) * cosElevation
        );
    }
}

/**
 * @brief String utilities
 */
namespace StringUtils {

    /**
     * @brief Format a float with specified precision
     */
    inline std::string formatFloat(float value, int precision = 2) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << value;
        return oss.str();
    }

    /**
     * @brief Format time in milliseconds to readable string
     */
    inline std::string formatTimeMs(double timeMs) {
        if (timeMs < 1.0) {
            return formatFloat(static_cast<float>(timeMs * 1000.0), 1) + " μs";
        } else if (timeMs < 1000.0) {
            return formatFloat(static_cast<float>(timeMs), 2) + " ms";
        } else {
            return formatFloat(static_cast<float>(timeMs / 1000.0), 2) + " s";
        }
    }

    /**
     * @brief Format memory size in bytes to readable string
     */
    inline std::string formatMemorySize(size_t bytes) {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unitIndex = 0;
        double size = static_cast<double>(bytes);

        while (size >= 1024.0 && unitIndex < 4) {
            size /= 1024.0;
            unitIndex++;
        }

        return formatFloat(static_cast<float>(size), 2) + " " + units[unitIndex];
    }

    /**
     * @brief Split string by delimiter
     */
    inline std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> result;
        std::stringstream ss(str);
        std::string item;

        while (std::getline(ss, item, delimiter)) {
            result.push_back(item);
        }

        return result;
    }

    /**
     * @brief Trim whitespace from string
     */
    inline std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\n\r");
        if (start == std::string::npos) return "";

        size_t end = str.find_last_not_of(" \t\n\r");
        return str.substr(start, end - start + 1);
    }
}

/**
 * @brief Audio utilities with SIMD optimization
 */
namespace AudioUtils {

    /**
     * @brief Apply windowing function to audio buffer
     */
    enum class WindowType {
        Rectangular,
        Hann,
        Hamming,
        Blackman,
        Tukey
    };

    void applyWindow(float* buffer, size_t size, WindowType windowType);

    /**
     * @brief SIMD-optimized RMS calculation (delegated to consolidated SIMD module)
     */
    inline float calculateRMS(const float* buffer, size_t size) {
        return simd::calculateRMS(buffer, size);
    }

    /**
     * @brief SIMD-optimized peak calculation (delegated to consolidated SIMD module)
     */
    inline float calculatePeak(const float* buffer, size_t size) {
        return simd::calculatePeak(buffer, size);
    }

    /**
     * @brief Apply gain with fade to prevent clicks
     */
    void applyGainWithFade(float* buffer, size_t size, float startGain, float endGain);

    /**
     * @brief SIMD-optimized buffer mixing (delegated to consolidated SIMD module)
     */
    inline void mixBuffers(float* destination, const float* source, size_t size, float gain = 1.0f) {
        simd::mixBuffers(destination, source, size, gain);
    }

    /**
     * @brief Clear audio buffer with optional fade out
     */
    void clearBuffer(float* buffer, size_t size, bool fadeOut = false);

    /**
     * @brief SIMD-optimized buffer copying (delegated to consolidated SIMD module)
     */
    inline void copyBuffer(float* destination, const float* source, size_t size) {
        simd::copyBuffer(destination, source, size);
    }

    /**
     * @brief Apply DC removal filter
     */
    inline void removeDC(float* buffer, size_t size, float& dcState, float alpha = 0.995f) {
        for (size_t i = 0; i < size; ++i) {
            dcState = alpha * dcState + buffer[i];
            buffer[i] -= dcState;
        }
    }
}

/**
 * @brief Common performance utilities and patterns
 */
namespace PerfUtils {
    /**
     * @brief RAII lock guard wrapper for better debugging
     */
    template<typename Mutex>
    class DebugLockGuard {
    public:
        explicit DebugLockGuard(Mutex& mutex, const char* location = "")
            : m_guard(mutex), m_location(location) {
#ifdef DEBUG_LOCKS
            LOG_DEBUG("Acquired lock at: {}", m_location);
#endif
        }

        ~DebugLockGuard() {
#ifdef DEBUG_LOCKS
            LOG_DEBUG("Released lock at: {}", m_location);
#endif
        }

    private:
        std::lock_guard<Mutex> m_guard;
        const char* m_location;
    };

    /**
     * @brief Fast atomic load/store helpers
     */
    template<typename T>
    inline T atomicLoad(const std::atomic<T>& atomic, std::memory_order order = std::memory_order_relaxed) {
        return atomic.load(order);
    }

    template<typename T>
    inline void atomicStore(std::atomic<T>& atomic, T value, std::memory_order order = std::memory_order_relaxed) {
        atomic.store(value, order);
    }

    /**
     * @brief Cache line size detection
     */
    static constexpr size_t CACHE_LINE_SIZE = 64;  // Most common cache line size

    /**
     * @brief Aligned memory allocation
     */
    inline void* alignedAlloc(size_t size, size_t alignment = CACHE_LINE_SIZE) {
#ifdef _WIN32
        return _aligned_malloc(size, alignment);
#else
        void* ptr = nullptr;
        return (posix_memalign(&ptr, alignment, size) == 0) ? ptr : nullptr;
#endif
    }

    inline void alignedFree(void* ptr) {
#ifdef _WIN32
        _aligned_free(ptr);
#else
        free(ptr);
#endif
    }
}

/**
 * @brief Memory pool for real-time allocations
 */
class MemoryPool {
public:
    explicit MemoryPool(size_t poolSize);
    ~MemoryPool();

    void* allocate(size_t size, size_t alignment = 16);
    void deallocate(void* ptr);
    void reset();

    size_t getUsedMemory() const { return m_used.load(std::memory_order_relaxed); }
    size_t getTotalMemory() const { return m_poolSize; }
    bool isFull() const { return m_used.load(std::memory_order_relaxed) >= m_poolSize; }
    float getUsagePercentage() const {
        return static_cast<float>(getUsedMemory()) / getTotalMemory() * 100.0f;
    }

private:
    void* m_pool;
    size_t m_poolSize;
    std::atomic<size_t> m_used;
    mutable std::mutex m_mutex;
};

// Common lock guard macro
#define SCOPED_LOCK(mutex) vrb::PerfUtils::DebugLockGuard<std::remove_reference_t<decltype(mutex)>> \
    _lock_guard(mutex, __FILE__ ":" STRINGIFY(__LINE__))
#define STRINGIFY(x) #x

} // namespace vrb