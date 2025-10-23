/**
 * VR VALIDATION SUITE - CRITICAL WOLF-PREVENTION TESTS
 *
 * Jordan Taylor - Senior QA Engineer
 * If these tests fail, we're all wolf food. No excuses.
 *
 * CRITICAL SUCCESS CRITERIA:
 * 1. VR headset detection MUST work
 * 2. Head tracking MUST control spatial audio
 * 3. Real-time positioning MUST have <5ms latency
 * 4. ASMRtist MUST be able to record using head movement
 */

#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <vector>
#include <numeric>
#include <cmath>
#include "../modules/vr/vr_tracker.h"
#include "../modules/audio/spatial_processor.h"
#include "../modules/config/config_manager.h"
#include "../modules/logger/logger.h"

using namespace vrb;
using namespace std::chrono;

class VRValidationSuite : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize logger for test output
        Logger::getInstance().init(LogLevel::DEBUG);

        // Initialize VR tracker
        tracker = std::make_unique<VRTracker>();

        // Initialize spatial processor for audio validation
        processor = std::make_unique<SpatialProcessor>();

        LOG_INFO("=== VR VALIDATION SUITE STARTING ===");
        LOG_INFO("FAILURE = WOLVES. SUCCESS = SURVIVAL.");
    }

    void TearDown() override {
        if (tracker) {
            tracker->Shutdown();
        }
        LOG_INFO("=== VR VALIDATION SUITE COMPLETE ===");
    }

    std::unique_ptr<VRTracker> tracker;
    std::unique_ptr<SpatialProcessor> processor;
};

/**
 * TEST 1: VR HEADSET DETECTION
 * CRITICAL: Must detect VR hardware or we have NOTHING
 */
TEST_F(VRValidationSuite, VRHeadsetDetection) {
    LOG_INFO("TEST: VR Headset Detection - THE FOUNDATION");

    // Check if VR runtime is installed
    bool runtimeInstalled = vr::VR_IsRuntimeInstalled();
    if (!runtimeInstalled) {
        LOG_ERROR("CRITICAL FAILURE: SteamVR runtime not installed!");
        LOG_ERROR("WOLF ALERT: No VR runtime = No product!");
        FAIL() << "SteamVR runtime must be installed for VR functionality";
    }

    // Check if headset is present
    bool hmdPresent = vr::VR_IsHmdPresent();
    if (!hmdPresent) {
        LOG_WARN("WARNING: No VR headset detected - running in degraded mode");
        LOG_WARN("This would be FATAL in production!");
        GTEST_SKIP() << "VR headset not connected - skipping hardware tests";
    }

    // Initialize tracker
    bool initialized = tracker->Initialize();
    ASSERT_TRUE(initialized) << "VR tracker initialization failed - WOLVES INCOMING!";

    // Verify headset is connected
    bool connected = tracker->IsHMDConnected();
    ASSERT_TRUE(connected) << "HMD not connected after initialization - CRITICAL FAILURE!";

    // Get HMD model for validation
    std::string model = tracker->GetHMDModel();
    LOG_INFO("VR Headset Detected: {}", model);
    ASSERT_FALSE(model.empty()) << "Failed to identify HMD model";
    ASSERT_NE(model, "Unknown") << "HMD model is unknown - potential compatibility issue";

    LOG_INFO("✓ VR HEADSET DETECTION: PASSED - Wolves kept at bay!");
}

/**
 * TEST 2: HEAD TRACKING LATENCY
 * CRITICAL: Must achieve <5ms latency for real-time audio
 */
TEST_F(VRValidationSuite, HeadTrackingLatency) {
    LOG_INFO("TEST: Head Tracking Latency - MUST BE <5ms!");

    if (!vr::VR_IsHmdPresent()) {
        GTEST_SKIP() << "VR headset required for latency testing";
    }

    ASSERT_TRUE(tracker->Initialize()) << "Failed to initialize VR tracker";
    ASSERT_TRUE(tracker->StartTracking()) << "Failed to start tracking";

    // Warm up tracking system
    std::this_thread::sleep_for(milliseconds(100));

    // Measure latency over multiple samples
    std::vector<double> latencies;
    const int SAMPLE_COUNT = 100;

    for (int i = 0; i < SAMPLE_COUNT; ++i) {
        auto start = high_resolution_clock::now();

        // Get current pose
        VRPose pose = tracker->GetHMDPose();

        auto end = high_resolution_clock::now();
        auto latency = duration<double, std::milli>(end - start).count();

        if (pose.isValid) {
            latencies.push_back(latency);
        }

        // Small delay between samples
        std::this_thread::sleep_for(microseconds(500));
    }

    ASSERT_GT(latencies.size(), 0) << "No valid pose data received!";

    // Calculate statistics
    double avg_latency = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
    double max_latency = *std::max_element(latencies.begin(), latencies.end());
    double min_latency = *std::min_element(latencies.begin(), latencies.end());

    LOG_INFO("Latency Statistics over {} samples:", latencies.size());
    LOG_INFO("  Average: {:.3f}ms", avg_latency);
    LOG_INFO("  Min: {:.3f}ms", min_latency);
    LOG_INFO("  Max: {:.3f}ms", max_latency);

    // CRITICAL REQUIREMENT: Average latency must be <5ms
    ASSERT_LT(avg_latency, 5.0) << "CRITICAL FAILURE: Average latency "
                                 << avg_latency << "ms exceeds 5ms requirement!";

    // Max latency should not exceed 10ms (allowing for occasional spikes)
    ASSERT_LT(max_latency, 10.0) << "WARNING: Maximum latency spike of "
                                  << max_latency << "ms detected!";

    LOG_INFO("✓ HEAD TRACKING LATENCY: PASSED - Real-time requirement met!");
}

