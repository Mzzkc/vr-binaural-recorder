// main.cpp - VR Binaural Recorder Entry Point
// Copyright (c) 2024 - Production Ready Spatial Audio for VR

#include <memory>
#include <thread>
#include <atomic>
#include <csignal>
#include "logger.h"
#include "config.h"
#include "audio_engine.h"
#include "vr_tracker.h"
#include "hrtf_processor.h"
#include "overlay_ui.h"

namespace vrb {

std::atomic<bool> g_running{true};

void SignalHandler(int signal) {
    LOG_INFO("Received signal {}, initiating shutdown", signal);
    g_running = false;
}

class Application {
public:
    Application() : m_config("vr_binaural_config.json") {
        LOG_INFO("VR Binaural Recorder v1.0.0 initializing");
    }

    ~Application() {
        Shutdown();
    }

    bool Initialize() {
        try {
            // Initialize logging
            Logger::Initialize(m_config.GetLogLevel(), m_config.GetLogPath());
            LOG_INFO("Logger initialized");

            // Initialize HRTF processor
            m_hrtf = std::make_unique<HRTFProcessor>();
            if (!m_hrtf->Initialize(m_config.GetHRTFDataPath())) {
                LOG_ERROR("Failed to initialize HRTF processor");
                return false;
            }
            LOG_INFO("HRTF processor initialized with dataset: {}", m_config.GetHRTFDataPath());

            // Initialize VR tracking
            m_vrTracker = std::make_unique<VRTracker>();
            if (!m_vrTracker->Initialize()) {
                LOG_ERROR("Failed to initialize VR tracking");
                return false;
            }
            LOG_INFO("VR tracking initialized");

            // Initialize audio engine
            m_audioEngine = std::make_unique<AudioEngine>();
            if (!m_audioEngine->Initialize(m_config, m_hrtf.get())) {
                LOG_ERROR("Failed to initialize audio engine");
                return false;
            }
            LOG_INFO("Audio engine initialized");

            // Initialize overlay UI
            m_overlay = std::make_unique<OverlayUI>();
            if (!m_overlay->Initialize(m_vrTracker.get(), m_audioEngine.get())) {
                LOG_ERROR("Failed to initialize overlay UI");
                return false;
            }
            LOG_INFO("Overlay UI initialized");

            // Set up VR tracking callback
            m_vrTracker->SetTrackingCallback([this](const VRPose& headPose, const VRPose& micPose) {
                m_hrtf->UpdateSpatialPosition(headPose, micPose);
            });

            LOG_INFO("Application initialized successfully");
            return true;

        } catch (const std::exception& e) {
            LOG_CRITICAL("Initialization failed: {}", e.what());
            return false;
        }
    }

    void Run() {
        LOG_INFO("Starting main application loop");
        
        // Start audio processing
        if (!m_audioEngine->Start()) {
            LOG_ERROR("Failed to start audio engine");
            return;
        }

        // Start VR tracking
        m_vrTracker->StartTracking();

        // Main loop
        while (g_running) {
            // Update overlay
            m_overlay->Update();

            // Process VR events
            m_vrTracker->ProcessEvents();

            // Check for configuration changes
            if (m_config.HasChanged()) {
                ApplyConfigChanges();
            }

            // Small sleep to prevent CPU spinning
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        LOG_INFO("Main loop ended");
    }

private:
    void Shutdown() {
        LOG_INFO("Shutting down application");

        if (m_audioEngine) {
            m_audioEngine->Stop();
        }

        if (m_vrTracker) {
            m_vrTracker->StopTracking();
        }

        if (m_overlay) {
            m_overlay->Shutdown();
        }

        LOG_INFO("Application shutdown complete");
    }

    void ApplyConfigChanges() {
        LOG_INFO("Applying configuration changes");
        m_config.Reload();
        
        // Update components with new config
        if (m_audioEngine) {
            m_audioEngine->UpdateConfiguration(m_config);
        }
    }

    Config m_config;
    std::unique_ptr<HRTFProcessor> m_hrtf;
    std::unique_ptr<VRTracker> m_vrTracker;
    std::unique_ptr<AudioEngine> m_audioEngine;
    std::unique_ptr<OverlayUI> m_overlay;
};

} // namespace vrb

int main(int argc, char* argv[]) {
    // Set up signal handlers
    std::signal(SIGINT, vrb::SignalHandler);
    std::signal(SIGTERM, vrb::SignalHandler);

    // Create and run application
    vrb::Application app;
    
    if (!app.Initialize()) {
        std::cerr << "Failed to initialize VR Binaural Recorder" << std::endl;
        return 1;
    }

    app.Run();
    
    return 0;
}