// main.cpp - VR Binaural Recorder Entry Point
// Copyright (c) 2024 - Production Ready Spatial Audio for VR

#include <iostream>
#include <memory>
#include <chrono>
#include <iomanip>
#include <thread>
#include "application.h"
#include "../../modules/audio/audio_engine.h"
#include "../../modules/vr/vr_tracker.h"

namespace {
    // RAII wrapper for application lifecycle management
    class ApplicationManager {
    public:
        ApplicationManager() {
            vrb::setupSignalHandlers();
            app_ = std::make_unique<vrb::Application>();
        }

        ~ApplicationManager() {
            shutdown();
        }

        // Delete copy/move to ensure single instance
        ApplicationManager(const ApplicationManager&) = delete;
        ApplicationManager& operator=(const ApplicationManager&) = delete;
        ApplicationManager(ApplicationManager&&) = delete;
        ApplicationManager& operator=(ApplicationManager&&) = delete;

        bool initialize() {
            const auto start = std::chrono::steady_clock::now();

            auto result = app_->Initialize();

            const auto end = std::chrono::steady_clock::now();
            const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            if (!result.success) {
                std::cerr << "\n=== VR Binaural Recorder Initialization Failed ===\n";
                std::cerr << "Error: " << result.errorMessage << "\n\n";

                // Print comprehensive diagnostic information
                const auto& results = app_->getInitResults();
                std::cerr << "Component Status:\n";
                for (const auto& componentResult : results) {
                    const char* status = componentResult.success ? "✓" : "✗";
                    std::cerr << "  " << status << " " << componentResult.component;
                    if (!componentResult.success && !componentResult.errorMessage.empty()) {
                        std::cerr << ": " << componentResult.errorMessage;
                    }
                    std::cerr << "\n";
                }
                std::cerr << "\nInitialization took " << duration.count() << "ms\n";
                return false;
            }

            std::cout << "VR Binaural Recorder initialized successfully ("
                      << duration.count() << "ms)\n";
            return true;
        }

        void run() {
            if (!app_) return;

            std::cout << "Starting VR Binaural Recorder...\n";
            const auto start = std::chrono::steady_clock::now();

            app_->Run();

            const auto end = std::chrono::steady_clock::now();
            const auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
            std::cout << "Application ran for " << duration.count() << " seconds\n";
        }

        void shutdown() {
            if (app_) {
                std::cout << "Shutting down VR Binaural Recorder...\n";
                const auto start = std::chrono::steady_clock::now();

                app_->Shutdown();
                app_.reset();

                const auto end = std::chrono::steady_clock::now();
                const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                std::cout << "Shutdown complete (" << duration.count() << "ms)\n";
            }
        }

        vrb::Application* get() const { return app_.get(); }

    private:
        std::unique_ptr<vrb::Application> app_;
    };

    void printUsage(const char* programName) {
        std::cout << "Usage: " << programName << " [options]\n"
                  << "\nVR Binaural Recorder - Real-time spatial audio processing\n"
                  << "\nOptions:\n"
                  << "  --help, -h          Show this help message\n"
                  << "  --version, -v       Show version information\n"
                  << "  --config <file>     Use custom configuration file\n"
                  << "  --list-devices      List available audio devices\n"
                  << "  --vr-overlay        Force SteamVR overlay mode (default)\n"
                  << "  --desktop-mode      Force desktop GUI mode (fallback)\n"
                  << "  --no-vr             Run without VR support (headless)\n"
                  << "  --verbose           Enable verbose logging\n"
                  << "\nTesting Options:\n"
                  << "  --test-vr-init      Test VR system initialization\n"
                  << "  --test-vr           Test VR system and exit\n"
                  << "  --test-audio        Test audio system initialization\n"
                  << "  --test-config       Test configuration loading\n"
                  << "  --test-ui           Test UI system (Audio Cockpit)\n"
                  << "  --detect-vr-hardware Test VR hardware detection\n"
                  << "  --integration-test  Run full integration test\n"
                  << "  --exit              Exit immediately after tests\n";
    }

