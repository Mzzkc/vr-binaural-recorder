// audio_cockpit_integration.cpp - Complete VRTracker + Audio Cockpit Integration Example
// This demonstrates how to wire the Creative Coder's Audio Cockpit to 90Hz VR tracking

#include "vr_tracker.h"
#include "audio_engine.h"
#include "hrtf_processor.h"
#include "logger.h"
#include <memory>
#include <chrono>

// Mock Audio Cockpit classes for integration demonstration
// These show exactly how the Creative Coder's real implementation will connect

namespace vrb {

/**
 * Example integration showing how Creative Coder's Audio Cockpit connects to VRTracker
 * This demonstrates the exact callback pattern and threading approach
 */
class AudioCockpitIntegrationExample {
public:
    AudioCockpitIntegrationExample(
        std::shared_ptr<VRTracker> vrTracker,
        std::shared_ptr<AudioEngine> audioEngine,
        std::shared_ptr<HRTFProcessor> hrtfProcessor
    ) :
        m_vrTracker(vrTracker),
        m_audioEngine(audioEngine),
        m_hrtfProcessor(hrtfProcessor),
        m_gestureDetectionActive(true)
    {
        LOG_INFO("Audio Cockpit integration example initialized");
    }

    bool InitializeAudioCockpitIntegration() {
        if (!m_vrTracker || !m_audioEngine || !m_hrtfProcessor) {
            LOG_ERROR("Missing required components for Audio Cockpit integration");
            return false;
        }

        // Wire VRTracker callbacks to Audio Cockpit gesture system
        m_vrTracker->SetTrackingCallback([this](const VRPose& hmd, const std::vector<VRPose>& controllers) {
            // This callback runs at 90Hz on the VR tracking thread
            OnVRTrackingUpdate(hmd, controllers);
        });

        LOG_INFO("Audio Cockpit integration complete - gesture detection active at 90Hz");
        return true;
    }

    void EnableGestureDetection(bool enable) {
        m_gestureDetectionActive = enable;
        LOG_INFO("Audio Cockpit gesture detection: {}", enable ? "ENABLED" : "DISABLED");
    }

private:
    void OnVRTrackingUpdate(const VRPose& hmd, const std::vector<VRPose>& controllers) {
        if (!m_gestureDetectionActive) {
            return;
        }

        // Update HMD position for audio orb positioning
        UpdateAudioOrbPositioning(hmd);

        // Process controller gestures at full 90Hz
        for (size_t i = 0; i < controllers.size(); ++i) {
            ProcessControllerGestures(controllers[i], i);
        }

        // Feed data to lock-free ring buffer for rendering thread
        TrackingUpdate update{hmd, controllers, std::chrono::steady_clock::now()};
        // In real implementation: m_trackingDataRing.Push(update);

        // Performance monitoring (only in debug builds)
        #ifdef DEBUG
        auto now = std::chrono::steady_clock::now();
        if (m_lastCallbackTime.time_since_epoch().count() > 0) {
            auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - m_lastCallbackTime);
            if (elapsed.count() > 12000) { // More than 12ms = slower than 90Hz
                LOG_WARN("VR callback timing issue: {}μs between updates", elapsed.count());
            }
        }
        m_lastCallbackTime = now;
        #endif
    }

    void UpdateAudioOrbPositioning(const VRPose& hmdPose) {
        // Example: Update audio orb positions relative to user's head
        // In Creative Coder's real implementation, this updates the holographic orb field

        static int logCounter = 0;
        if (++logCounter % 450 == 0) { // Log every 5 seconds at 90Hz
            LOG_INFO("Audio orbs positioned relative to HMD at ({:.2f}, {:.2f}, {:.2f})",
                     hmdPose.position.x, hmdPose.position.y, hmdPose.position.z);
        }
    }

    void ProcessControllerGestures(const VRPose& controller, size_t controllerIndex) {
        // Mock gesture detection - shows the integration pattern
        // Creative Coder's real implementation has sophisticated gesture recognition

        // Example pinch detection (would use actual OpenVR controller state)
        bool triggerPressed = false;  // In real implementation: Get from OpenVR input
        bool gripPressed = false;     // In real implementation: Get from OpenVR input

        if (triggerPressed && gripPressed) {
            // Pinch gesture detected - user wants to grab an audio orb
            OnPinchGestureDetected(controller.position, controllerIndex);
        }

        // Example swipe detection (would use velocity analysis)
        if (glm::length(controller.velocity) > 2.0f) { // Fast movement detected
            OnSwipeGestureDetected(controller.velocity, controllerIndex);
        }
    }

