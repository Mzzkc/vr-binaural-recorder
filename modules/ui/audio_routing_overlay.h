// audio_routing_overlay.h - Creative SteamVR Audio Routing Interface
// 🎧 THE AUDIO COCKPIT - Where sound becomes magic! 🎧
#pragma once

#include <openvr.h>
#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include <functional>
#include <map>
#include <chrono>
#include "vr_types.h"
#include "vr_tracker.h"
#include "audio_engine.h"
#include "hrtf_processor.h"

namespace vrb {

// Forward declarations
class HRTFProcessor;

/**
 * @brief Revolutionary SteamVR Audio Routing Interface
 *
 * This is NOT just another boring control panel!
 * This is a holographic audio playground where users:
 * - SEE their audio streams as flowing particles
 * - GRAB sound sources and route them intuitively
 * - CONDUCT their audio like a spatial orchestra
 * - TRANSFORM boring audio routing into a magical VR experience
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

    // Spatial positioning
    void SetPosition(const Vec3& position);
    void SetScale(float scale);
    void AttachToController(bool enabled);

    // Quick preset system - instant audio magic!
    enum class AudioPreset {
        CINEMA,     // Movie theater experience
        CONCERT,    // Live performance feel
        PODCAST,    // Intimate voice chat
        GAMING,     // Competitive gaming audio
        MUSIC       // High-fidelity listening
    };
    void ApplyPreset(AudioPreset preset);

    // Audio source management
    struct AudioSource {
        std::string name;
        float volume = 1.0f;
        Vec3 spatialPosition{0, 0, 0};
        bool enabled = true;
        bool isMuted = false;
        float* spectrumData = nullptr;  // Real-time spectrum visualization
        int spectrumSize = 0;

        // Visual representation
        Vec3 orbPosition{0, 0, 0};     // Position of the audio orb
        float orbSize = 0.1f;          // Size based on volume
        Vec3 orbColor{1, 1, 1};        // Color based on frequency content
        float orbPulse = 0.0f;         // Pulsing animation based on audio
    };

    void AddAudioSource(const std::string& name);
    void RemoveAudioSource(const std::string& name);
    AudioSource* GetAudioSource(const std::string& name);
    std::vector<AudioSource*> GetAllAudioSources();

    // Gesture callbacks - make audio routing FUN!
    using GestureCallback = std::function<void(const std::string& gesture, const Vec3& position)>;
    void RegisterGestureCallback(GestureCallback callback);

    // VR Integration - Direct callbacks from 90Hz VR tracking thread
    void UpdateGestureDetection(const std::vector<VRPose>& controllers);
    void UpdateAudioOrbPositions(const VRPose& hmdPose);

    // Audio Engine Integration - Real-time stats for orb visualization
    // TODO: Implement when AudioEngine has SetStatsCallback method
    // void OnAudioStatsUpdate(const AudioStats& stats);
    void SetHRTFProcessor(HRTFProcessor* processor);

private:
    // === CORE VR OVERLAY MAGIC ===
    bool CreateOverlay();
    void DestroyOverlay();
    bool CreateRenderTarget();
    void DestroyRenderTarget();

    void RenderToTexture();
    void UpdateOverlayTexture();
    void UpdateOverlayTransform();

    // === GESTURE RECOGNITION SYSTEM ===
    struct ControllerState {
        Vec3 position{0, 0, 0};
        Vec3 velocity{0, 0, 0};
        float trigger = 0.0f;
        bool isGripping = false;
        bool wasGripping = false;
        AudioSource* grabbedSource = nullptr;

        // Gesture detection
        std::chrono::steady_clock::time_point lastUpdate;
        std::vector<Vec3> positionHistory;  // For swipe detection
        float twistAngle = 0.0f;            // For parameter adjustment
    };

    void UpdateControllerStates();
    void ProcessGestures();
    void DetectPinchGesture(ControllerState& controller);
    void DetectSwipeGesture(ControllerState& controller);
    void DetectTwistGesture(ControllerState& controller);
    void DetectClapGesture();  // Two-handed proximity

    // === HOLOGRAPHIC UI RENDERING ===
    void RenderAudioCockpit();
    void RenderAudioOrbs();
    void RenderParticleStreams();
    void RenderSpatialGrid();
    void RenderQuickActionRing();
    void RenderHRTFVisualizer();

    // Audio orb interaction
    void UpdateAudioOrbs();
    AudioSource* GetOrbAtPosition(const Vec3& position, float tolerance = 0.1f);
    void HandleOrbGrab(AudioSource* source, const Vec3& controllerPos);
    void HandleOrbRelease(AudioSource* source);

    // === PARTICLE SYSTEM - AUDIO VISUALIZATION ===
    struct AudioParticle {
        Vec3 position;
        Vec3 velocity;
        float lifetime;
        float intensity;
        Vec3 color;
        AudioSource* sourceOrb = nullptr;

        void Update(float deltaTime);
        bool IsAlive() const { return lifetime > 0.0f; }
    };

    std::vector<AudioParticle> m_particles;
    void UpdateParticleSystem();
    void EmitParticlesFromSource(AudioSource* source);
    void RenderParticles();

    // === CREATIVE SHORTCUTS & HACKS ===
    // Global audio buffer for instant visualization access
    // TODO: Properly thread this when we optimize!
    static constexpr size_t VISUALIZATION_BUFFER_SIZE = 1024;
    float m_globalAudioBuffer[VISUALIZATION_BUFFER_SIZE];
    std::atomic<bool> m_bufferReady{false};

    // Pre-calculated HRTF presets for instant switching
    struct HRTFPresetData {
        float roomSize;
        float reverbMix;
        float distanceAttenuation;
        bool nearFieldCompensation;
        std::string displayName;
    };
    std::map<AudioPreset, HRTFPresetData> m_hrtfPresets;

    // Ring buffer for lock-free audio stats
    template<typename T, size_t SIZE>
    class LockFreeRing {
    public:
        void Push(const T& item) {
            size_t idx = m_writeIndex.fetch_add(1) % SIZE;
            m_buffer[idx] = item;
        }

        bool Pop(T& item) {
            size_t writeIdx = m_writeIndex.load();
            size_t readIdx = m_readIndex.load();
            if (readIdx == writeIdx) return false;

            item = m_buffer[readIdx % SIZE];
            m_readIndex.store((readIdx + 1) % SIZE);
            return true;
        }

    private:
        std::atomic<size_t> m_writeIndex{0};
        std::atomic<size_t> m_readIndex{0};
        T m_buffer[SIZE];
    };

    // TODO: Enable when AudioEngine stats callback is available
    // LockFreeRing<AudioStats, 64> m_audioStatsRing;

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

    // Audio sources
    std::map<std::string, std::unique_ptr<AudioSource>> m_audioSources;

    // UI state
    struct UIState {
        bool showParticles = true;
        bool showSpatialGrid = true;
        bool showQuickRing = true;
        bool showHRTFVisualizer = true;

        float particleIntensity = 1.0f;
        float gridOpacity = 0.3f;
        AudioPreset currentPreset = AudioPreset::GAMING;

        // Animation timers
        float pulseTimer = 0.0f;
        float rotationSpeed = 1.0f;

        // Auto-hide functionality
        bool autoHide = true;
        int autoHideDelayMs = 5000;
        std::chrono::steady_clock::time_point lastInteraction;
    } m_uiState;

    // Callbacks
    std::vector<GestureCallback> m_gestureCallbacks;

    // === PERFORMANCE OPTIMIZATION ===
    void InitializePresets();
    void UpdateMetrics();
    void HandleAutoHide();
    void SyncToCompositor();  // Piggyback on VR compositor timing

    // Audio integration
    void OnAudioCallback(const float* inputBuffer, size_t frames);
    void UpdateAudioVisualization();

    // Helper methods
    // Using Vec3 instead of VRControllerState_t for stub compatibility
    Vec3 ControllerToWorldSpace(const Vec3& position, uint32_t deviceIndex);
    float CalculateDistance(const Vec3& a, const Vec3& b);
    Vec3 HSVToRGB(float h, float s, float v);  // For spectrum coloring
    float SmoothStep(float edge0, float edge1, float x);  // Smooth animations

    // Debug visualization
    bool m_debugMode = false;
    void RenderDebugInfo();
    void ToggleDebugMode() { m_debugMode = !m_debugMode; }
};

} // namespace vrb