// premium_headset_integration.cpp
// Example demonstrating Quest Pro and Bigscreen Beyond integration
// This shows how to use the enhanced VR binaural recorder with premium headsets

#include <iostream>
#include <memory>
#include <chrono>
#include <thread>

#include "config.h"
#include "audio_engine.h"
#include "hrtf_processor.h"
#include "vr_tracker.h"
#include "vr_hardware_detection.h"
#include "logger.h"

using namespace vrb;

class PremiumHeadsetDemo {
public:
    PremiumHeadsetDemo() = default;

    bool Initialize() {
        std::cout << "=== VR Binaural Recorder - Premium Headset Integration Demo ===\n\n";

        // Initialize configuration
        m_config = std::make_unique<Config>();
        if (!m_config->LoadFromFile("config/vr_binaural_config.json")) {
            std::cerr << "Warning: Could not load config, using defaults\n";
        }

        // Create and initialize VR tracker with hardware detection
        m_vrTracker = std::make_unique<VRTracker>();
        if (!m_vrTracker->Initialize()) {
            std::cerr << "Error: Failed to initialize VR tracker\n";
            return false;
        }

        // Get detected headset information
        VRHeadsetInfo headset = m_vrTracker->GetDetectedHeadset();
        std::cout << "Hardware Status:\n" << m_vrTracker->GetHardwareStatus() << "\n";

        // Create HRTF processor
        m_hrtfProcessor = std::make_unique<HRTFProcessor>();

        // Create audio engine
        m_audioEngine = std::make_unique<AudioEngine>();

        // Initialize with headset-specific optimizations if headset detected
        bool audioInitialized = false;
        if (!headset.modelName.empty()) {
            std::cout << "Initializing with headset-specific optimizations...\n";
            audioInitialized = m_audioEngine->InitializeWithHeadset(*m_config, m_hrtfProcessor.get(), headset);
        } else {
            std::cout << "No headset detected, using generic initialization...\n";
            audioInitialized = m_audioEngine->Initialize(*m_config, m_hrtfProcessor.get());
        }

        if (!audioInitialized) {
            std::cerr << "Error: Failed to initialize audio engine\n";
            return false;
        }

        // Display headset-specific features
        DisplayHeadsetFeatures(headset);

        return true;
    }

    void DisplayHeadsetFeatures(const VRHeadsetInfo& headset) {
        if (headset.modelName.empty()) {
            std::cout << "No VR headset detected. Running in simulation mode.\n\n";
            return;
        }

        std::cout << "\n=== Headset-Specific Features ===\n";
        std::cout << "Model: " << headset.modelName << "\n";

        // Quest Pro features
        if (headset.vendor == VRVendor::Meta && headset.modelName.find("Quest Pro") != std::string::npos) {
            std::cout << "\n*** META QUEST PRO PREMIUM FEATURES ***\n";

            if (headset.hasOffEarAudio) {
                std::cout << "✓ Off-ear spatial audio design - Enhanced ambient awareness\n";
            }

            if (headset.hasHandTracking) {
                std::cout << "✓ Advanced hand tracking - Natural gesture control\n";
            }

            if (headset.hasEyeTracking) {
                std::cout << "✓ Eye tracking - Foveated rendering support\n";
            }

            if (headset.hasColorPassthrough) {
                std::cout << "✓ Mixed Reality - Color passthrough cameras\n";
            }

            if (headset.hasIPDAdjustment) {
                std::cout << "✓ Motorized IPD - Automatic adjustment ("
                         << headset.ipd_min << "-" << headset.ipd_max << "mm)\n";
            }

            std::cout << "✓ Wireless connectivity - Air Link and USB-C Link\n";
            std::cout << "✓ High refresh rate - 72/90/120Hz support\n";
            std::cout << "✓ Resolution: 1800x1920 per eye\n";
        }

        // Bigscreen Beyond features
        else if (headset.vendor == VRVendor::Bigscreen && headset.modelName.find("Beyond") != std::string::npos) {
            std::cout << "\n*** BIGSCREEN BEYOND ULTRA-PREMIUM FEATURES ***\n";

            std::cout << "✓ Ultra-lightweight - Only 127g total weight\n";
            std::cout << "✓ Sub-10ms latency - Motion-to-photon optimization\n";
            std::cout << "✓ Ultra-high resolution - 2560x2560 per eye micro-OLED\n";

            if (headset.hasDMASAudio) {
                std::cout << "✓ DMAS Audio - Detachable Modular Audio Strap\n";
                std::cout << "  - High-fidelity 96kHz audio processing\n";
                std::cout << "  - Flat frequency response\n";
            }

            if (headset.hasLighthouseTracking) {
                std::cout << "✓ SteamVR 2.0 Lighthouse - Sub-millimeter precision\n";
            }

            if (headset.hasCustomFittedLenses) {
                std::cout << "✓ Custom prescription lenses - No IPD adjustment needed\n";
            }

            std::cout << "✓ DisplayPort 1.4+ connection - Maximum bandwidth\n";
            std::cout << "✓ Refresh rate: 75Hz/90Hz\n";
        }

        // Display supported features
        if (!headset.supportedFeatures.empty()) {
            std::cout << "\nSupported Features:\n";
            for (const auto& feature : headset.supportedFeatures) {
                std::cout << "  • " << feature << "\n";
            }
        }

        // Display known limitations
        if (!headset.knownLimitations.empty()) {
            std::cout << "\nImportant Notes:\n";
            for (const auto& limitation : headset.knownLimitations) {
                std::cout << "  ⚠ " << limitation << "\n";
            }
        }

        std::cout << "\n";
    }