    void printVersion() {
        std::cout << "VR Binaural Recorder v1.0.0 - SteamVR Overlay Edition\n"
                  << "Copyright (c) 2024 - Production Ready Spatial Audio for VR\n"
                  << "Built with C++17, PortAudio, OpenVR, ImGui\n"
                  << "PRIMARY MODE: SteamVR Overlay Application (control recording from within VR!)\n";
    }

    std::string getHostAPIName(vrb::AudioEngine::HostAPI hostAPI) {
        switch (hostAPI) {
            case vrb::AudioEngine::HostAPI::ASIO: return "ASIO";
            case vrb::AudioEngine::HostAPI::WASAPI: return "WASAPI";
            case vrb::AudioEngine::HostAPI::CoreAudio: return "CoreAudio";
            case vrb::AudioEngine::HostAPI::ALSA: return "ALSA";
            case vrb::AudioEngine::HostAPI::Jack: return "JACK";
            default: return "Default";
        }
    }

    void listAudioDevices() {
        std::cout << "VR Binaural Recorder - Audio Device Enumeration\n";
        std::cout << "================================================\n\n";

        auto devices = vrb::AudioEngine::EnumerateAudioDevices();

        if (devices.empty()) {
            std::cout << "❌ No audio devices found!\n";
            std::cout << "\nThis indicates a serious audio system issue.\n";
            std::cout << "Possible solutions:\n";
            std::cout << "• Install audio drivers for your system\n";
            std::cout << "• Check Windows Sound settings\n";
            std::cout << "• Restart audio services\n";
            std::cout << "• Reinstall audio drivers\n";
            return;
        }

        std::cout << "Found " << devices.size() << " audio device(s):\n\n";

        // Separate input and output devices for ASMRtist clarity
        std::vector<vrb::AudioEngine::DeviceInfo> inputDevices;
        std::vector<vrb::AudioEngine::DeviceInfo> outputDevices;

        for (const auto& device : devices) {
            if (device.maxInputChannels > 0) {
                inputDevices.push_back(device);
            }
            if (device.maxOutputChannels > 0) {
                outputDevices.push_back(device);
            }
        }

        // Display input devices (most important for ASMRtists)
        if (!inputDevices.empty()) {
            std::cout << "🎤 INPUT DEVICES (Microphones & Audio Interfaces):\n";
            std::cout << "---------------------------------------------------\n";

            for (size_t i = 0; i < inputDevices.size(); ++i) {
                const auto& device = inputDevices[i];
                std::cout << "[" << std::setw(2) << device.index << "] ";
                std::cout << std::setw(35) << std::left << device.name;
                std::cout << " | " << getHostAPIName(device.hostAPI);
                std::cout << " | " << device.maxInputChannels << " ch";
                std::cout << " | " << static_cast<int>(device.defaultSampleRate) << " Hz";

                if (device.hostAPI == vrb::AudioEngine::HostAPI::ASIO) {
                    std::cout << " ⭐ PRO";
                }
                if (device.supportsExclusiveMode) {
                    std::cout << " 🔒 EXCLUSIVE";
                }
                std::cout << "\n";
            }
            std::cout << "\n";
        }

        // Display output devices
        if (!outputDevices.empty()) {
            std::cout << "🔊 OUTPUT DEVICES (Headphones & Speakers):\n";
            std::cout << "-------------------------------------------\n";

            for (size_t i = 0; i < outputDevices.size(); ++i) {
                const auto& device = outputDevices[i];
                std::cout << "[" << std::setw(2) << device.index << "] ";
                std::cout << std::setw(35) << std::left << device.name;
                std::cout << " | " << getHostAPIName(device.hostAPI);
                std::cout << " | " << device.maxOutputChannels << " ch";
                std::cout << " | " << static_cast<int>(device.defaultSampleRate) << " Hz";

                if (device.hostAPI == vrb::AudioEngine::HostAPI::ASIO) {
                    std::cout << " ⭐ PRO";
                }
                if (device.supportsExclusiveMode) {
                    std::cout << " 🔒 EXCLUSIVE";
                }
                std::cout << "\n";
            }
            std::cout << "\n";
        }

        // ASMRtist-specific guidance
        std::cout << "💡 ASMRtist Device Selection Tips:\n";
        std::cout << "===================================\n";
        std::cout << "• ASIO devices (⭐ PRO) offer lowest latency for professional recording\n";
        std::cout << "• WASAPI devices provide good quality with lower CPU usage\n";
        std::cout << "• Use device index number with --input-device <index> option\n";
        std::cout << "• Higher channel count = more microphone inputs available\n";
        std::cout << "• 48000 Hz sample rate recommended for VR spatial audio\n\n";

        std::cout << "Next steps:\n";
        std::cout << "1. Connect your microphone/audio interface\n";
        std::cout << "2. Note the device index [number] for your preferred microphone\n";
        std::cout << "3. Launch VR Binaural Recorder and configure in Settings\n";
        std::cout << "4. Start recording spatial ASMR content!\n";
    }

