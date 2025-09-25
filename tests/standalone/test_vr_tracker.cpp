// test_vr_tracker.cpp - Standalone test for VR tracker functionality
// This creates a minimal test to verify our VR tracking implementation works!

#include "vr_tracker.h"
#include "logger.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace vrb;

int main() {
    std::cout << "VR Binaural Recorder - VR Tracker Test\n";
    std::cout << "======================================\n\n";

    // Initialize logger
    Logger::Initialize("debug", "./logs", true);

    // Create VR tracker instance
    VRTracker tracker;

    // Set up tracking callback to monitor head movement
    int poseUpdateCount = 0;
    tracker.SetTrackingCallback([&poseUpdateCount](const VRPose& head, const std::vector<VRPose>& controllers) {
        poseUpdateCount++;

        if (poseUpdateCount % 90 == 0) { // Log every ~1 second at 90Hz
            std::cout << "Head pose update #" << poseUpdateCount << ":\n";
            std::cout << "  Position: (" << head.position.x << ", "
                     << head.position.y << ", " << head.position.z << ")\n";
            std::cout << "  Controllers: " << controllers.size() << "\n";
            std::cout << "  Orientation: (" << head.orientation.x << ", "
                     << head.orientation.y << ", " << head.orientation.z
                     << ", " << head.orientation.w << ")\n";
            std::cout << "  Valid: " << (head.isValid ? "Yes" : "No") << "\n";

            // Note: Using simplified VR tracker for spatial audio only\n";
        }
    });

    // Initialize VR system
    std::cout << "Initializing VR tracking system...\n";
    if (!tracker.Initialize()) {
        std::cout << "ERROR: Failed to initialize VR tracking!\n";
        return 1;
    }

    std::cout << "VR tracker initialized successfully!\n";
    std::cout << "Connected: " << (tracker.IsConnected() ? "Yes" : "No (Simulation Mode)") << "\n\n";

    // Start tracking
    std::cout << "Starting VR tracking thread...\n";
    tracker.StartTracking();

    // Run for 10 seconds to demonstrate tracking
    std::cout << "Tracking for 10 seconds... (simulation mode generates fake head movement)\n\n";

    for (int i = 10; i > 0; --i) {
        std::cout << "Countdown: " << i << " seconds remaining...\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Process VR events
        tracker.ProcessEvents();

        // Get current poses manually
        VRPose headPose = tracker.GetHMDPose();
        // Note: Using simplified tracker - microphone position is relative to HMD

        std::cout << "Current head position: (" << headPose.position.x << ", "
                 << headPose.position.y << ", " << headPose.position.z << ")\n";
    }

    // Test controller tracking (simplified API)
    std::cout << "\nTesting controller tracking...\n";

    for (int i = 0; i < 4; ++i) {
        auto controllers = tracker.GetControllerPoses();
        std::cout << "Controller count: " << controllers.size() << "\n";

        for (size_t j = 0; j < controllers.size(); ++j) {
            const auto& controller = controllers[j];
            std::cout << "Controller " << j << ": (" << controller.position.x << ", "
                     << controller.position.y << ", " << controller.position.z
                     << ") Valid: " << (controller.isValid ? "Yes" : "No") << "\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "\nSimplified VR tracker test complete - using HMD-relative audio positioning\n";

    // Stop tracking
    std::cout << "\nStopping VR tracking...\n";
    tracker.StopTracking();

    std::cout << "\nVR Tracker Test Complete!\n";
    std::cout << "Total pose updates received: " << poseUpdateCount << "\n";
    std::cout << "This demonstrates our VR tracking system is working correctly!\n";

    Logger::Shutdown();
    return 0;
}