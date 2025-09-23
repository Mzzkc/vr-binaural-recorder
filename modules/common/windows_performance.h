// windows_performance.h - Windows-specific performance optimizations
// Comprehensive Windows performance optimization utilities

#pragma once

#ifdef _WIN32

#include <windows.h>
#include <winternl.h>
#include <psapi.h>
#include <avrt.h>
#include <powerbase.h>
#include <powersetting.h>
#include <string>
#include <atomic>
#include <memory>
#include <chrono>

#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "avrt.lib")
#pragma comment(lib, "powrprof.lib")
#pragma comment(lib, "psapi.lib")

namespace vrb::performance {

/**
 * @brief Windows-specific performance optimization manager
 *
 * This class provides comprehensive Windows performance optimizations:
 * - High-precision timer management
 * - Thread priority and affinity optimization
 * - Memory management and page locking
 * - CPU power state management
 * - DPI and display optimization
 * - Audio thread scheduling with MMCSS
 * - Windows-specific SIMD optimizations
 */
class WindowsPerformanceManager {
public:
    struct PerformanceConfig {
        bool enableHighPrecisionTimer = true;
        bool enableMmcssScheduling = true;
        bool enableMemoryOptimizations = true;
        bool enableCpuOptimizations = true;
        bool enablePowerOptimizations = true;
        bool enableDpiOptimizations = true;
        bool enableAffinityOptimizations = false;  // Can be intrusive
        std::string mmcssTask = "Pro Audio";
        int audioThreadPriority = THREAD_PRIORITY_TIME_CRITICAL;
        size_t workingSetSize = 64 * 1024 * 1024;  // 64MB working set
    };

    WindowsPerformanceManager();
    ~WindowsPerformanceManager();

    /**
     * @brief Initialize performance optimizations
     * @param config Performance configuration
     * @return true if successful
     */
    bool Initialize(const PerformanceConfig& config);

    /**
     * @brief Apply optimizations to current process
     * @return true if successful
     */
    bool OptimizeProcess();

    /**
     * @brief Optimize audio thread for real-time performance
     * @param threadHandle Handle to audio thread (optional, uses current thread if null)
     * @return MMCSS task handle, or null if failed
     */
    HANDLE OptimizeAudioThread(HANDLE threadHandle = nullptr);

    /**
     * @brief Revert audio thread optimizations
     * @param mmcssHandle MMCSS task handle from OptimizeAudioThread
     */
    void RevertAudioThread(HANDLE mmcssHandle);

    /**
     * @brief Set thread CPU affinity for optimal performance
     * @param threadHandle Thread handle
     * @param preferredCores Preferred CPU cores (empty for auto-select)
     * @return true if successful
     */
    bool SetOptimalThreadAffinity(HANDLE threadHandle, const std::vector<int>& preferredCores = {});

    /**
     * @brief Enable high-precision timer resolution
     * @param targetResolutionMs Target resolution in milliseconds
     * @return true if successful
     */
    bool EnableHighPrecisionTimer(double targetResolutionMs = 1.0);

    /**
     * @brief Disable high-precision timer resolution
     */
    void DisableHighPrecisionTimer();

    /**
     * @brief Lock process memory pages to prevent paging
     * @param size Memory size to lock (0 for working set size)
     * @return true if successful
     */
    bool LockProcessMemory(size_t size = 0);

    /**
     * @brief Unlock process memory pages
     */
    void UnlockProcessMemory();

    /**
     * @brief Set CPU power scheme for high performance
     * @return true if successful
     */
    bool SetHighPerformancePowerScheme();

    /**
     * @brief Restore original power scheme
     */
    void RestoreOriginalPowerScheme();

    /**
     * @brief Configure DPI awareness for optimal rendering
     * @return true if successful
     */
    bool ConfigureDpiAwareness();

    /**
     * @brief Get system performance metrics
     */
    struct PerformanceMetrics {
        double cpuUsage;
        size_t memoryUsageMB;
        size_t workingSetMB;
        double timerResolutionMs;
        bool mmcssEnabled;
        bool highPerformancePower;
        int processPriority;
        DWORD_PTR processAffinity;
        double latency;
    };
    PerformanceMetrics GetPerformanceMetrics() const;

    /**
     * @brief Check if running on battery power
     */
    bool IsOnBatteryPower() const;

    /**
     * @brief Get CPU topology information
     */
    struct CpuTopology {
        int logicalProcessors;
        int physicalCores;
        int numaNodes;
        std::vector<int> performanceCores;
        std::vector<int> efficiencyCores;
    };
    CpuTopology GetCpuTopology() const;

    /**
     * @brief Optimize for specific hardware configuration
     * @param cpuBrand CPU brand (Intel, AMD, etc.)
     * @param ramSize RAM size in GB
     * @return true if optimizations applied
     */
    bool OptimizeForHardware(const std::string& cpuBrand, int ramSizeGB);

    /**
     * @brief Enable Windows-specific SIMD optimizations
     */
    bool EnableSIMDOptimizations();