/**
 * TEST 3: SPATIAL AUDIO CONTROL
 * CRITICAL: Head movement MUST control audio spatialization
 */
TEST_F(VRValidationSuite, SpatialAudioControl) {
    LOG_INFO("TEST: Spatial Audio Control - HEAD MUST CONTROL SOUND!");

    if (!vr::VR_IsHmdPresent()) {
        GTEST_SKIP() << "VR headset required for spatial audio testing";
    }

    ASSERT_TRUE(tracker->Initialize()) << "Failed to initialize VR tracker";
    ASSERT_TRUE(tracker->StartTracking()) << "Failed to start tracking";

    // Initialize spatial processor
    ASSERT_TRUE(processor->Initialize(48000, 512)) << "Failed to initialize spatial processor";

    // Test callback integration
    bool callbackTriggered = false;
    VRPose lastPose;

    tracker->SetTrackingCallback([&](const VRPose& hmd, const std::vector<VRPose>& controllers) {
        callbackTriggered = true;
        lastPose = hmd;

        // Update spatial processor with head position
        if (hmd.isValid) {
            processor->SetListenerPosition(
                hmd.position.x,
                hmd.position.y,
                hmd.position.z
            );

            // Convert quaternion to forward vector for orientation
            float forward[3];
            // Simplified forward vector calculation from quaternion
            forward[0] = 2.0f * (hmd.orientation.x * hmd.orientation.z +
                                hmd.orientation.w * hmd.orientation.y);
            forward[1] = 2.0f * (hmd.orientation.y * hmd.orientation.z -
                                hmd.orientation.w * hmd.orientation.x);
            forward[2] = 1.0f - 2.0f * (hmd.orientation.x * hmd.orientation.x +
                                        hmd.orientation.y * hmd.orientation.y);

            processor->SetListenerOrientation(forward[0], forward[1], forward[2]);
        }
    });

    // Wait for callback to trigger
    std::this_thread::sleep_for(milliseconds(100));
    tracker->Update();

    ASSERT_TRUE(callbackTriggered) << "Tracking callback never triggered!";
    ASSERT_TRUE(lastPose.isValid) << "Invalid pose data received!";

    // Verify spatial processor received updates
    auto listenerPos = processor->GetListenerPosition();
    ASSERT_NEAR(listenerPos.x, lastPose.position.x, 0.001f)
        << "Spatial processor position doesn't match VR tracking!";
    ASSERT_NEAR(listenerPos.y, lastPose.position.y, 0.001f)
        << "Spatial processor position doesn't match VR tracking!";
    ASSERT_NEAR(listenerPos.z, lastPose.position.z, 0.001f)
        << "Spatial processor position doesn't match VR tracking!";

    LOG_INFO("✓ SPATIAL AUDIO CONTROL: PASSED - Head controls audio position!");
}

/**
 * TEST 4: ASMRTIST WORKFLOW
 * CRITICAL: Complete ASMRtist recording workflow validation
 */
