// vr_tracker.cpp - Simplified OpenVR Direct API Implementation
// Clean, efficient VR tracking using OpenVR's optimized APIs directly

#include "vr_tracker.h"
#include "logger.h"
#include <chrono>
#include <cmath>
#include <thread>

namespace vrb {

VRTracker::VRTracker() : m_vrSystem(nullptr) {
    // Initialize to safe defaults
    m_hmdPose.position = {0.0f, 1.7f, 0.0f};  // Average head height
    m_hmdPose.orientation = {1.0f, 0.0f, 0.0f, 0.0f};  // Identity quaternion
    m_hmdPose.isValid = false;

    LOG_INFO("VRTracker created - ready for direct OpenVR integration");
}

VRTracker::~VRTracker() {
    StopTracking();  // Ensure tracking thread is stopped

    // Clean up atomic callback pointer
    TrackingCallback* oldCallback = m_audioCallback.load();
    if (oldCallback) {
        delete oldCallback;
        m_audioCallback.store(nullptr);
    }

    if (m_vrSystem) {
        vr::VR_Shutdown();
        m_vrSystem = nullptr;
        LOG_INFO("OpenVR shutdown complete");
    }
}

bool VRTracker::Initialize() {
    LOG_INFO("Initializing OpenVR system...");

    // Check if VR runtime is available
    if (!vr::VR_IsRuntimeInstalled()) {
        LOG_WARN("SteamVR runtime not installed");
        return false;
    }

    if (!vr::VR_IsHmdPresent()) {
        LOG_WARN("No VR headset detected");
        return false;
    }

    // Initialize OpenVR - this is all we need!
    vr::EVRInitError vrError = vr::VRInitError_None;
    m_vrSystem = vr::VR_Init(&vrError, vr::VRApplication_Background);

    if (vrError != vr::VRInitError_None) {
        m_vrSystem = nullptr;
        std::string errorDesc = vr::VR_GetVRInitErrorAsEnglishDescription(vrError);
        LOG_ERROR("Failed to initialize OpenVR: {}", errorDesc);
        return false;
    }

    // Log basic system info
    char buffer[1024];
    uint32_t len = m_vrSystem->GetStringTrackedDeviceProperty(
        vr::k_unTrackedDeviceIndex_Hmd,
        vr::Prop_ModelNumber_String,
        buffer,
        sizeof(buffer),
        nullptr
    );
    std::string hmdModel = (len > 0) ? std::string(buffer) : "Unknown";

    LOG_INFO("OpenVR initialized successfully");
    LOG_INFO("HMD Model: {}", hmdModel.empty() ? "Unknown" : hmdModel);
    LOG_INFO("VR system ready for efficient tracking");

    return true;
}

bool VRTracker::StartTracking() {
    if (!Initialize()) {
        LOG_ERROR("Failed to initialize VR tracking");
        return false;
    }

    if (m_trackingThread.joinable()) {
        LOG_WARN("Tracking already running");
        return true;
    }

    m_running.store(true);
    m_trackingThread = std::thread(&VRTracker::TrackingLoop, this);

    LOG_INFO("VR tracking started - integrated with audio pipeline");
    return true;
}

void VRTracker::StopTracking() {
    if (m_running.load()) {
        m_running.store(false);
        if (m_trackingThread.joinable()) {
            m_trackingThread.join();
        }
        LOG_INFO("VR tracking stopped");
    }
}

void VRTracker::Shutdown() {
    LOG_INFO("Shutting down VR tracker...");

    // Stop tracking thread if running
    StopTracking();

    // Clean up atomic callback pointer
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    TrackingCallback* oldCallback = m_audioCallback.load();
    if (oldCallback) {
        delete oldCallback;
        m_audioCallback.store(nullptr);
    }

    // Shutdown OpenVR system
    if (m_vrSystem) {
        vr::VR_Shutdown();
        m_vrSystem = nullptr;
        LOG_INFO("OpenVR shutdown complete");
    }

    // Reset poses to safe defaults
    {
        std::lock_guard<std::mutex> poseLock(m_poseMutex);
        m_hmdPose.isValid = false;
        m_controllerPoses.clear();
    }

    LOG_INFO("VR tracker shutdown complete");
}

void VRTracker::TrackingLoop() {
    // Use compositor frame timing instead of fixed 90Hz
    while (m_running.load()) {
        Update(); // Now uses WaitGetPoses for proper frame sync
    }
}

void VRTracker::Update() {
    if (!m_vrSystem) {
        return;
    }

    // Modern API with fallback for development stub
    vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];

