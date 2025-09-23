// vr_tracker.h - Simplified OpenVR Direct API Tracking
// Direct OpenVR integration for HMD and controller tracking only

#pragma once

#include <openvr.h>
#include <functional>
#include <mutex>
#include <vector>
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
    void Update();  // Call this each frame to get latest poses
    void SetTrackingCallback(TrackingCallback callback);

    // Pose access
    VRPose GetHMDPose() const;
    std::vector<VRPose> GetControllerPoses() const;

    // Simple state queries
    bool IsInitialized() const { return m_vrSystem != nullptr; }
    bool IsHMDConnected() const;
    int GetConnectedControllerCount() const;

    // Basic headset info for audio optimizations (extracted directly from OpenVR)
    std::string GetHMDModel() const;
    float GetHMDRefreshRate() const;

private:
    // Core OpenVR functionality only
    VRPose ConvertOpenVRPose(const vr::TrackedDevicePose_t& vrPose);
    void ExtractHMDPose(const vr::TrackedDevicePose_t poses[]);
    void ExtractControllerPoses(const vr::TrackedDevicePose_t poses[]);

    // OpenVR system
    vr::IVRSystem* m_vrSystem;

    // Tracking data - updated each frame
    VRPose m_hmdPose;
    std::vector<VRPose> m_controllerPoses;
    mutable std::mutex m_poseMutex;

    // Callback for audio engine
    TrackingCallback m_callback;
};

} // namespace vrb