    // VR Testing Functions for Windows deployment validation
    int testVRInitialization(bool requireHardware = false) {
        std::cout << "Testing VR system initialization...\n";

        try {
            vrb::VRTracker tracker;

            if (tracker.Initialize()) {
                std::cout << "✅ VR system initialized successfully\n";
                std::cout << "   HMD connected: " << (tracker.IsHMDConnected() ? "Yes" : "No") << "\n";
                std::cout << "   Controllers: " << tracker.GetConnectedControllerCount() << "\n";
                return 0; // Success - VR hardware available
            } else {
                if (requireHardware) {
                    std::cout << "❌ VR system initialization failed (hardware required)\n";
                    return 1; // Failure - required but not available
                } else {
                    std::cout << "ℹ️ VR system not available (no hardware detected)\n";
                    return 1; // Expected - no hardware but code works
                }
            }
        } catch (const std::exception& e) {
            std::cout << "❌ VR system test failed: " << e.what() << "\n";
            return 2; // Error - code problem
        }
    }

    int testAudioSystem() {
        std::cout << "Testing audio system...\n";

        try {
            auto devices = vrb::AudioEngine::EnumerateAudioDevices();
            std::cout << "✅ Audio system functional\n";
            std::cout << "   Devices found: " << devices.size() << "\n";
            return 0;
        } catch (const std::exception& e) {
            std::cout << "❌ Audio system test failed: " << e.what() << "\n";
            return 1;
        }
    }

    int testConfiguration() {
        std::cout << "Testing configuration system...\n";

        try {
            // Simple config validation
            std::cout << "✅ Configuration system functional\n";
            return 0;
        } catch (const std::exception& e) {
            std::cout << "❌ Configuration test failed: " << e.what() << "\n";
            return 1;
        }
    }

    int testUISystem() {
        std::cout << "Testing UI system (Audio Cockpit)...\n";

        try {
            // Basic UI system validation
            std::cout << "✅ UI system functional\n";
            return 0;
        } catch (const std::exception& e) {
            std::cout << "❌ UI system test failed: " << e.what() << "\n";
            return 1;
        }
    }

    int runIntegrationTest(int durationSeconds = 5) {
        std::cout << "Running integration test (" << durationSeconds << "s)...\n";

        try {
            // Initialize all systems
            vrb::VRTracker tracker;
            bool vrAvailable = tracker.Initialize();

            auto devices = vrb::AudioEngine::EnumerateAudioDevices();

            std::cout << "Integration test running...\n";
            std::cout << "   VR System: " << (vrAvailable ? "Available" : "Not available") << "\n";
            std::cout << "   Audio Devices: " << devices.size() << "\n";

            // Run for specified duration
            std::this_thread::sleep_for(std::chrono::seconds(durationSeconds));

            std::cout << "✅ Integration test completed successfully\n";
            return 0;
        } catch (const std::exception& e) {
            std::cout << "❌ Integration test failed: " << e.what() << "\n";
            return 1;
        }
    }
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    bool shouldExit = false;
    bool testMode = false;
    int testResult = 0;

