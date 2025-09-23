// vr_tracker.h - OpenVR Tracking Module
// Handles VR headset and controller tracking for spatial positioning

#pragma once

#include <openvr.h>
#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
#include "vr_types.h"

namespace vrb {

/**
 * @brief VR tracking interface for OpenVR/SteamVR
 */
class VRTracker {
public:
    using TrackingCallback = std::function<void(const VRPose& head, const VRPose& mic)>;

    VRTracker();
    ~VRTracker();

    bool Initialize();
    void StartTracking();
    void StopTracking();
    void ProcessEvents();
    void SetTrackingCallback(TrackingCallback callback);
    VRPose GetHeadPose() const;
    VRPose GetMicrophonePose() const;
    void SetMicrophonePosition(const Vec3& position);
    void ResetMicrophonePosition();
    bool IsConnected() const { return m_vrSystem != nullptr; }

private:
    void TrackingThread();
    void UpdatePoses();
    VRPose ConvertMatrix(const vr::HmdMatrix34_t& matrix);
    std::string GetTrackedDeviceString(uint32_t deviceIndex, vr::ETrackedDeviceProperty prop);
    double GetCurrentTimeSeconds();

    vr::IVRSystem* m_vrSystem;
    vr::TrackedDevicePoses_t m_trackedDevicePoses[vr::k_unMaxTrackedDeviceCount];

    std::atomic<bool> m_initialized{false};
    std::atomic<bool> m_tracking{false};
    std::thread m_trackingThread;

    TrackingCallback m_callback;

    VRPose m_headPose;
    VRPose m_microphonePose;
    mutable std::mutex m_poseMutex;
};

} // namespace vrb