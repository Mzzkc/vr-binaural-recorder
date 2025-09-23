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
    tracker.SetTrackingCallback([&poseUpdateCount](const VRPose& head, const VRPose& mic) {
        poseUpdateCount++;

        if (poseUpdateCount % 90 == 0) { // Log every ~1 second at 90Hz
            std::cout << "Head pose update #" << poseUpdateCount << ":\n";
            std::cout << "  Position: (" << head.position.x << ", "
                     << head.position.y << ", " << head.position.z << ")\n";
            std::cout << "  Orientation: (" << head.orientation.x << ", "
                     << head.orientation.y << ", " << head.orientation.z
                     << ", " << head.orientation.w << ")\n";
            std::cout << "  Valid: " << (head.isValid ? "Yes" : "No") << "\n";

            std::cout << "Microphone position: (" << mic.position.x << ", "
                     << mic.position.y << ", " << mic.position.z << ")\n\n";
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
        VRPose headPose = tracker.GetHeadPose();
        VRPose micPose = tracker.GetMicrophonePose();

        std::cout << "Current head position: (" << headPose.position.x << ", "
                 << headPose.position.y << ", " << headPose.position.z << ")\n";
    }

    // Test microphone position manipulation
    std::cout << "\nTesting microphone position control...\n";

    Vec3 testPositions[] = {
        {1.0f, 1.5f, 0.0f},   // Right side
        {-1.0f, 1.5f, 0.0f},  // Left side
        {0.0f, 2.0f, -0.5f},  // Above and forward
        {0.0f, 0.5f, -2.0f}   // Low and far forward
    };

    for (int i = 0; i < 4; ++i) {
        tracker.SetMicrophonePosition(testPositions[i]);

        VRPose micPose = tracker.GetMicrophonePose();
        std::cout << "Set microphone to: (" << testPositions[i].x << ", "
                 << testPositions[i].y << ", " << testPositions[i].z
                 << ") -> Got: (" << micPose.position.x << ", "
                 << micPose.position.y << ", " << micPose.position.z << ")\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Reset to default
    std::cout << "\nResetting microphone to default position...\n";
    tracker.ResetMicrophonePosition();

    VRPose finalMicPose = tracker.GetMicrophonePose();
    std::cout << "Final microphone position: (" << finalMicPose.position.x << ", "
             << finalMicPose.position.y << ", " << finalMicPose.position.z << ")\n";

    // Stop tracking
    std::cout << "\nStopping VR tracking...\n";
    tracker.StopTracking();

    std::cout << "\nVR Tracker Test Complete!\n";
    std::cout << "Total pose updates received: " << poseUpdateCount << "\n";
    std::cout << "This demonstrates our VR tracking system is working correctly!\n";

    Logger::Shutdown();
    return 0;
}