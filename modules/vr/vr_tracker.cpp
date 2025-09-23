// vr_tracker.cpp - OpenVR Tracking Implementation
// High-velocity VR tracking system with prediction and thread safety!

#include "vr_tracker.h"
#include "logger.h"
#include <chrono>
#include <cstring>
#include <cmath>

namespace vrb {

VRTracker::VRTracker() : m_vrSystem(nullptr) {
    // Initialize the tracking device poses array to safe defaults
    std::memset(m_trackedDevicePoses, 0, sizeof(m_trackedDevicePoses));

    // Set up default microphone position - this is where the magic happens!
    // Start it 1 meter in front of user at chest height for optimal binaural capture
    m_microphonePose.position = {0.0f, 1.2f, -1.0f};
    m_microphonePose.orientation = {1.0f, 0.0f, 0.0f, 0.0f}; // Identity quaternion
    m_microphonePose.isValid = true;
    m_microphonePose.timestamp = 0.0;

    // Initialize head pose to safe defaults
    m_headPose.position = {0.0f, 1.6f, 0.0f}; // Average head height
    m_headPose.orientation = {1.0f, 0.0f, 0.0f, 0.0f};
    m_headPose.isValid = false;
    m_headPose.timestamp = 0.0;

    LOG_INFO("VRTracker constructed - ready for spatial audio magic!");
}

VRTracker::~VRTracker() {
    // Clean shutdown sequence - no hanging threads!
    StopTracking();

    if (m_vrSystem) {
        vr::VR_Shutdown();
        m_vrSystem = nullptr;
        LOG_INFO("OpenVR shutdown complete - VR tracking terminated");
    }
}

bool VRTracker::Initialize() {
    if (m_initialized) {
        LOG_WARN("VR tracker already initialized - skipping reinit");
        return true;
    }

    LOG_INFO("Initializing VR tracking system...");

    // First, check if VR runtime is even installed
    if (!vr::VR_IsRuntimeInstalled()) {
        LOG_ERROR("SteamVR runtime not found! Please install SteamVR first.");
        return false;
    }

    // Check if we have a headset connected
    if (!vr::VR_IsHmdPresent()) {
        LOG_WARN("No VR headset detected - this will work in simulator mode");
        // Don't fail here - we can still run for development/testing
    }

    // Initialize OpenVR system - this is the critical moment!
    vr::EVRInitError vrError = vr::VRInitError_None;
    m_vrSystem = vr::VR_Init(&vrError, vr::VRApplication_Overlay);

    if (vrError != vr::VRInitError_None) {
        m_vrSystem = nullptr;
        std::string errorDesc = vr::VR_GetVRInitErrorAsEnglishDescription(vrError);
        LOG_WARN("Failed to initialize OpenVR: {} - Running in simulation mode", errorDesc);

        // For development: create a simulator mode that generates fake head movement
        m_initialized = true;
        return true; // Continue in simulation mode
    }

    // Get display frequency for optimal tracking timing
    float displayFreq = 90.0f;  // Safe default
    if (m_vrSystem->IsTrackedDeviceConnected(vr::k_unTrackedDeviceIndex_Hmd)) {
        displayFreq = m_vrSystem->GetFloatTrackedDeviceProperty(
            vr::k_unTrackedDeviceIndex_Hmd,
            vr::Prop_DisplayFrequency_Float,
            nullptr
        );
    }

    // Validate and log system info
    std::string driverName = GetTrackedDeviceString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
    std::string modelName = GetTrackedDeviceString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_ModelNumber_String);

    LOG_INFO("VR System initialized successfully!");
    LOG_INFO("  Driver: {}", driverName);
    LOG_INFO("  Model: {}", modelName);
    LOG_INFO("  Display Frequency: {}Hz", displayFreq);

    m_initialized = true;
    return true;
}