    // Modern API implementation with stub compatibility
    #ifdef OPENVR_STUB_BUILD
        // Development stub - use optimized legacy API with proper prediction
        m_vrSystem->GetDeviceToAbsoluteTrackingPose(
            vr::TrackingUniverseStanding,
            0.011f,  // ~11ms prediction for 90Hz (1/90 ≈ 0.011s) - optimized for low latency
            poses,
            vr::k_unMaxTrackedDeviceCount
        );
    #else
        // Production: Use modern frame-synchronized API for optimal performance
        if (vr::VRCompositor() != nullptr) {
            // WaitGetPoses provides frame-synchronized poses with <5ms latency
            vr::VRCompositor()->WaitGetPoses(poses, vr::k_unMaxTrackedDeviceCount, nullptr, 0);
        } else {
            // Fallback for non-compositor applications
            m_vrSystem->GetDeviceToAbsoluteTrackingPose(
                vr::TrackingUniverseStanding,
                0.011f,  // Optimized prediction timing
                poses,
                vr::k_unMaxTrackedDeviceCount
            );
        }
    #endif

    // Extract what we actually need
    {
        std::lock_guard<std::mutex> lock(m_poseMutex);
        ExtractHMDPose(poses);
        ExtractControllerPoses(poses);
    }

    // Notify Audio Cockpit with thread-safe callback system
    TrackingCallback* callback = m_audioCallback.load();
    if (callback) {
        (*callback)(m_hmdPose, m_controllerPoses);  // Creative Coder's gesture magic happens here!
    }
}

void VRTracker::ProcessEvents() {
    // ProcessEvents is an alias for Update for compatibility with Application
    Update();
}

void VRTracker::SetTrackingCallback(TrackingCallback callback) {
    std::lock_guard<std::mutex> lock(m_callbackMutex);

    // Clean up old callback
    TrackingCallback* oldCallback = m_audioCallback.load();
    if (oldCallback) {
        delete oldCallback;
    }

    // Set new callback atomically for Creative Coder's Audio Cockpit
    m_audioCallback.store(new TrackingCallback(std::move(callback)));
    LOG_INFO("Thread-safe tracking callback registered for Audio Cockpit integration");
}

VRPose VRTracker::GetHMDPose() const {
    std::lock_guard<std::mutex> lock(m_poseMutex);
    return m_hmdPose;
}

std::vector<VRPose> VRTracker::GetControllerPoses() const {
    std::lock_guard<std::mutex> lock(m_poseMutex);
    return m_controllerPoses;
}

bool VRTracker::IsHMDConnected() const {
    if (!m_vrSystem) return false;
    return m_vrSystem->IsTrackedDeviceConnected(vr::k_unTrackedDeviceIndex_Hmd);
}

int VRTracker::GetConnectedControllerCount() const {
    if (!m_vrSystem) return 0;

    int count = 0;
    for (uint32_t device = 0; device < vr::k_unMaxTrackedDeviceCount; ++device) {
        if (m_vrSystem->GetTrackedDeviceClass(device) == vr::TrackedDeviceClass_Controller &&
            m_vrSystem->IsTrackedDeviceConnected(device)) {
            count++;
        }
    }
    return count;
}

VRPose VRTracker::ConvertOpenVRPose(const vr::TrackedDevicePose_t& vrPose) {
    VRPose pose;

    if (!vrPose.bPoseIsValid || !vrPose.bDeviceIsConnected) {
        pose.isValid = false;
        return pose;
    }

    const vr::HmdMatrix34_t& matrix = vrPose.mDeviceToAbsoluteTracking;

    // Extract position directly from OpenVR matrix
    pose.position.x = matrix.m[0][3];
    pose.position.y = matrix.m[1][3];
    pose.position.z = matrix.m[2][3];

    // Extract rotation matrix and convert to quaternion using robust method
    float m00 = matrix.m[0][0], m01 = matrix.m[0][1], m02 = matrix.m[0][2];
    float m10 = matrix.m[1][0], m11 = matrix.m[1][1], m12 = matrix.m[1][2];
    float m20 = matrix.m[2][0], m21 = matrix.m[2][1], m22 = matrix.m[2][2];

    float trace = m00 + m11 + m22;

    if (trace > 0.0f) {
        float s = 2.0f * std::sqrt(trace + 1.0f);
        pose.orientation.w = 0.25f * s;
        pose.orientation.x = (m21 - m12) / s;
        pose.orientation.y = (m02 - m20) / s;
        pose.orientation.z = (m10 - m01) / s;
    } else if ((m00 > m11) && (m00 > m22)) {
        float s = 2.0f * std::sqrt(1.0f + m00 - m11 - m22);
        pose.orientation.w = (m21 - m12) / s;
        pose.orientation.x = 0.25f * s;
        pose.orientation.y = (m01 + m10) / s;
        pose.orientation.z = (m02 + m20) / s;
    } else if (m11 > m22) {
        float s = 2.0f * std::sqrt(1.0f + m11 - m00 - m22);
        pose.orientation.w = (m02 - m20) / s;
        pose.orientation.x = (m01 + m10) / s;
        pose.orientation.y = 0.25f * s;
        pose.orientation.z = (m12 + m21) / s;
    } else {
        float s = 2.0f * std::sqrt(1.0f + m22 - m00 - m11);
        pose.orientation.w = (m10 - m01) / s;
        pose.orientation.x = (m02 + m20) / s;
        pose.orientation.y = (m12 + m21) / s;
        pose.orientation.z = 0.25f * s;
    }

    // Use OpenVR's velocity data directly - no custom prediction needed!
    pose.velocity.x = vrPose.vVelocity.v[0];
    pose.velocity.y = vrPose.vVelocity.v[1];
    pose.velocity.z = vrPose.vVelocity.v[2];

    pose.angularVelocity.x = vrPose.vAngularVelocity.v[0];
    pose.angularVelocity.y = vrPose.vAngularVelocity.v[1];
    pose.angularVelocity.z = vrPose.vAngularVelocity.v[2];

    // Set timestamp
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    pose.timestamp = std::chrono::duration<double>(duration).count();

    pose.isValid = true;
    return pose;
}