    /**
     * @brief Monitor performance in real-time
     * @param callback Function called with performance data
     */
    void StartPerformanceMonitoring(std::function<void(const PerformanceMetrics&)> callback);
    void StopPerformanceMonitoring();

private:
    /**
     * @brief Detect optimal CPU cores for audio processing
     */
    std::vector<int> DetectOptimalAudioCores() const;

    /**
     * @brief Check Windows version and capabilities
     */
    bool CheckWindowsCapabilities();

    /**
     * @brief Enable Windows 10+ specific optimizations
     */
    bool EnableWindows10Optimizations();

    /**
     * @brief Configure process priority class
     */
    bool SetProcessPriorityClass();

    /**
     * @brief Configure memory management
     */
    bool ConfigureMemoryManagement();

    /**
     * @brief Monitor thread for performance metrics
     */
    void PerformanceMonitorThread();

    // Configuration
    PerformanceConfig m_config;
    std::atomic<bool> m_initialized{false};

    // Timer management
    bool m_timerResolutionSet{false};
    UINT m_originalTimerResolution{0};
    UINT m_currentTimerResolution{0};

    // Memory management
    bool m_memoryLocked{false};
    size_t m_lockedMemorySize{0};
    HANDLE m_processToken{nullptr};

    // Power management
    GUID m_originalPowerScheme{};
    bool m_powerSchemeChanged{false};

    // Performance monitoring
    std::atomic<bool> m_monitoringActive{false};
    std::thread m_monitorThread;
    std::function<void(const PerformanceMetrics&)> m_monitorCallback;

    // System information
    SYSTEM_INFO m_systemInfo{};
    OSVERSIONINFOEX m_osVersion{};
    bool m_isWindows10OrLater{false};
    bool m_isWindows11OrLater{false};

    // CPU information
    mutable std::unique_ptr<CpuTopology> m_cpuTopology;

    // Performance counters
    mutable FILETIME m_lastCpuTime{};
    mutable FILETIME m_lastSystemTime{};
    mutable std::chrono::steady_clock::time_point m_lastMetricsTime;
};

/**
 * @brief RAII helper for MMCSS audio thread optimization
 */
class MmcssAudioThread {
public:
    explicit MmcssAudioThread(const std::string& taskName = "Pro Audio");
    ~MmcssAudioThread();

    bool IsActive() const { return m_mmcssHandle != nullptr; }
    HANDLE GetHandle() const { return m_mmcssHandle; }

private:
    HANDLE m_mmcssHandle{nullptr};
    DWORD m_taskIndex{0};
};

/**
 * @brief RAII helper for high-precision timer
 */
class HighPrecisionTimer {
public:
    explicit HighPrecisionTimer(double targetResolutionMs = 1.0);
    ~HighPrecisionTimer();

    bool IsActive() const { return m_active; }
    double GetResolution() const { return m_resolutionMs; }

private:
    bool m_active{false};
    double m_resolutionMs{0.0};
    UINT m_originalResolution{0};
};

/**
 * @brief High-performance memory allocator for audio buffers
 */
class PerformanceAllocator {
public:
    /**
     * @brief Allocate aligned memory for SIMD operations
     * @param size Size in bytes
     * @param alignment Alignment in bytes (default: 32 for AVX)
     * @return Aligned memory pointer
     */
    static void* AllocateAligned(size_t size, size_t alignment = 32);

    /**
     * @brief Free aligned memory
     * @param ptr Pointer from AllocateAligned
     */
    static void FreeAligned(void* ptr);

    /**
     * @brief Allocate locked memory pages
     * @param size Size in bytes
     * @return Locked memory pointer
     */
    static void* AllocateLocked(size_t size);

    /**
     * @brief Free locked memory
     * @param ptr Pointer from AllocateLocked
     * @param size Original size
     */
    static void FreeLocked(void* ptr, size_t size);
};

/**
 * @brief Windows-specific performance utilities
 */
namespace utils {
    /**
     * @brief Get current thread's CPU usage
     */
    double GetCurrentThreadCpuUsage();

    /**
     * @brief Get process memory usage in MB
     */
    size_t GetProcessMemoryUsageMB();

    /**
     * @brief Get system memory information
     */
    MEMORYSTATUSEX GetSystemMemoryInfo();

    /**
     * @brief Check if running under Wine/Proton
     */
    bool IsRunningUnderWine();

    /**
     * @brief Get Windows build number
     */
    DWORD GetWindowsBuildNumber();

    /**
     * @brief Check if Windows Game Mode is active
     */
    bool IsGameModeActive();

    /**
     * @brief Enable Windows Game Mode
     */
    bool EnableGameMode();

    /**
     * @brief Disable Windows Game Mode
     */
    bool DisableGameMode();

    /**
     * @brief Get hardware acceleration capabilities
     */
    struct HardwareCapabilities {
        bool avx2Support;
        bool avx512Support;
        bool hyperthreading;
        bool turboBoost;
        int cacheLineSizeBytes;
        int l3CacheSizeKB;
    };
    HardwareCapabilities GetHardwareCapabilities();

    /**
     * @brief Optimize process for audio workload
     */
    bool OptimizeForAudioWorkload();

    /**
     * @brief Restore normal process settings
     */
    void RestoreNormalSettings();
}

} // namespace vrb::performance

#endif // _WIN32