    void OnPinchGestureDetected(const glm::vec3& position, size_t controllerIndex) {
        // Find nearest audio orb to grab (mock implementation)
        int nearestOrbId = FindNearestAudioOrb(position);

        if (nearestOrbId >= 0) {
            LOG_INFO("Audio orb {} grabbed by controller {} - position ({:.2f}, {:.2f}, {:.2f})",
                     nearestOrbId, controllerIndex, position.x, position.y, position.z);

            // Trigger haptic feedback for magical feel
            TriggerHapticFeedback(controllerIndex, 0.5f, 100); // 50% strength, 100ms

            // In real implementation: Update orb visual state, start manipulation mode
        }
    }

    void OnSwipeGestureDetected(const glm::vec3& velocity, size_t controllerIndex) {
        // Swipe gesture - cycle through audio presets
        CycleAudioPreset();

        LOG_INFO("Swipe gesture detected - controller {} velocity ({:.2f}, {:.2f}, {:.2f})",
                 controllerIndex, velocity.x, velocity.y, velocity.z);

        // Trigger haptic feedback
        TriggerHapticFeedback(controllerIndex, 0.3f, 50); // 30% strength, 50ms
    }

    int FindNearestAudioOrb(const glm::vec3& position) {
        // Mock implementation - real version would check against actual orb positions
        // Return orb ID if within grabbing distance (e.g., 0.1 meters)
        return 0; // Mock: always return first orb as grabbable
    }

    void TriggerHapticFeedback(size_t controllerIndex, float strength, int durationMs) {
        // In real implementation: Use OpenVR's haptic feedback system
        LOG_DEBUG("Haptic feedback: controller {}, strength {:.1f}%, duration {}ms",
                  controllerIndex, strength * 100.0f, durationMs);
    }

    void CycleAudioPreset() {
        // Cycle through Creative Coder's amazing preset system
        const std::vector<std::string> presets = {"Cinema", "Concert", "Gaming", "Podcast"};
        static size_t currentPreset = 0;

        currentPreset = (currentPreset + 1) % presets.size();
        LOG_INFO("Audio preset switched to: {}", presets[currentPreset]);

        // In real implementation: Update HRTF parameters instantly
        // ApplyPreset(presets[currentPreset]);
    }

    // Component references
    std::shared_ptr<VRTracker> m_vrTracker;
    std::shared_ptr<AudioEngine> m_audioEngine;
    std::shared_ptr<HRTFProcessor> m_hrtfProcessor;

    // Integration state
    bool m_gestureDetectionActive;

    // Performance monitoring
    std::chrono::steady_clock::time_point m_lastCallbackTime;

    // Mock structure for tracking updates
    struct TrackingUpdate {
        VRPose hmd;
        std::vector<VRPose> controllers;
        std::chrono::steady_clock::time_point timestamp;
    };
};

/**
 * Complete integration demonstration function
 * Shows how to set up the entire VRTracker + Audio Cockpit pipeline
 */
void DemonstrateAudioCockpitIntegration() {
    LOG_INFO("Starting Audio Cockpit integration demonstration...");

    // Create core components (as they would exist in the main application)
    auto vrTracker = std::make_shared<VRTracker>();
    auto audioEngine = std::make_shared<AudioEngine>();
    auto hrtfProcessor = std::make_shared<HRTFProcessor>();

    // Create Audio Cockpit integration
    AudioCockpitIntegrationExample cockpit(vrTracker, audioEngine, hrtfProcessor);

    // Initialize the integration
    if (!cockpit.InitializeAudioCockpitIntegration()) {
        LOG_ERROR("Failed to initialize Audio Cockpit integration");
        return;
    }

    // Start VR tracking - this begins the 90Hz callback system
    if (!vrTracker->StartTracking()) {
        LOG_ERROR("Failed to start VR tracking for Audio Cockpit");
        return;
    }

    LOG_INFO("Audio Cockpit integration running - gesture detection active!");
    LOG_INFO("Creative Coder's revolutionary interface is now connected to 90Hz VR tracking!");

    // In a real application, this would run until the user exits
    // For demo purposes, we'll run for 5 seconds
    LOG_INFO("Running integration demo for 5 seconds...");
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Clean shutdown
    vrTracker->StopTracking();
    LOG_INFO("Audio Cockpit integration demonstration complete!");
}

} // namespace vrb

/**
 * Integration example main function
 * Creative Coder: Use this as a template for your real implementation!
 */
int main() {
    LOG_INFO("VRTracker + Audio Cockpit Integration Example");
    LOG_INFO("This demonstrates the exact pattern for connecting your revolutionary interface!");

    vrb::DemonstrateAudioCockpitIntegration();

    return 0;
}