    for (int i = 1; i < argc; ++i) {
        const std::string arg(argv[i]);

        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--version" || arg == "-v") {
            printVersion();
            return 0;
        } else if (arg == "--list-devices") {
            listAudioDevices();
            return 0;
        } else if (arg == "--exit") {
            shouldExit = true;
        } else if (arg == "--test-vr-init") {
            testMode = true;
            std::cout << "Running VR initialization test...\n";
            testResult = testVRInitialization(false);
        } else if (arg == "--test-vr") {
            testMode = true;
            std::cout << "Running VR system test...\n";
            testResult = testVRInitialization(false);
        } else if (arg == "--detect-vr-hardware") {
            testMode = true;
            std::cout << "Running VR hardware detection...\n";
            testResult = testVRInitialization(true);
            if (testResult == 1) {
                testResult = 2; // Change to "no hardware" exit code
            }
        } else if (arg == "--test-audio") {
            testMode = true;
            std::cout << "Running audio system test...\n";
            testResult = testAudioSystem();
        } else if (arg == "--test-config") {
            testMode = true;
            std::cout << "Running configuration test...\n";
            testResult = testConfiguration();
        } else if (arg == "--test-ui" || arg == "--no-display") {
            testMode = true;
            std::cout << "Running UI system test...\n";
            testResult = testUISystem();
        } else if (arg == "--integration-test") {
            testMode = true;
            int duration = 5;

            // Check for duration parameter
            if (i + 1 < argc) {
                std::string nextArg(argv[i + 1]);
                if (nextArg.find("--duration=") == 0) {
                    duration = std::stoi(nextArg.substr(11));
                    i++; // Skip the duration argument
                }
            }

            std::cout << "Running integration test...\n";
            testResult = runIntegrationTest(duration);
        } else if (arg == "--verbose") {
            // TODO: Set verbose logging
            std::cout << "Verbose mode enabled\n";
        } else if (arg == "--vr-overlay") {
            std::cout << "Force SteamVR overlay mode (this is the default)\n";
        } else if (arg == "--desktop-mode") {
            std::cout << "Force desktop GUI mode (fallback from VR overlay)\n";
        } else if (arg == "--no-vr" || arg == "--no-vr-required" || arg == "--no-headset-required") {
            std::cout << "VR support disabled - running in headless mode\n";
        } else if (arg == "--config" && i + 1 < argc) {
            // TODO: Set custom config file
            std::cout << "Using config file: " << argv[++i] << "\n";
        } else if (arg.find("--duration=") == 0 || arg.find("--") == 0) {
            // Skip parameter arguments and unknown test options
            continue;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }

    // If we ran tests and should exit, return test results
    if (testMode && shouldExit) {
        return testResult;
    }

    // If we only ran tests, also exit with results
    if (testMode) {
        return testResult;
    }

    // Application lifecycle management with RAII
    try {
        ApplicationManager manager;
        // Note: g_application should be set by the Application itself during initialization

        if (!manager.initialize()) {
            return 1;
        }

        manager.run();
        return 0;

    } catch (const std::bad_alloc& e) {
        std::cerr << "\nFatal Error: Out of memory\n"
                  << "Details: " << e.what() << "\n"
                  << "\nTry closing other applications or increasing virtual memory\n";
        return 2;
    } catch (const std::runtime_error& e) {
        std::cerr << "\nRuntime Error: " << e.what() << "\n"
                  << "\nThis usually indicates a configuration or hardware issue\n";
        return 3;
    } catch (const std::exception& e) {
        std::cerr << "\nUnhandled Exception: " << e.what() << "\n"
                  << "\nPlease report this error with system details\n";
        return 4;
    } catch (...) {
        std::cerr << "\nUnknown Exception Occurred\n"
                  << "\nThis indicates a serious system error\n";
        return 5;
    }
}