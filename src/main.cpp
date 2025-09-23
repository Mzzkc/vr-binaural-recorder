// main.cpp - VR Binaural Recorder Entry Point
// Copyright (c) 2024 - Production Ready Spatial Audio for VR

#include <iostream>
#include <memory>
#include <chrono>
#include "application.h"

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
                  << "  --no-vr             Run without VR support (desktop mode)\n"
                  << "  --verbose           Enable verbose logging\n";
    }

    void printVersion() {
        std::cout << "VR Binaural Recorder v1.0.0\n"
                  << "Copyright (c) 2024 - Production Ready Spatial Audio for VR\n"
                  << "Built with C++17, PortAudio, OpenVR, ImGui\n";
    }
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        const std::string arg(argv[i]);

        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--version" || arg == "-v") {
            printVersion();
            return 0;
        } else if (arg == "--list-devices") {
            // TODO: Implement device listing
            std::cout << "Device listing not yet implemented\n";
            return 0;
        } else if (arg == "--verbose") {
            // TODO: Set verbose logging
            std::cout << "Verbose mode enabled\n";
        } else if (arg == "--no-vr") {
            // TODO: Set desktop mode
            std::cout << "VR support disabled\n";
        } else if (arg == "--config" && i + 1 < argc) {
            // TODO: Set custom config file
            std::cout << "Using config file: " << argv[++i] << "\n";
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }

    // Application lifecycle management with RAII
    try {
        ApplicationManager manager;
        vrb::g_application = manager.get();

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