void VRTracker::ExtractHMDPose(const vr::TrackedDevicePose_t poses[]) {
    // HMD is always device index 0 in OpenVR
    m_hmdPose = ConvertOpenVRPose(poses[vr::k_unTrackedDeviceIndex_Hmd]);

    if (m_hmdPose.isValid) {
        LOG_DEBUG("HMD pose: pos({:.2f}, {:.2f}, {:.2f})",
                 m_hmdPose.position.x, m_hmdPose.position.y, m_hmdPose.position.z);
    }
}

void VRTracker::ExtractControllerPoses(const vr::TrackedDevicePose_t poses[]) {
    m_controllerPoses.clear();

    // Find all connected controllers
    for (uint32_t device = 0; device < vr::k_unMaxTrackedDeviceCount; ++device) {
        if (m_vrSystem->GetTrackedDeviceClass(device) == vr::TrackedDeviceClass_Controller) {
            VRPose controllerPose = ConvertOpenVRPose(poses[device]);
            if (controllerPose.isValid) {
                m_controllerPoses.push_back(controllerPose);
                LOG_DEBUG("Controller {} pose: pos({:.2f}, {:.2f}, {:.2f})",
                         m_controllerPoses.size() - 1,
                         controllerPose.position.x, controllerPose.position.y, controllerPose.position.z);
            }
        }
    }

    LOG_DEBUG("Extracted {} valid controller poses", m_controllerPoses.size());
}

std::string VRTracker::GetHMDModel() const {
    if (!m_vrSystem) {
        return "Unknown";
    }

    // Get model directly from OpenVR - no complex detection needed
    char buffer[1024];
    uint32_t len = m_vrSystem->GetStringTrackedDeviceProperty(
        vr::k_unTrackedDeviceIndex_Hmd,
        vr::Prop_ModelNumber_String,
        buffer,
        sizeof(buffer),
        nullptr
    );

    if (len == 0) {
        return "Unknown";
    }

    return std::string(buffer);
}

float VRTracker::GetHMDRefreshRate() const {
    if (!m_vrSystem) {
        return 90.0f;  // Safe default
    }

    // Get refresh rate directly from OpenVR
    float refreshRate = m_vrSystem->GetFloatTrackedDeviceProperty(
        vr::k_unTrackedDeviceIndex_Hmd,
        vr::Prop_DisplayFrequency_Float,
        nullptr
    );

    // Return safe default if property not available
    return (refreshRate > 0.0f) ? refreshRate : 90.0f;
}

std::vector<std::string> VRTracker::GetTroubleshootingSteps() const {
    std::vector<std::string> steps;

    if (!m_vrSystem) {
        steps.push_back("1. Install SteamVR from Steam");
        steps.push_back("2. Connect your VR headset to PC");
        steps.push_back("3. Launch SteamVR and complete room setup");
        steps.push_back("4. Ensure headset is detected in SteamVR status");
        steps.push_back("5. Restart this application");
    } else if (!IsHMDConnected()) {
        steps.push_back("1. Check all VR headset cables are connected");
        steps.push_back("2. Restart SteamVR");
        steps.push_back("3. Check headset power and tracking");
        steps.push_back("4. Verify headset appears in SteamVR devices");
    } else {
        steps.push_back("VR system is working correctly");
        steps.push_back("All tracking devices are connected and functional");
    }

    return steps;
}

} // namespace vrb