void VRTracker::StartTracking() {
    if (!m_initialized) {
        LOG_ERROR("Cannot start tracking - VR system not initialized!");
        return;
    }

    if (m_tracking) {
        LOG_WARN("Tracking already started - ignoring duplicate start request");
        return;
    }

    LOG_INFO("Starting VR tracking thread - prepare for spatial audio awesomeness!");

    m_tracking = true;
    m_trackingThread = std::thread(&VRTracker::TrackingThread, this);

    LOG_INFO("VR tracking thread launched successfully");
}

void VRTracker::StopTracking() {
    if (!m_tracking) {
        return; // Already stopped
    }

    LOG_INFO("Stopping VR tracking...");

    m_tracking = false;

    // Wait for tracking thread to finish cleanly
    if (m_trackingThread.joinable()) {
        m_trackingThread.join();
    }

    LOG_INFO("VR tracking stopped cleanly");
}

void VRTracker::ProcessEvents() {
    if (!m_vrSystem) {
        return;
    }

    // Process all pending VR events - this keeps the system responsive
    vr::VREvent_t event;
    while (m_vrSystem->PollNextEvent(&event, sizeof(event))) {
        switch (event.eventType) {
            case vr::VREvent_TrackedDeviceActivated:
                LOG_INFO("VR Device {} activated - tracking enhanced!", event.trackedDeviceIndex);
                break;

            case vr::VREvent_TrackedDeviceDeactivated:
                LOG_INFO("VR Device {} deactivated", event.trackedDeviceIndex);
                break;

            case vr::VREvent_TrackedDeviceUpdated:
                LOG_DEBUG("VR Device {} updated", event.trackedDeviceIndex);
                break;

            case vr::VREvent_DashboardActivated:
                LOG_DEBUG("SteamVR Dashboard activated");
                break;

            case vr::VREvent_DashboardDeactivated:
                LOG_DEBUG("SteamVR Dashboard deactivated");
                break;

            case vr::VREvent_ChaperoneDataHasChanged:
                LOG_INFO("Chaperone bounds updated - recalibrating space");
                break;

            case vr::VREvent_StatusUpdate:
                LOG_DEBUG("VR system status update");
                break;

            default:
                // Lots of events we don't care about - keep it clean
                break;
        }
    }
}

void VRTracker::SetTrackingCallback(TrackingCallback callback) {
    m_callback = callback;
    LOG_INFO("Tracking callback registered - spatial updates will be delivered!");
}

VRPose VRTracker::GetHeadPose() const {
    std::lock_guard<std::mutex> lock(m_poseMutex);
    return m_headPose;
}

VRPose VRTracker::GetMicrophonePose() const {
    std::lock_guard<std::mutex> lock(m_poseMutex);
    return m_microphonePose;
}

void VRTracker::SetMicrophonePosition(const Vec3& position) {
    std::lock_guard<std::mutex> lock(m_poseMutex);
    m_microphonePose.position = position;
    m_microphonePose.timestamp = GetCurrentTimeSeconds();

    LOG_INFO("Virtual microphone positioned at: ({:.2f}, {:.2f}, {:.2f})",
             position.x, position.y, position.z);
}

void VRTracker::ResetMicrophonePosition() {
    // Reset to optimal binaural recording position
    SetMicrophonePosition({0.0f, 1.2f, -1.0f});
    LOG_INFO("Microphone reset to default binaural position");
}

void VRTracker::TrackingThread() {
    LOG_INFO("VR tracking thread running - delivering spatial data at warp speed!");

    // Target 90Hz tracking for smooth audio positioning
    const auto trackingPeriod = std::chrono::microseconds(11111); // ~90Hz
    auto nextUpdate = std::chrono::steady_clock::now();

    while (m_tracking) {
        // Update all tracked device poses
        UpdatePoses();

        // Deliver tracking data via callback if registered
        if (m_callback) {
            std::lock_guard<std::mutex> lock(m_poseMutex);
            m_callback(m_headPose, m_microphonePose);
        }

        // Maintain precise timing for smooth audio
        nextUpdate += trackingPeriod;
        std::this_thread::sleep_until(nextUpdate);
    }

    LOG_INFO("VR tracking thread terminated gracefully");
}

