// audio_routing_overlay.h - Simple ASMRtist Microphone Positioning Interface
// 🎤 Simple tools for content creators to position their mic in VR space 🎤
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include <functional>
#include <chrono>

// Using real OpenVR SDK for VR-first spatial audio
#include <openvr.h>
#include "../../core/include/vr_types.h"
#include "../vr/vr_tracker.h"
#include "../audio/audio_engine.h"
#include "../audio/hrtf_processor.h"

namespace vrb {

// Forward declarations
class HRTFProcessor;

/**
 * @brief Simple VR Microphone Positioning Interface for ASMRtists
 *
 * A focused tool that helps ASMRtists:
 * - Position their microphone in VR space
 * - Move around to create spatial audio effects
 * - Record/stream spatialized output
 * - Simple controls for content creation workflow
 */
class AudioRoutingOverlay {
public:
    AudioRoutingOverlay();
    ~AudioRoutingOverlay();

    // Core lifecycle
    bool Initialize(VRTracker* vrTracker, AudioEngine* audioEngine);
    void Update();
    void Shutdown();

    // Visibility control
    void SetVisible(bool visible);
    bool IsVisible() const { return m_visible.load(); }

    // Microphone positioning for ASMRtists
    void SetMicrophonePosition(const Vec3& position);
    void SetMicrophoneOrientation(const Vec3& forward);
    Vec3 GetMicrophonePosition() const;

    // Simple recording controls
    void StartRecording();
    void StopRecording();
    bool IsRecording() const;
    void SetMonitoring(bool enabled);  // Hear spatial effect in real-time

    // Virtual microphone representation
    struct VirtualMicrophone {
        Vec3 position{0, 1.5f, 0};     // Default at head height in front of user
        Vec3 orientation{0, 0, -1};    // Facing forward
        float sensitivity = 1.0f;
        bool isVisible = true;
        bool isActive = false;
    };

    VirtualMicrophone& GetVirtualMicrophone() { return m_virtualMic; }

    // Simple VR Integration - Basic controller tracking for mic positioning
    void UpdateControllerTracking(const std::vector<VRPose>& controllers);
    void UpdateMicrophoneTracking(const VRPose& hmdPose);

    // Gesture detection for Audio Cockpit compatibility
    void UpdateGestureDetection(const std::vector<VRPose>& controllers);
    void UpdateAudioOrbPositions(const VRPose& hmdPose);
    void RegisterGestureCallback(std::function<void(const std::string&, const Vec3&)> callback);

    // Audio Engine Integration - Simple level monitoring
    void SetHRTFProcessor(HRTFProcessor* processor);
    float GetInputLevel() const;
    float GetSpatializedOutputLevel() const;

private:
    // === CORE VR OVERLAY MAGIC ===
    bool CreateOverlay();
    void DestroyOverlay();
    bool CreateRenderTarget();
    void DestroyRenderTarget();

    void RenderToTexture();
    void UpdateOverlayTexture();
    void UpdateOverlayTransform();

    // === SIMPLE CONTROLLER INTERACTION ===
    struct ControllerState {
        Vec3 position{0, 0, 0};
        bool triggerPressed = false;
        bool wasTriggered = false;
        bool isDraggingMic = false;  // Simple drag to position microphone
    };

    void UpdateControllerStates();
    void HandleMicrophonePositioning();  // Simple trigger-drag to move mic

    // === SIMPLE UI RENDERING ===
    void RenderMicrophoneVisual();      // Simple mic icon in VR space
    void RenderRecordingStatus();       // Recording indicator
    void RenderSimpleControls();        // Basic start/stop/monitor buttons

    // === SIMPLE AUDIO LEVEL VISUALIZATION ===
    void RenderAudioLevelMeter();       // Basic level meter for input monitoring

    // === SIMPLE AUDIO MONITORING ===
    // Basic audio level tracking for content creators
    std::atomic<float> m_inputLevel{0.0f};
    std::atomic<float> m_outputLevel{0.0f};
    std::atomic<bool> m_isRecording{false};

    // === STATE MANAGEMENT ===
    std::atomic<bool> m_initialized{false};
    std::atomic<bool> m_visible{true};
    mutable std::mutex m_mutex;

    // VR Components
    VRTracker* m_vrTracker = nullptr;
    AudioEngine* m_audioEngine = nullptr;
    HRTFProcessor* m_hrtfProcessor = nullptr;

    // OpenVR overlay
    vr::VROverlayHandle_t m_overlayHandle = vr::k_ulOverlayHandleInvalid;
    vr::VROverlayHandle_t m_thumbnailHandle = vr::k_ulOverlayHandleInvalid;

    // Rendering
    unsigned int m_framebuffer = 0;
    unsigned int m_colorTexture = 0;
    unsigned int m_depthTexture = 0;
    int m_textureWidth = 1024;
    int m_textureHeight = 768;

    // Transform & positioning
    vr::HmdMatrix34_t m_overlayTransform;
    Vec3 m_position{0, 1.5f, -2.0f};  // Floating in front of user
    float m_scale = 1.0f;
    bool m_attachToController = false;

    // Controller tracking
    ControllerState m_leftController;
    ControllerState m_rightController;
    // Fallback for OpenVR stub compatibility
    static constexpr uint32_t k_unTrackedDeviceIndexInvalid = 0xFFFFFFFF;
    uint32_t m_leftControllerIndex = k_unTrackedDeviceIndexInvalid;
    uint32_t m_rightControllerIndex = k_unTrackedDeviceIndexInvalid;

    // Virtual microphone state
    VirtualMicrophone m_virtualMic;

    // Simple UI state
    struct UIState {
        bool showMicrophone = true;
        bool showControls = true;
        bool showLevelMeter = true;
        bool monitoring = false;  // Real-time audio monitoring

        // Auto-hide functionality for minimal interface
        bool autoHide = true;
        int autoHideDelayMs = 3000;  // Shorter for content creation focus
        std::chrono::steady_clock::time_point lastInteraction;
    } m_uiState;

    // === SIMPLE FUNCTIONALITY ===
    void HandleAutoHide();
    void UpdateAudioLevels();

    // Helper methods
    float CalculateDistance(const Vec3& a, const Vec3& b);
    bool IsControllerNearMicrophone(const Vec3& controllerPos, float tolerance = 0.2f);

    // Creative solution: New methods for VR recording controls
    bool IsControllerNearPosition(const Vec3& targetPos, float tolerance = 0.3f);
    void ToggleRecording();  // Start/stop recording toggle

    // Button state tracking for VR interaction
    bool m_recordButtonPressed = false;
    bool m_monitorButtonPressed = false;

    // Gesture callback for compatibility
    std::function<void(const std::string&, const Vec3&)> m_gestureCallback;

    // Simple debug info
    bool m_debugMode = false;
    void RenderDebugInfo();
};

} // namespace vrb

// Simple utility functions for ASMRtist workflow
namespace vrb::asmr_utils {
    /**
     * @brief Calculate optimal microphone positioning for spatial effect
     * @param userPos Current user/HMD position
     * @param distance Desired distance from user
     * @return Recommended microphone position
     */
    Vec3 CalculateOptimalMicPosition(const Vec3& userPos, float distance = 1.0f);

    /**
     * @brief Simple spatial audio preview without recording
     * @param micPos Virtual microphone position
     * @param userPos User head position
     * @return Spatial audio preview settings
     */
    struct SpatialPreview {
        float leftVolume;
        float rightVolume;
        float distance;
    };
    SpatialPreview CalculatePreview(const Vec3& micPos, const Vec3& userPos);
}