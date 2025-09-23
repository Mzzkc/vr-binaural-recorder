// main.cpp - VR Binaural Recorder Entry Point
// Copyright (c) 2024 - Production Ready Spatial Audio for VR

#include <iostream>
#include "application.h"

int main(int argc, char* argv[]) {
    try {
        // Set up signal handlers
        vrb::setupSignalHandlers();

        // Create global application instance
        vrb::g_application = std::make_unique<vrb::Application>();

        // Initialize application
        auto initResult = vrb::g_application->Initialize();
        if (!initResult.success) {
            std::cerr << "Failed to initialize VR Binaural Recorder: "
                      << initResult.errorMessage << std::endl;

            // Print diagnostic information
            const auto& results = vrb::g_application->getInitResults();
            for (const auto& result : results) {
                if (!result.success) {
                    std::cerr << "  - " << result.component << ": " << result.errorMessage << std::endl;
                }
            }

            return 1;
        }

        // Run main application loop
        vrb::g_application->Run();

        // Clean shutdown
        vrb::g_application->Shutdown();
        vrb::g_application.reset();

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return 1;
    }
}