void VRTracker::UpdatePoses() {
    double currentTime = GetCurrentTimeSeconds();

    if (!m_vrSystem) {
        // SIMULATION MODE - Generate fake head movement for development
        std::lock_guard<std::mutex> lock(m_poseMutex);

        // Create subtle head movement simulation - gentle swaying motion
        float time = static_cast<float>(currentTime);
        m_headPose.position.x = 0.1f * std::sin(time * 0.5f);  // Side-to-side
        m_headPose.position.y = 1.6f + 0.05f * std::sin(time * 0.7f);  // Slight up-down
        m_headPose.position.z = 0.02f * std::sin(time * 0.3f);  // Forward-back

        // Gentle head rotation simulation
        float yaw = 0.1f * std::sin(time * 0.4f);  // Look left-right
        float pitch = 0.05f * std::sin(time * 0.6f);  // Nod slightly

        // Convert to quaternion (simplified Euler to quaternion)
        float cy = std::cos(yaw * 0.5f);
        float sy = std::sin(yaw * 0.5f);
        float cp = std::cos(pitch * 0.5f);
        float sp = std::sin(pitch * 0.5f);

        m_headPose.orientation.w = cy * cp;
        m_headPose.orientation.x = sy * sp;
        m_headPose.orientation.y = sy * cp;
        m_headPose.orientation.z = cy * sp;

        m_headPose.isValid = true;
        m_headPose.timestamp = currentTime;

        LOG_DEBUG("Simulation head pose: pos({:.2f}, {:.2f}, {:.2f})",
                 m_headPose.position.x, m_headPose.position.y, m_headPose.position.z);
        return;
    }

    // REAL VR MODE - Get the latest poses from all tracked devices
    vr::VRCompositor()->GetLastPoses(m_trackedDevicePoses, vr::k_unMaxTrackedDeviceCount,
                                     nullptr, 0);

    // Find and update HMD pose - this is our primary tracking target
    for (uint32_t device = 0; device < vr::k_unMaxTrackedDeviceCount; ++device) {
        if (m_vrSystem->GetTrackedDeviceClass(device) == vr::TrackedDeviceClass_HMD) {
            const vr::TrackedDevicePose_t& devicePose = m_trackedDevicePoses[device];

            if (devicePose.bPoseIsValid && devicePose.bDeviceIsConnected) {
                std::lock_guard<std::mutex> lock(m_poseMutex);

                // Convert OpenVR matrix to our pose format
                m_headPose = ConvertMatrix(devicePose.mDeviceToAbsoluteTracking);
                m_headPose.isValid = true;
                m_headPose.timestamp = currentTime;

                // Extract velocity data for prediction
                m_headPose.velocity = Vec3(
                    devicePose.vVelocity.v[0],
                    devicePose.vVelocity.v[1],
                    devicePose.vVelocity.v[2]
                );

                m_headPose.angularVelocity = Vec3(
                    devicePose.vAngularVelocity.v[0],
                    devicePose.vAngularVelocity.v[1],
                    devicePose.vAngularVelocity.v[2]
                );

                LOG_DEBUG("Head pose updated: pos({:.2f}, {:.2f}, {:.2f})",
                         m_headPose.position.x, m_headPose.position.y, m_headPose.position.z);
            } else {
                // Mark head pose as invalid if tracking is lost
                std::lock_guard<std::mutex> lock(m_poseMutex);
                m_headPose.isValid = false;
                LOG_WARN("Head tracking lost - pose marked invalid");
            }
            break;
        }
    }

    // TODO: Add controller tracking for microphone positioning
    // This is where we'll implement hand-held microphone simulation
}

