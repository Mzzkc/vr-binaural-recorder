// vr_demo.cpp - VR Binaural Recorder Beta Demo
// Show off our amazing VR spatial audio capabilities!

#include "vr_tracker.h"
#include "vr_hardware_detection.h"
#include "vr_capabilities_manager.h"
#include "vr_testing_framework.h"
#include "logger.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>

using namespace vrb;

class VRBetaDemo {
public:
    VRBetaDemo() {
        std::cout << "🎵 VR Binaural Recorder - Beta Demo\n";
        std::cout << "===================================\n\n";

        // Initialize logger
        Logger::Initialize("info", "./demo_logs", true);

        // Initialize our systems
        m_capManager = std::make_unique<VRCapabilitiesManager>();
        m_tracker = std::make_unique<VRTracker>();
        m_testFramework = std::make_unique<VRTestingFramework>();
    }

    void RunDemo() {
        ShowWelcomeMessage();
        DetectSystemCapabilities();
        DemonstrateVRTracking();
        ShowSpatialAudioDemo();
        RunQuickValidation();
        ShowBetaRecommendations();
        ShowConclusion();
    }

private:
    std::unique_ptr<VRCapabilitiesManager> m_capManager;
    std::unique_ptr<VRTracker> m_tracker;
    std::unique_ptr<VRTestingFramework> m_testFramework;

    void ShowWelcomeMessage() {
        std::cout << "Welcome to the future of spatial audio!\n\n";
        std::cout << "This demo will showcase the VR Binaural Recorder's capabilities\n";
        std::cout << "on your system. No VR hardware? No problem! Our simulation mode\n";
        std::cout << "provides a full spatial audio experience.\n\n";

        std::cout << "Press Enter to begin the demo...";
        std::cin.get();
        std::cout << "\n";
    }

    void DetectSystemCapabilities() {
        std::cout << "🔍 Step 1: Detecting System Capabilities\n";
        std::cout << "========================================\n";

        // Initialize and scan capabilities
        m_capManager->Initialize();

        SystemProfile profile = m_capManager->GetSystemProfile();
        std::cout << "System Profile: ";

        switch (profile) {
            case SystemProfile::Minimal:
                std::cout << "Minimal (Perfect for testing basic features)\n";
                break;
            case SystemProfile::Standard:
                std::cout << "Standard (Great for typical VR experiences)\n";
                break;
            case SystemProfile::Enhanced:
                std::cout << "Enhanced (Excellent for high-quality spatial audio)\n";
                break;
            case SystemProfile::Premium:
                std::cout << "Premium (Maximum quality available!)\n";
                break;
            case SystemProfile::Enterprise:
                std::cout << "Enterprise (Professional-grade capabilities)\n";
                break;
        }

        std::cout << "\nCapability Highlights:\n";

        // Check key capabilities
        auto checkCapability = [&](const std::string& name, const std::string& description) {
            CapabilityLevel level = m_capManager->GetCapabilityLevel(name);
            std::cout << "  " << description << ": ";
            switch (level) {
                case CapabilityLevel::None: std::cout << "❌ Not Available"; break;
                case CapabilityLevel::Basic: std::cout << "🔶 Basic"; break;
                case CapabilityLevel::Standard: std::cout << "✅ Standard"; break;
                case CapabilityLevel::Enhanced: std::cout << "🌟 Enhanced"; break;
                case CapabilityLevel::Premium: std::cout << "💎 Premium"; break;
            }
            std::cout << "\n";
        };

        checkCapability("vr_headset_tracking", "VR Hardware");
        checkCapability("real_time_hrtf", "Spatial Audio Processing");
        checkCapability("low_latency_audio", "Low Latency Audio");
        checkCapability("multi_source_audio", "Multiple Audio Sources");

        bool betaCompatible = m_capManager->IsBetaCompatible();
        std::cout << "\nBeta Compatibility: " << (betaCompatible ? "✅ COMPATIBLE" : "❌ ISSUES DETECTED") << "\n";

        std::cout << "\nPress Enter to continue...";
        std::cin.get();
        std::cout << "\n";
    }