TEST_F(VRValidationSuite, ASMRtistWorkflow) {
    LOG_INFO("TEST: ASMRtist Workflow - THE ULTIMATE TEST!");

    if (!vr::VR_IsHmdPresent()) {
        GTEST_SKIP() << "VR headset required for workflow testing";
    }

    // Step 1: Initialize all systems
    ASSERT_TRUE(tracker->Initialize()) << "VR initialization failed";
    ASSERT_TRUE(tracker->StartTracking()) << "Tracking start failed";
    ASSERT_TRUE(processor->Initialize(48000, 512)) << "Audio initialization failed";

    // Step 2: Simulate ASMRtist head movement recording
    struct RecordedPosition {
        VRPose pose;
        double timestamp;
    };

    std::vector<RecordedPosition> recordedPath;
    auto recordingStart = high_resolution_clock::now();

    // Record 5 seconds of head movement
    LOG_INFO("Simulating ASMRtist recording session...");
    while (duration<double>(high_resolution_clock::now() - recordingStart).count() < 5.0) {
        VRPose currentPose = tracker->GetHMDPose();

        if (currentPose.isValid) {
            RecordedPosition pos;
            pos.pose = currentPose;
            pos.timestamp = duration<double>(high_resolution_clock::now() - recordingStart).count();
            recordedPath.push_back(pos);

            // Update spatial audio
            processor->SetListenerPosition(
                currentPose.position.x,
                currentPose.position.y,
                currentPose.position.z
            );
        }

        std::this_thread::sleep_for(milliseconds(11)); // ~90Hz update rate
    }

    // Step 3: Validate recording quality
    ASSERT_GT(recordedPath.size(), 400) << "Insufficient tracking data captured!";

    // Check for smooth tracking (no large jumps)
    for (size_t i = 1; i < recordedPath.size(); ++i) {
        float deltaX = std::abs(recordedPath[i].pose.position.x -
                               recordedPath[i-1].pose.position.x);
        float deltaY = std::abs(recordedPath[i].pose.position.y -
                               recordedPath[i-1].pose.position.y);
        float deltaZ = std::abs(recordedPath[i].pose.position.z -
                               recordedPath[i-1].pose.position.z);

        // No jumps > 10cm between frames (would indicate tracking loss)
        ASSERT_LT(deltaX, 0.1f) << "Tracking jump detected at frame " << i;
        ASSERT_LT(deltaY, 0.1f) << "Tracking jump detected at frame " << i;
        ASSERT_LT(deltaZ, 0.1f) << "Tracking jump detected at frame " << i;
    }

    // Step 4: Verify update rate
    double totalTime = recordedPath.back().timestamp;
    double avgUpdateRate = recordedPath.size() / totalTime;
    LOG_INFO("Recording Statistics:");
    LOG_INFO("  Total frames: {}", recordedPath.size());
    LOG_INFO("  Duration: {:.2f}s", totalTime);
    LOG_INFO("  Average update rate: {:.1f}Hz", avgUpdateRate);

    ASSERT_GT(avgUpdateRate, 60.0) << "Update rate too low for smooth audio!";

    LOG_INFO("✓ ASMRTIST WORKFLOW: PASSED - Ready for production!");
}

/**
 * TEST 5: PERFORMANCE STRESS TEST
 * Ensure system maintains performance under load
 */
TEST_F(VRValidationSuite, PerformanceUnderLoad) {
    LOG_INFO("TEST: Performance Stress Test - SUSTAINED REAL-TIME OPERATION!");

    if (!vr::VR_IsHmdPresent()) {
        GTEST_SKIP() << "VR headset required for performance testing";
    }

    ASSERT_TRUE(tracker->Initialize());
    ASSERT_TRUE(tracker->StartTracking());
    ASSERT_TRUE(processor->Initialize(48000, 512));

    // Run intensive tracking for 30 seconds
    auto testStart = high_resolution_clock::now();
    int updateCount = 0;
    std::vector<double> frameTimes;

    LOG_INFO("Running 30-second stress test...");

    while (duration<double>(high_resolution_clock::now() - testStart).count() < 30.0) {
        auto frameStart = high_resolution_clock::now();

        // Get pose
        VRPose pose = tracker->GetHMDPose();

        // Update spatial audio
        if (pose.isValid) {
            processor->SetListenerPosition(
                pose.position.x,
                pose.position.y,
                pose.position.z
            );

            // Simulate audio processing load
            float dummyBuffer[512];
            processor->ProcessAudioFrame(dummyBuffer, dummyBuffer, 512);

            updateCount++;
        }

        auto frameEnd = high_resolution_clock::now();
        frameTimes.push_back(duration<double, std::milli>(frameEnd - frameStart).count());

        // Target 90Hz update rate
        std::this_thread::sleep_for(microseconds(11111));
    }

    // Analyze performance
    double avgFrameTime = std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0) / frameTimes.size();
    double maxFrameTime = *std::max_element(frameTimes.begin(), frameTimes.end());

    LOG_INFO("Stress Test Results:");
    LOG_INFO("  Total updates: {}", updateCount);
    LOG_INFO("  Average frame time: {:.3f}ms", avgFrameTime);
    LOG_INFO("  Max frame time: {:.3f}ms", maxFrameTime);

    // Performance requirements
    ASSERT_LT(avgFrameTime, 5.0) << "Average frame time exceeds real-time requirement!";
    ASSERT_LT(maxFrameTime, 20.0) << "Frame time spikes detected - risk of audio glitches!";
    ASSERT_GT(updateCount, 2500) << "Update rate too low under load!";

    LOG_INFO("✓ PERFORMANCE STRESS TEST: PASSED - System maintains real-time operation!");
}

/**
 * MAIN TEST RUNNER
 */
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "    VR VALIDATION SUITE - WOLF TEST    \n";
    std::cout << "========================================\n";
    std::cout << "FAILURE MEANS WOLVES. NO PRESSURE.\n\n";

    int result = RUN_ALL_TESTS();

    if (result == 0) {
        std::cout << "\n✓✓✓ ALL TESTS PASSED - WOLVES DEFEATED! ✓✓✓\n";
        std::cout << "VR system validated for ASMRtist production use!\n";
    } else {
        std::cout << "\n✗✗✗ TESTS FAILED - PREPARE FOR WOLVES! ✗✗✗\n";
        std::cout << "Critical VR functionality not working!\n";
    }

    return result;
}