    void RunDemo() {
        if (!Initialize()) {
            return;
        }

        std::cout << "=== Starting Audio Processing Demo ===\n";

        // Start VR tracking
        m_vrTracker->StartTracking();

        // Start audio engine
        if (!m_audioEngine->Start()) {
            std::cerr << "Error: Failed to start audio engine\n";
            return;
        }

        std::cout << "Audio processing started. Press Enter to stop...\n";

        // Set up tracking callback for real-time updates
        m_vrTracker->SetTrackingCallback([this](const VRPose& head, const VRPose& mic) {
            // Update HRTF with current poses
            if (m_hrtfProcessor) {
                m_hrtfProcessor->UpdateSpatialPosition(head, mic);
            }

            // Log position updates periodically
            static auto lastLog = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - lastLog).count() >= 5) {
                std::cout << "Head position: ("
                         << std::fixed << std::setprecision(2)
                         << head.position.x << ", "
                         << head.position.y << ", "
                         << head.position.z << ")\n";
                lastLog = now;
            }
        });

        // Wait for user input
        std::cin.get();

        // Cleanup
        std::cout << "Stopping audio processing...\n";
        m_audioEngine->Stop();
        m_vrTracker->StopTracking();

        // Display final statistics
        auto stats = m_audioEngine->GetStats();
        std::cout << "\n=== Session Statistics ===\n";
        std::cout << "Frames processed: " << stats.framesProcessed << "\n";
        std::cout << "Input latency: " << std::fixed << std::setprecision(2)
                 << stats.inputLatency * 1000.0 << "ms\n";
        std::cout << "Output latency: " << std::fixed << std::setprecision(2)
                 << stats.outputLatency * 1000.0 << "ms\n";
        std::cout << "Peak input level: " << stats.peakInputLevel << "\n";
        std::cout << "Peak output level: " << stats.peakOutputLevel << "\n";
        std::cout << "Buffer underruns: " << stats.underruns << "\n";
        std::cout << "Buffer overruns: " << stats.overruns << "\n";

        std::cout << "\nDemo completed successfully!\n";
    }

private:
    std::unique_ptr<Config> m_config;
    std::unique_ptr<AudioEngine> m_audioEngine;
    std::unique_ptr<HRTFProcessor> m_hrtfProcessor;
    std::unique_ptr<VRTracker> m_vrTracker;
};

int main() {
    try {
        PremiumHeadsetDemo demo;
        demo.RunDemo();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}