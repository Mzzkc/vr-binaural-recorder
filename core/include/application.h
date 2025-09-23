// application.h - Enhanced application management with robust initialization
#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <atomic>
#include <string>
#include <csignal>
#include "logger.h"
#include "config.h"
#include "audio_engine.h"
#include "vr_tracker.h"
#include "hrtf_processor.h"
#include "overlay_ui.h"
#include "utils.h"

namespace vrb {

/**
 * @brief Application states for robust state management
 */
enum class ApplicationState {
    Uninitialized,
    Initializing,
    Running,
    Stopping,
    Error,
    Shutdown
};

/**
 * @brief Result of initialization steps
 */
struct InitResult {
    bool success;
    std::string component;
    std::string errorMessage;

    InitResult(bool success = true, const std::string& component = "", const std::string& error = "")
        : success(success), component(component), errorMessage(error) {}
};

/**
 * @brief Component interface for managed initialization
 */
class IComponent {
public:
    virtual ~IComponent() = default;
    virtual const char* getName() const = 0;
    virtual InitResult initialize() = 0;
    virtual void shutdown() = 0;
    virtual bool isInitialized() const = 0;
};

/**
 * @brief Enhanced application manager with robust initialization and cleanup
 */
class Application {
public:
    Application();
    ~Application();

    /**
     * @brief Initialize application with comprehensive error handling
     */
    InitResult Initialize();

    /**
     * @brief Main application loop with error recovery
     */
    void Run();

    /**
     * @brief Graceful shutdown with cleanup validation
     */
    void Shutdown();

    /**
     * @brief Get current application state
     */
    ApplicationState getState() const { return m_state; }

    /**
     * @brief Check if application should continue running
     */
    bool isRunning() const { return m_running.load(); }

    /**
     * @brief Request application shutdown
     */
    void requestShutdown() { m_running.store(false); }

    /**
     * @brief Get initialization results for diagnostics
     */
    const std::vector<InitResult>& getInitResults() const { return m_initResults; }

    /**
     * @brief Register cleanup callback
     */
    void registerCleanupCallback(std::function<void()> callback);

    /**
     * @brief Apply configuration changes with validation
     */
    InitResult applyConfigChanges();

    /**
     * @brief Validate system requirements
     */
    InitResult validateSystemRequirements();

private:
    // Initialization phases
    InitResult initializeLogging();
    InitResult initializeConfig();
    InitResult initializeHRTF();
    InitResult initializeVRTracking();
    InitResult initializeAudioEngine();
    InitResult initializeOverlayUI();
    InitResult initializeWindowsGUI();
    InitResult initializeHeadsetSupport();
    InitResult connectComponents();

    // Shutdown phases
    void shutdownComponents();
    void validateShutdown();

    // Error handling
    void handleInitializationError(const InitResult& result);
    void setState(ApplicationState newState);

    // Main loop methods
    void processVREvents();
    void updateMetrics();
    void handleConfigReload();

private:
    std::atomic<ApplicationState> m_state{ApplicationState::Uninitialized};
    std::atomic<bool> m_running{false};

    // Configuration
    std::unique_ptr<Config> m_config;

    // Core components
    std::unique_ptr<HRTFProcessor> m_hrtf;
    std::unique_ptr<VRTracker> m_vrTracker;
    std::unique_ptr<AudioEngine> m_audioEngine;
    std::unique_ptr<OverlayUI> m_overlay;

    // Component management
    std::vector<std::unique_ptr<IComponent>> m_components;
    std::vector<InitResult> m_initResults;
    std::vector<std::function<void()>> m_cleanupCallbacks;

    // Performance monitoring
    Timer m_mainLoopTimer;
    MovingAverage<double> m_frameTimeAverage;

    // Error recovery
    int m_errorCount;
    std::chrono::steady_clock::time_point m_lastError;

    mutable std::mutex m_stateMutex;
};

/**
 * @brief Global application instance for signal handling
 */
extern std::unique_ptr<Application> g_application;

/**
 * @brief Signal handler for graceful shutdown
 */
void signalHandler(int signal);

/**
 * @brief Initialize global signal handlers
 */
void setupSignalHandlers();

/**
 * @brief System validation utilities
 */
namespace SystemValidation {

    struct SystemInfo {
        std::string os;
        std::string architecture;
        int cpuCores;
        size_t totalMemory;
        size_t availableMemory;
        bool hasVRCapable;
        bool hasAudioCapable;
        std::vector<std::string> audioDevices;
        std::string vrRuntime;
    };

    /**
     * @brief Get comprehensive system information
     */
    SystemInfo getSystemInfo();

    /**
     * @brief Validate VR system requirements
     */
    InitResult validateVRRequirements();

    /**
     * @brief Validate audio system requirements
     */
    InitResult validateAudioRequirements();

    /**
     * @brief Check for required dependencies
     */
    InitResult validateDependencies();

    /**
     * @brief Performance capability check
     */
    InitResult validatePerformance();
}

} // namespace vrb