    void DemonstrateVRTracking() {
        std::cout << "🎯 Step 2: VR Tracking Demonstration\n";
        std::cout << "====================================\n";

        // Initialize VR tracker
        m_tracker->Initialize();

        std::string hardwareStatus = m_tracker->GetHardwareStatus();
        std::cout << "Hardware Status:\n" << hardwareStatus << "\n";

        if (m_tracker->IsSimulationMode()) {
            std::cout << "🎭 Running in Simulation Mode\n";
            std::cout << "This mode provides realistic head movement patterns that\n";
            std::cout << "demonstrate spatial audio without requiring VR hardware.\n\n";
        } else {
            std::cout << "🥽 Real VR Hardware Detected!\n";
            std::cout << "Your VR headset will provide accurate head tracking for\n";
            std::cout << "the most immersive spatial audio experience.\n\n";
        }

        // Start tracking and show some pose data
        std::cout << "Starting tracking demonstration...\n";
        m_tracker->StartTracking();

        std::cout << "Head position over 5 seconds:\n";
        for (int i = 0; i < 50; ++i) {
            VRPose headPose = m_tracker->GetHeadPose();
            if (headPose.isValid) {
                std::cout << "\r  Position: ("
                          << std::fixed << std::setprecision(2)
                          << std::setw(6) << headPose.position.x << ", "
                          << std::setw(6) << headPose.position.y << ", "
                          << std::setw(6) << headPose.position.z << ")";
                std::cout.flush();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        m_tracker->StopTracking();
        std::cout << "\n\nTracking demonstration complete!\n";

        std::cout << "\nPress Enter to continue...";
        std::cin.get();
        std::cout << "\n";
    }

    void ShowSpatialAudioDemo() {
        std::cout << "🔊 Step 3: Spatial Audio Simulation\n";
        std::cout << "===================================\n";

        std::cout << "In a real application, you would now hear:\n\n";

        std::cout << "🎵 Sound Source Demo:\n";
        std::cout << "  • Guitar playing to your left (-2m, 0m, 0m)\n";
        std::cout << "  • Violin playing to your right (+2m, 0m, 0m)\n";
        std::cout << "  • Piano playing in front (0m, 0m, +2m)\n";
        std::cout << "  • Drums playing behind (0m, 0m, -2m)\n\n";

        // Simulate audio source movement
        std::cout << "🌀 Moving Sound Demo:\n";
        std::cout << "Simulating a sound source moving in a circle around you...\n";

        for (int i = 0; i < 36; ++i) {
            float angle = i * 10.0f * 3.14159f / 180.0f; // 10 degrees per step
            float x = 2.0f * std::cos(angle);
            float z = 2.0f * std::sin(angle);

            std::cout << "\r  Source at: ("
                      << std::fixed << std::setprecision(1)
                      << std::setw(5) << x << ", 1.6, "
                      << std::setw(5) << z << ") - "
                      << std::setw(3) << (i * 10) << "°";
            std::cout.flush();

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        std::cout << "\n\n🎧 With headphones, you would experience:\n";
        std::cout << "  • Accurate left/right positioning\n";
        std::cout << "  • Natural front/back localization\n";
        std::cout << "  • Realistic distance perception\n";
        std::cout << "  • Smooth movement tracking\n";
        std::cout << "  • Head movement adaptation\n";

        std::cout << "\nPress Enter to continue...";
        std::cin.get();
        std::cout << "\n";
    }

    void RunQuickValidation() {
        std::cout << "✅ Step 4: Quick System Validation\n";
        std::cout << "==================================\n";

        bool isValid = m_testFramework->ValidateSystemForBeta();
        std::cout << "System Validation: " << (isValid ? "✅ PASSED" : "❌ FAILED") << "\n\n";

        if (isValid) {
            std::cout << "Your system is ready for VR Binaural Recorder beta testing!\n";
            std::cout << "All core features are functional and ready for evaluation.\n\n";

            // Show recommended quality settings
            QualityPreset recommended = m_capManager->GetRecommendedPreset();
            std::cout << "Recommended Settings:\n";
            std::cout << "  • Quality Level: " << recommended.name << "\n";
            std::cout << "  • Sample Rate: " << recommended.sampleRate << " Hz\n";
            std::cout << "  • HRTF Filter: " << recommended.hrtfFilterLength << " taps\n";
            std::cout << "  • Max Audio Sources: " << recommended.maxAudioSources << "\n";
            std::cout << "  • Tracking Rate: " << recommended.trackingRate << " Hz\n";
        } else {
            std::cout << "Some features may be limited, but simulation mode is available.\n";
            std::cout << "You can still experience spatial audio and provide valuable feedback!\n";
        }

        std::cout << "\nPress Enter to continue...";
        std::cin.get();
        std::cout << "\n";
    }

    void ShowBetaRecommendations() {
        std::cout << "💡 Step 5: Beta Testing Recommendations\n";
        std::cout << "=======================================\n";

        std::string recommendations = m_capManager->GetBetaRecommendations();
        std::cout << recommendations << "\n";

        // Show specific test scenarios
        m_testFramework->LoadBetaTestSuite();
        auto scenarios = m_testFramework->GetAvailableScenarios();

        std::cout << "📋 Recommended Test Scenarios:\n";
        for (size_t i = 0; i < std::min(scenarios.size(), size_t(5)); ++i) {
            std::cout << "  " << (i + 1) << ". " << scenarios[i].name << "\n";
            std::cout << "     " << scenarios[i].description << "\n";
        }

        std::cout << "\nPress Enter to continue...";
        std::cin.get();
        std::cout << "\n";
    }

    void ShowConclusion() {
        std::cout << "🎉 Demo Complete - Welcome to the Beta!\n";
        std::cout << "======================================\n\n";

        std::cout << "What you've just experienced:\n";
        std::cout << "✅ Automatic hardware detection and compatibility assessment\n";
        std::cout << "✅ Intelligent quality adaptation based on your system\n";
        std::cout << "✅ Simulation mode that works without VR hardware\n";
        std::cout << "✅ Real-time spatial audio processing capabilities\n";
        std::cout << "✅ Graceful degradation for optimal performance\n\n";

        std::cout << "Next Steps:\n";
        std::cout << "1. 🎧 Put on your best headphones\n";
        std::cout << "2. 🚀 Launch the full VR Binaural Recorder application\n";
        std::cout << "3. 🎵 Load your favorite audio content\n";
        std::cout << "4. 🎯 Experience spatial audio like never before\n";
        std::cout << "5. 💬 Share your feedback with our development team\n\n";

        if (m_tracker->IsSimulationMode()) {
            std::cout << "💡 Pro Tip: While simulation mode is great for testing,\n";
            std::cout << "consider getting a VR headset for the ultimate experience!\n";
            std::cout << "Check our hardware compatibility guide for recommendations.\n\n";
        } else {
            std::cout << "🥽 You have VR hardware - you're in for an amazing treat!\n";
            std::cout << "The combination of real head tracking and spatial audio\n";
            std::cout << "creates an incredibly immersive experience.\n\n";
        }

        std::cout << "Thank you for participating in the VR Binaural Recorder beta!\n";
        std::cout << "Your feedback helps us create the future of spatial audio.\n\n";

        // Generate a final compatibility report
        auto compatResult = BetaCompatibilityChecker::CheckBetaCompatibility();
        std::string compatReport = BetaCompatibilityChecker::GenerateCompatibilityReport(compatResult);

        std::cout << "📊 Final Compatibility Report:\n";
        std::cout << "==============================\n";
        std::cout << compatReport << "\n";

        std::cout << "🎵 Enjoy the spatial audio revolution!\n";
    }
};

int main() {
    try {
        VRBetaDemo demo;
        demo.RunDemo();
        return 0;
    } catch (const std::exception& e) {
        std::cout << "❌ Demo crashed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "❌ Demo crashed with unknown exception" << std::endl;
        return 1;
    }
}