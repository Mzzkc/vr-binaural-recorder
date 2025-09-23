// application.cpp - Enhanced application management implementation
#include "application.h"
#include <thread>
#include <chrono>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#endif

namespace vrb {

// Global application instance for signal handling
std::unique_ptr<Application> g_application = nullptr;

void signalHandler(int signal) {
    if (g_application) {
        LOG_INFO("Received signal {}, requesting shutdown", signal);
        g_application->requestShutdown();
    }
}

void setupSignalHandlers() {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
#ifdef SIGQUIT
    std::signal(SIGQUIT, signalHandler);
#endif
}

Application::Application()
    : m_frameTimeAverage(60)  // 60 frame rolling average
    , m_errorCount(0) {

    LOG_INFO("VR Binaural Recorder Application created");
}

Application::~Application() {
    if (m_state.load() != ApplicationState::Shutdown) {
        Shutdown();
    }
}

InitResult Application::Initialize() {
    PROFILE_SCOPE("Application::Initialize");

    setState(ApplicationState::Initializing);
    LOG_INFO("Starting application initialization");

    // Clear previous initialization results
    m_initResults.clear();

    // System validation first
    auto result = validateSystemRequirements();
    m_initResults.push_back(result);
    if (!result.success) {
        handleInitializationError(result);
        return result;
    }

    // Initialize components in dependency order
    std::vector<std::function<InitResult()>> initSteps = {
        [this]() { return initializeLogging(); },
        [this]() { return initializeConfig(); },
        [this]() { return initializeHRTF(); },
        [this]() { return initializeVRTracking(); },
        [this]() { return initializeAudioEngine(); },
        [this]() { return initializeHeadsetSupport(); },
        [this]() { return initializeOverlayUI(); }, // OVERLAY FIRST - Primary VR interface!
        [this]() { return initializeWindowsGUI(); }, // Desktop GUI is optional fallback
        [this]() { return connectComponents(); }
    };

    for (auto& step : initSteps) {
        result = step();
        m_initResults.push_back(result);

        if (!result.success) {
            handleInitializationError(result);
            return result;
        }
    }

    setState(ApplicationState::Running);
    m_running.store(true);

    LOG_INFO("Application initialization completed successfully");
    return InitResult(true, "Application");
}

void Application::Run() {
    if (m_state.load() != ApplicationState::Running) {
        LOG_ERROR("Cannot run application - not in running state");
        return;
    }

    LOG_INFO("Starting main application loop");
    m_mainLoopTimer.reset();

    // Start audio processing
    if (m_audioEngine && !m_audioEngine->Start()) {
        LOG_ERROR("Failed to start audio engine");
        setState(ApplicationState::Error);
        return;
    }

    // Start VR tracking
    if (m_vrTracker) {
        m_vrTracker->StartTracking();
    }

    // PRIMARY MODE: SteamVR Overlay Application!
    if (m_overlay && m_vrTracker) {
        LOG_INFO("Running as SteamVR OVERLAY APPLICATION - all controls accessible in VR!");

        // This is the main VR overlay loop - users interact entirely in VR!
        while (m_running.load() && m_state.load() == ApplicationState::Running) {
            Timer frameTimer;

            try {
                // Update VR overlay (this is the primary UI!)
                m_overlay->Update();

                // Process VR events
                processVREvents();

                // Update metrics
                updateMetrics();

                // Handle configuration reload
                handleConfigReload();

                // Record frame time
                double frameTime = frameTimer.elapsedMilliseconds();
                m_frameTimeAverage.add(frameTime);

                // Target 90fps for VR comfort and responsiveness
                const double targetFrameTime = 11.11; // ~90fps for VR
                if (frameTime < targetFrameTime) {
                    std::this_thread::sleep_for(
                        std::chrono::microseconds(static_cast<int>((targetFrameTime - frameTime) * 1000))
                    );
                }

            } catch (const std::exception& e) {
                LOG_ERROR("Exception in VR overlay loop: {}", e.what());
                m_errorCount++;

                if (m_errorCount > 10) {
                    LOG_CRITICAL("Too many errors in VR mode, stopping application");
                    setState(ApplicationState::Error);
                    break;
                }
            }
        }
    }
    // FALLBACK MODE: Desktop GUI (only if VR overlay failed)
    else if (m_windowsGUI) {
        LOG_WARN("VR overlay not available - falling back to desktop GUI mode");
        LOG_INFO("Starting Windows GUI main loop");
        int exitCode = m_windowsGUI->Run();
        LOG_INFO("Windows GUI exited with code: {}", exitCode);
        m_running.store(false);
    }
    // EMERGENCY MODE: Headless operation
    else {
        LOG_WARN("No UI available - running in headless mode");
        while (m_running.load() && m_state.load() == ApplicationState::Running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    LOG_INFO("Main loop ended");
}

void Application::Shutdown() {
    if (m_state.load() == ApplicationState::Shutdown) {
        return;
    }

    setState(ApplicationState::Stopping);
    LOG_INFO("Shutting down application");

    // Stop the main loop
    m_running.store(false);

    // Stop components in reverse order
    shutdownComponents();

    // Execute cleanup callbacks
    for (auto& callback : m_cleanupCallbacks) {
        try {
            callback();
        } catch (const std::exception& e) {
            LOG_WARN("Exception in cleanup callback: {}", e.what());
        }
    }

    // Validate shutdown
    validateShutdown();

    setState(ApplicationState::Shutdown);
    LOG_INFO("Application shutdown complete");
}

InitResult Application::validateSystemRequirements() {
    PROFILE_SCOPE("SystemValidation");

    LOG_INFO("Validating system requirements");

    // Get system information
    auto sysInfo = SystemValidation::getSystemInfo();
    LOG_INFO("System: {} {} - {} cores, {} MB RAM",
             sysInfo.os, sysInfo.architecture, sysInfo.cpuCores, sysInfo.totalMemory / (1024*1024));

    // Validate each subsystem
    auto vrResult = SystemValidation::validateVRRequirements();
    if (!vrResult.success) {
        LOG_WARN("VR validation: {}", vrResult.errorMessage);
    }

    auto audioResult = SystemValidation::validateAudioRequirements();
    if (!audioResult.success) {
        return audioResult; // Audio is critical
    }

    auto depResult = SystemValidation::validateDependencies();
    if (!depResult.success) {
        return depResult; // Dependencies are critical
    }

    auto perfResult = SystemValidation::validatePerformance();
    if (!perfResult.success) {
        LOG_WARN("Performance validation: {}", perfResult.errorMessage);
        // Performance issues are warnings, not failures
    }

    return InitResult(true, "SystemValidation");
}

InitResult Application::initializeLogging() {
    // Logging is already initialized by static initialization
    // This is a placeholder for any additional logging setup
    LOG_INFO("Logging system ready");
    return InitResult(true, "Logging");
}

InitResult Application::initializeConfig() {
    PROFILE_SCOPE("Config::Initialize");

    try {
        m_config = std::make_unique<Config>("vr_binaural_config.json");
        LOG_INFO("Configuration loaded from: vr_binaural_config.json");

        // Update logger level if specified in config
        Logger::SetLevel(m_config->GetLogLevel());

        return InitResult(true, "Config");
    } catch (const std::exception& e) {
        return InitResult(false, "Config", e.what());
    }
}

InitResult Application::initializeHRTF() {
    PROFILE_SCOPE("HRTF::Initialize");

    try {
        m_hrtf = std::make_unique<HRTFProcessor>();

        if (!m_hrtf->Initialize(m_config->GetHRTFDataPath())) {
            return InitResult(false, "HRTF", "Failed to initialize HRTF processor");
        }

        LOG_INFO("HRTF processor initialized with dataset: {}", m_config->GetHRTFDataPath());
        return InitResult(true, "HRTF");
    } catch (const std::exception& e) {
        return InitResult(false, "HRTF", e.what());
    }
}

InitResult Application::initializeVRTracking() {
    PROFILE_SCOPE("VR::Initialize");

    try {
        m_vrTracker = std::make_unique<VRTracker>();

        if (!m_vrTracker->Initialize()) {
            // VR might not be available - log warning but continue
            LOG_WARN("VR tracking not available - running in desktop mode");
            return InitResult(true, "VR", "VR not available - desktop mode");
        }

        LOG_INFO("VR tracking initialized");
        return InitResult(true, "VR");
    } catch (const std::exception& e) {
        // VR failure is not critical - continue without VR
        LOG_WARN("VR initialization failed: {} - continuing without VR", e.what());
        return InitResult(true, "VR", "VR disabled: " + std::string(e.what()));
    }
}

InitResult Application::initializeAudioEngine() {
    PROFILE_SCOPE("Audio::Initialize");

    try {
        m_audioEngine = std::make_unique<AudioEngine>();

        if (!m_audioEngine->Initialize(*m_config, m_hrtf.get())) {
            return InitResult(false, "Audio", "Failed to initialize audio engine");
        }

        LOG_INFO("Audio engine initialized");
        return InitResult(true, "Audio");
    } catch (const std::exception& e) {
        return InitResult(false, "Audio", e.what());
    }
}

InitResult Application::initializeOverlayUI() {
    PROFILE_SCOPE("UI::Initialize");

    try {
        m_overlay = std::make_unique<OverlayUI>();

        if (!m_overlay->Initialize(m_vrTracker.get(), m_audioEngine.get())) {
            // UI failure is not critical if we don't have VR
            if (!m_vrTracker || !m_vrTracker->IsConnected()) {
                LOG_WARN("Overlay UI disabled - no VR runtime available");
                return InitResult(true, "UI", "UI disabled - no VR");
            }
            return InitResult(false, "UI", "Failed to initialize overlay UI");
        }

        LOG_INFO("Overlay UI initialized");
        return InitResult(true, "UI");
    } catch (const std::exception& e) {
        // UI failure is not critical in desktop mode
        if (!m_vrTracker || !m_vrTracker->IsConnected()) {
            LOG_WARN("Overlay UI disabled: {}", e.what());
            return InitResult(true, "UI", "UI disabled: " + std::string(e.what()));
        }
        return InitResult(false, "UI", e.what());
    }
}

InitResult Application::initializeWindowsGUI() {
    PROFILE_SCOPE("WindowsGUI::Initialize");
    try {
        LOG_INFO("Initializing Windows GUI...");
        m_windowsGUI = std::make_unique<WindowsGUI>();

        if (!m_windowsGUI->Initialize(m_config.get())) {
            return InitResult(false, "WindowsGUI", "Failed to initialize Windows GUI");
        }

        LOG_INFO("Windows GUI initialized successfully");
        return InitResult(true, "WindowsGUI");
    } catch (const std::exception& e) {
        LOG_ERROR("Windows GUI initialization failed: {}", e.what());
        return InitResult(false, "WindowsGUI", e.what());
    }
}

InitResult Application::initializeHeadsetSupport() {
    PROFILE_SCOPE("HeadsetSupport::Initialize");
    try {
        LOG_INFO("Initializing headset support systems...");
        m_headsetSupport = std::make_unique<HeadsetSupportManager>();

        if (!m_headsetSupport->Initialize()) {
            LOG_WARN("Headset support initialization had issues, but continuing");
            return InitResult(true, "HeadsetSupport", "Partial initialization");
        }

        // Detect connected headsets
        if (m_headsetSupport->DetectConnectedHeadset()) {
            auto headset = m_headsetSupport->GetConnectedHeadset();
            LOG_INFO("Detected VR headset: {}", headset.modelName);
        }

        LOG_INFO("Headset support initialized successfully");
        return InitResult(true, "HeadsetSupport");
    } catch (const std::exception& e) {
        LOG_WARN("Headset support initialization failed: {}", e.what());
        // Headset support failure is not critical
        return InitResult(true, "HeadsetSupport", "Failed: " + std::string(e.what()));
    }
}

InitResult Application::connectComponents() {
    PROFILE_SCOPE("Components::Connect");

    try {
        // Set up VR tracking callback if VR is available
        if (m_vrTracker && m_vrTracker->IsConnected() && m_hrtf) {
            m_vrTracker->SetTrackingCallback([this](const VRPose& headPose, const VRPose& micPose) {
                m_hrtf->UpdateSpatialPosition(headPose, micPose);
            });
            LOG_INFO("VR tracking connected to HRTF processor");
        }

        // Set up overlay parameter callbacks if UI is available
        if (m_overlay && m_audioEngine) {
            m_overlay->RegisterParameterCallback([this](const std::string& param, float value) {
                // Forward parameter changes to audio engine
                if (m_audioEngine) {
                    // TODO: Implement parameter forwarding
                    LOG_DEBUG("Parameter changed: {} = {}", param, value);
                }
            });
            LOG_INFO("Overlay UI connected to audio engine");
        }

        // Connect Windows GUI to audio engine and VR tracker
        if (m_windowsGUI) {
            if (m_audioEngine) {
                m_windowsGUI->ConnectAudioEngine(std::shared_ptr<AudioEngine>(m_audioEngine.get(), [](AudioEngine*){}));
                LOG_INFO("Windows GUI connected to audio engine");
            }
            if (m_vrTracker) {
                m_windowsGUI->ConnectVRTracker(std::shared_ptr<VRTracker>(m_vrTracker.get(), [](VRTracker*){}));
                LOG_INFO("Windows GUI connected to VR tracker");
            }
        }

        // Connect headset support to audio engine
        if (m_headsetSupport && m_audioEngine) {
            m_headsetSupport->OptimizeAudioForHeadset(m_audioEngine.get());
            LOG_INFO("Headset support connected to audio engine");
        }

        return InitResult(true, "ComponentConnections");
    } catch (const std::exception& e) {
        return InitResult(false, "ComponentConnections", e.what());
    }
}

void Application::shutdownComponents() {
    // Stop audio processing first to prevent audio artifacts
    if (m_audioEngine) {
        m_audioEngine->Stop();
    }

    // Stop VR tracking
    if (m_vrTracker) {
        m_vrTracker->StopTracking();
    }

    // Shutdown UI components
    if (m_windowsGUI) {
        m_windowsGUI->Shutdown();
    }
    if (m_overlay) {
        m_overlay->Shutdown();
    }

    // Shutdown headset support
    if (m_headsetSupport) {
        m_headsetSupport->Shutdown();
    }

    // Clean up components in reverse order of initialization
    m_windowsGUI.reset();
    m_headsetSupport.reset();
    m_overlay.reset();
    m_audioEngine.reset();
    m_vrTracker.reset();
    m_hrtf.reset();
    m_config.reset();
}

void Application::validateShutdown() {
    // Check that all components are properly cleaned up
    bool cleanShutdown = true;

    if (m_overlay) {
        LOG_WARN("Overlay UI not properly cleaned up");
        cleanShutdown = false;
    }

    if (m_audioEngine) {
        LOG_WARN("Audio engine not properly cleaned up");
        cleanShutdown = false;
    }

    if (m_vrTracker) {
        LOG_WARN("VR tracker not properly cleaned up");
        cleanShutdown = false;
    }

    if (cleanShutdown) {
        LOG_INFO("All components shut down cleanly");
    } else {
        LOG_WARN("Some components did not shut down cleanly");
    }
}

void Application::handleInitializationError(const InitResult& result) {
    setState(ApplicationState::Error);
    LOG_ERROR("Initialization failed at component '{}': {}", result.component, result.errorMessage);

    // Attempt partial cleanup
    shutdownComponents();
}

void Application::setState(ApplicationState newState) {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    ApplicationState oldState = m_state.load();
    m_state.store(newState);

    LOG_DEBUG("Application state changed: {} -> {}",
              static_cast<int>(oldState), static_cast<int>(newState));
}

void Application::processVREvents() {
    if (m_vrTracker) {
        m_vrTracker->ProcessEvents();
    }
}

void Application::updateMetrics() {
    // Update performance metrics periodically
    static auto lastUpdate = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();

    if (std::chrono::duration_cast<std::chrono::seconds>(now - lastUpdate).count() >= 1) {
        // Log performance metrics every second
        double avgFrameTime = m_frameTimeAverage.getAverage();
        LOG_DEBUG("Average frame time: {:.2f}ms ({:.1f} FPS)",
                  avgFrameTime, 1000.0 / avgFrameTime);

        lastUpdate = now;
    }
}

void Application::handleConfigReload() {
    if (m_config && m_config->HasChanged()) {
        LOG_INFO("Configuration file changed, reloading");
        auto result = applyConfigChanges();
        if (!result.success) {
            LOG_ERROR("Failed to apply configuration changes: {}", result.errorMessage);
        }
    }
}

InitResult Application::applyConfigChanges() {
    try {
        m_config->Reload();

        // Update logger level
        Logger::SetLevel(m_config->GetLogLevel());

        // Update audio engine configuration
        if (m_audioEngine) {
            m_audioEngine->UpdateConfiguration(*m_config);
        }

        LOG_INFO("Configuration changes applied successfully");
        return InitResult(true, "ConfigReload");
    } catch (const std::exception& e) {
        return InitResult(false, "ConfigReload", e.what());
    }
}

void Application::registerCleanupCallback(std::function<void()> callback) {
    m_cleanupCallbacks.push_back(callback);
}

// System validation implementations
namespace SystemValidation {

SystemInfo getSystemInfo() {
    SystemInfo info;

#ifdef _WIN32
    info.os = "Windows";
    info.architecture = "x64";

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    info.cpuCores = sysInfo.dwNumberOfProcessors;

    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    info.totalMemory = memInfo.ullTotalPhys;
    info.availableMemory = memInfo.ullAvailPhys;
#else
    struct utsname unameData;
    uname(&unameData);
    info.os = unameData.sysname;
    info.architecture = unameData.machine;

    info.cpuCores = sysconf(_SC_NPROCESSORS_ONLN);

    struct sysinfo sysInfo;
    sysinfo(&sysInfo);
    info.totalMemory = sysInfo.totalram * sysInfo.mem_unit;
    info.availableMemory = sysInfo.freeram * sysInfo.mem_unit;
#endif

    return info;
}

InitResult validateVRRequirements() {
    // Check if OpenVR runtime is available
    // This is a simplified check - actual implementation would be more thorough
    return InitResult(true, "VRRequirements", "VR validation not implemented");
}

InitResult validateAudioRequirements() {
    // Check audio system availability
    // This would check for audio drivers, devices, etc.
    return InitResult(true, "AudioRequirements");
}

InitResult validateDependencies() {
    // Check for required libraries and dependencies
    return InitResult(true, "Dependencies");
}

InitResult validatePerformance() {
    // Check system performance capabilities
    auto info = getSystemInfo();

    if (info.cpuCores < 2) {
        return InitResult(false, "Performance", "Insufficient CPU cores (minimum 2 required)");
    }

    if (info.availableMemory < 1024 * 1024 * 1024) { // 1GB
        return InitResult(false, "Performance", "Insufficient available memory (minimum 1GB required)");
    }

    return InitResult(true, "Performance");
}

} // namespace SystemValidation

} // namespace vrb