VRPose VRTracker::ConvertMatrix(const vr::HmdMatrix34_t& matrix) {
    VRPose pose;

    // Extract position from transformation matrix
    pose.position.x = matrix.m[0][3];
    pose.position.y = matrix.m[1][3];
    pose.position.z = matrix.m[2][3];

    // Extract rotation matrix components for quaternion conversion
    // This is the robust Shepperd's method for matrix-to-quaternion conversion
    float m00 = matrix.m[0][0], m01 = matrix.m[0][1], m02 = matrix.m[0][2];
    float m10 = matrix.m[1][0], m11 = matrix.m[1][1], m12 = matrix.m[1][2];
    float m20 = matrix.m[2][0], m21 = matrix.m[2][1], m22 = matrix.m[2][2];

    float trace = m00 + m11 + m22;

    if (trace > 0.0f) {
        // Standard case - no singularities
        float s = 2.0f * sqrtf(trace + 1.0f);
        pose.orientation.w = 0.25f * s;
        pose.orientation.x = (m21 - m12) / s;
        pose.orientation.y = (m02 - m20) / s;
        pose.orientation.z = (m10 - m01) / s;
    } else if ((m00 > m11) && (m00 > m22)) {
        // m00 is largest diagonal element
        float s = 2.0f * sqrtf(1.0f + m00 - m11 - m22);
        pose.orientation.w = (m21 - m12) / s;
        pose.orientation.x = 0.25f * s;
        pose.orientation.y = (m01 + m10) / s;
        pose.orientation.z = (m02 + m20) / s;
    } else if (m11 > m22) {
        // m11 is largest diagonal element
        float s = 2.0f * sqrtf(1.0f + m11 - m00 - m22);
        pose.orientation.w = (m02 - m20) / s;
        pose.orientation.x = (m01 + m10) / s;
        pose.orientation.y = 0.25f * s;
        pose.orientation.z = (m12 + m21) / s;
    } else {
        // m22 is largest diagonal element
        float s = 2.0f * sqrtf(1.0f + m22 - m00 - m11);
        pose.orientation.w = (m10 - m01) / s;
        pose.orientation.x = (m02 + m20) / s;
        pose.orientation.y = (m12 + m21) / s;
        pose.orientation.z = 0.25f * s;
    }

    // Normalize quaternion to handle floating point precision issues
    float norm = sqrtf(pose.orientation.x * pose.orientation.x +
                      pose.orientation.y * pose.orientation.y +
                      pose.orientation.z * pose.orientation.z +
                      pose.orientation.w * pose.orientation.w);

    if (norm > 0.0001f) {
        pose.orientation.x /= norm;
        pose.orientation.y /= norm;
        pose.orientation.z /= norm;
        pose.orientation.w /= norm;
    } else {
        // Fallback to identity quaternion if normalization fails
        pose.orientation = {1.0f, 0.0f, 0.0f, 0.0f};
        LOG_WARN("Quaternion normalization failed - using identity");
    }

    return pose;
}

// Helper function to get device strings safely
std::string VRTracker::GetTrackedDeviceString(uint32_t deviceIndex, vr::ETrackedDeviceProperty prop) {
    if (!m_vrSystem) {
        return "Unknown";
    }

    uint32_t requiredBufferLen = m_vrSystem->GetStringTrackedDeviceProperty(
        deviceIndex, prop, nullptr, 0, nullptr);

    if (requiredBufferLen == 0) {
        return "Unknown";
    }

    std::string result(requiredBufferLen, '\0');
    m_vrSystem->GetStringTrackedDeviceProperty(
        deviceIndex, prop, &result[0], requiredBufferLen, nullptr);

    // Remove null terminator if present
    if (!result.empty() && result.back() == '\0') {
        result.pop_back();
    }

    return result;
}

// Helper to get current time in seconds with high precision
double VRTracker::GetCurrentTimeSeconds() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration<double>(duration).count();
}

} // namespace vrb