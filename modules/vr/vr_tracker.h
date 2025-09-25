// vr_tracker.h - Simplified OpenVR Direct API Tracking
// Direct OpenVR integration for HMD and controller tracking only

#pragma once

#include <openvr.h>
#include <functional>
#include <mutex>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include "vr_types.h"

namespace vrb {

/**
 * @brief Simplified VR tracker using direct OpenVR API calls
 * Focuses only on HMD + controller tracking for spatial audio
 */
class VRTracker {
public:
    using TrackingCallback = std::function<void(const VRPose& hmd, const std::vector<VRPose>& controllers)>;

    VRTracker();
    ~VRTracker();

    // Core functionality
    bool Initialize();
    bool StartTracking();  // Start 90Hz tracking thread as per Solution Architect design
    void StopTracking();   // Stop tracking thread
    void Update();  // Call this each frame to get latest poses
    void ProcessEvents();  // Process VR events (alias for Update for compatibility)
    void SetTrackingCallback(TrackingCallback callback);
    void Shutdown();  // Complete shutdown and cleanup - stops tracking and releases VR resources

    // Pose access
    VRPose GetHMDPose() const;
    std::vector<VRPose> GetControllerPoses() const;

    // Simple state queries
    bool IsInitialized() const { return m_vrSystem != nullptr; }
    bool IsHMDConnected() const;
    bool IsConnected() const { return IsHMDConnected(); } // Alias for test compatibility
    int GetConnectedControllerCount() const;

    // Testing framework compatibility methods
    void EnableSimulationMode(bool enable) { /* Not implemented - hardware-only tracker */ }
    bool IsSimulationMode() const { return false; } // Always hardware mode
    std::vector<std::string> GetTroubleshootingSteps() const;

    // Basic headset info for audio optimizations (extracted directly from OpenVR)
    std::string GetHMDModel() const;
    float GetHMDRefreshRate() const;

private:
    // Core OpenVR functionality only
    VRPose ConvertOpenVRPose(const vr::TrackedDevicePose_t& vrPose);
    void ExtractHMDPose(const vr::TrackedDevicePose_t poses[]);
    void ExtractControllerPoses(const vr::TrackedDevicePose_t poses[]);

    // Threading implementation as per Solution Architect design
    void TrackingLoop();  // 90Hz tracking thread main function

    // OpenVR system
    vr::IVRSystem* m_vrSystem;

    // Tracking data - updated each frame
    VRPose m_hmdPose;
    std::vector<VRPose> m_controllerPoses;
    mutable std::mutex m_poseMutex;

    // Threading infrastructure
    std::thread m_trackingThread;
    std::atomic<bool> m_running{false};

    // Thread-safe callback system for Audio Cockpit integration
    std::atomic<TrackingCallback*> m_audioCallback{nullptr};
    std::mutex m_callbackMutex;
};

} // namespace vrb