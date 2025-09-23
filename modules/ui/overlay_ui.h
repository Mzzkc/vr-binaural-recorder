// overlay_ui.h - VR Overlay UI using ImGui
#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <openvr.h>
#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include <functional>
#include "vr_tracker.h"
#include "audio_engine.h"
#include "config.h"
#include "vr_types.h"

namespace vrb {

/**
 * @brief VR Overlay UI for real-time control and monitoring
 *
 * Provides an ImGui-based interface rendered as a VR overlay
 * for adjusting parameters and monitoring system status.
 */
class OverlayUI {
public:
    OverlayUI();
    ~OverlayUI();

    /**
     * @brief Initialize overlay UI system
     * @param vrTracker Pointer to VR tracker for position updates
     * @param audioEngine Pointer to audio engine for control
     * @return true if initialization successful
     */
    bool Initialize(VRTracker* vrTracker, AudioEngine* audioEngine);

    /**
     * @brief Update overlay UI (render and process input)
     */
    void Update();

    /**
     * @brief Shutdown overlay UI
     */
    void Shutdown();

    /**
     * @brief Show/hide overlay
     */
    void SetVisible(bool visible);

    /**
     * @brief Check if overlay is visible
     */
    bool IsVisible() const { return m_visible; }

    /**
     * @brief Set overlay position in VR space
     */
    void SetPosition(float x, float y, float z);

    /**
     * @brief Set overlay scale
     */
    void SetScale(float scale);

    /**
     * @brief Register callback for parameter changes
     */
    using ParameterCallback = std::function<void(const std::string& param, float value)>;
    void RegisterParameterCallback(ParameterCallback callback);

private:
    // UI rendering methods - enhanced for complete recording interface!
    void RenderMainWindow();
    void RenderRecordingControlPanel();    // NEW! Main recording controls
    void RenderSessionStatusPanel();       // NEW! Recording session info
    void RenderAudioMonitoringPanel();     // Enhanced audio monitoring
    void RenderTakeManagerPanel();         // NEW! Take management
    void RenderSettingsPanel();            // Enhanced settings
    void RenderFileManagementPanel();      // NEW! File naming and storage
    void RenderAudioPanel();
    void RenderVRPanel();
    void RenderHRTFPanel();
    void RenderPerformancePanel();
    void RenderAdvancedPanel();
    void RenderDebugPanel();
    void RenderSpectrum();
    void RenderSpatialVisualization();
    void RenderLevelMeters();

    // Recording workflow methods
    void StartNewRecordingSession();
    void StopRecordingSession();
    void PauseRecordingSession();
    void ResumeRecordingSession();
    void StartNewTake();
    void ReviewLastTake();
    void DeleteCurrentTake();
    void ApplyRecordingPreset(int presetIndex);

    // File management
    void UpdateStorageInfo();
    void GenerateRecordingFilename();
    bool CheckDiskSpace();
    void CleanupOldRecordings();

    // Audio monitoring
    void UpdateAudioMetrics();
    void CheckForClipping();
    void CalculateSignalToNoise();
    void UpdateFileSpacing();

    // OpenGL setup
    bool InitializeOpenGL();
    void CleanupOpenGL();
    bool CreateFramebuffer();
    void UpdateTexture();

    // VR overlay management
    bool CreateOverlay();
    void UpdateOverlayTransform();
    void ProcessOverlayInput();

    // Helper methods
    void ApplyTheme();
    void UpdateMetrics();
    void HandleAutoHide();
    ImVec4 GetStatusColor(float value, float warning, float critical) const;
    void DrawVUMeter(const char* label, float value, const ImVec2& size);
    void DrawSpectralDisplay(const float* spectrum, int bins);
    void Draw3DSpatialView();

    // Audio parameter accessors
    int GetBufferSize() const;
    float GetSampleRate() const;

private:
    // State
    std::atomic<bool> m_initialized{false};
    std::atomic<bool> m_visible{true};
    mutable std::mutex m_mutex;

    // Components
    VRTracker* m_vrTracker;
    AudioEngine* m_audioEngine;
    std::unique_ptr<Config> m_config;

    // OpenGL resources
    GLFWwindow* m_window;
    GLuint m_framebuffer;
    GLuint m_colorTexture;
    GLuint m_depthTexture;
    int m_textureWidth;
    int m_textureHeight;

    // VR overlay
    vr::VROverlayHandle_t m_overlayHandle;
    vr::VROverlayHandle_t m_thumbnailHandle;
    vr::HmdMatrix34_t m_overlayTransform;
    float m_overlayWidth;
    float m_overlayScale;
    Vec3 m_overlayPosition;

    // UI state
    struct UIState {
        // Panels
        bool showAudioPanel = true;
        bool showVRPanel = true;
        bool showHRTFPanel = true;
        bool showPerformance = false;
        bool showAdvanced = false;
        bool showDebug = false;

        // Audio controls
        float inputGain = 1.0f;
        float outputGain = 1.0f;
        int selectedInput = 0;
        std::vector<std::string> inputDevices;
        bool muteInput = false;
        bool muteOutput = false;
        bool bypass = false;

        // VR controls
        float micDistance = 1.0f;
        float micHeight = 1.2f;
        float micAngle = 0.0f;
        bool lockMicPosition = false;
        bool showChaperone = true;
        bool smoothTracking = true;
        float smoothingFactor = 0.95f;

        // HRTF controls
        bool enableHRTF = true;
        int hrtfPreset = 0;
        float distanceAttenuation = 1.0f;
        bool nearFieldCompensation = true;
        float crossfeedLevel = 0.0f;

        // Visualization
        bool showSpectrum = false;
        bool show3DView = false;
        bool showMeters = true;
        std::vector<float> spectrumData;
        float inputLevel = 0.0f;
        float outputLevelL = 0.0f;
        float outputLevelR = 0.0f;

        // Performance
        float cpuUsage = 0.0f;
        float audioLatency = 0.0f;
        int bufferUnderruns = 0;
        float trackingRate = 0.0f;

        // Auto-hide
        bool autoHide = true;
        int autoHideDelay = 5000;
        std::chrono::steady_clock::time_point lastInteraction;
    } m_uiState;

    // Callbacks
    std::vector<ParameterCallback> m_parameterCallbacks;

    // Controller tracking for microphone positioning
    void InitializeControllerTracking();
    void UpdateControllerStates();
    void UpdateMicrophonePositions();
    void RenderMicrophoneVisualizations();
    void HandleControllerPointer(float x, float y);
    void HandleControllerClick(float x, float y, bool pressed);
    void HandleControllerScroll(float xDelta, float yDelta);
    void ToggleRecording();
    void TogglePause();
    void ApplyPreset(int presetType);

    // Controller state tracking
    uint32_t m_leftControllerIndex = UINT32_MAX;  // Invalid device index
    uint32_t m_rightControllerIndex = UINT32_MAX; // Invalid device index
    struct ControllerState {
        float trigger = 0.0f;
        bool isConnected = false;
    } m_leftControllerState, m_rightControllerState;
    bool m_leftControllerGrabbing = false;
    bool m_rightControllerGrabbing = false;
    Vec3 m_microphonePositions[2]; // Position of virtual microphones

    // Recording state - enhanced for complete session management!
    bool m_isRecording = false;
    bool m_isPaused = false;

    // Recording session management
    struct RecordingSession {
        std::chrono::steady_clock::time_point startTime;
        std::chrono::steady_clock::time_point pauseStart;
        std::chrono::duration<float> totalPausedTime{0};
        int currentTake = 1;
        std::string baseName = "VR_Recording";
        std::string outputDirectory = "./recordings/";
        bool autoIncrement = true;

        // File format settings
        int sampleRate = 48000;
        int bitDepth = 24;
        enum class Format { WAV, FLAC, OGG } format = Format::WAV;

        // Quality monitoring
        float peakInputLevel = 0.0f;
        float peakOutputLevelL = 0.0f;
        float peakOutputLevelR = 0.0f;
        bool hasClipped = false;
        float signalToNoiseRatio = 0.0f;

        // Storage management
        uint64_t estimatedFileSize = 0;
        uint64_t availableSpace = 0;
        bool lowSpaceWarning = false;
    } m_recordingSession;

    // Take management
    std::vector<std::string> m_recordedTakes;
    std::string m_lastRecordingPath;
    bool m_showTakeManager = false;

    // Quick playback for review
    bool m_isPlayingback = false;
    float m_playbackPosition = 0.0f;

    // Recording presets - different settings for different scenarios
    struct RecordingPreset {
        std::string name;
        int sampleRate;
        int bitDepth;
        RecordingSession::Format format;
        float roomSize;
        float reverbMix;
        bool enableNoisereduction;
        bool enableCompression;
    };

    std::vector<RecordingPreset> m_recordingPresets = {
        {"Studio Quality", 48000, 24, RecordingSession::Format::WAV, 5.0f, 0.1f, false, false},
        {"Live Performance", 44100, 16, RecordingSession::Format::FLAC, 20.0f, 0.3f, true, true},
        {"Podcast/Voice", 44100, 16, RecordingSession::Format::OGG, 8.0f, 0.15f, true, true},
        {"High Quality Archive", 96000, 32, RecordingSession::Format::WAV, 10.0f, 0.2f, false, false}
    };
    int m_selectedPreset = 0;

    // Spatial audio parameters
    float m_stereoWidth = 0.5f;
    float m_roomSize = 10.0f;
    float m_reverbMix = 0.2f;
    bool m_overlayFollowHead = false;

    // Preset types
    enum PresetType {
        Studio = 0,
        ConcertHall = 1,
        Outdoor = 2
    };

    // Theme colors
    struct Theme {
        ImVec4 background;
        ImVec4 text;
        ImVec4 header;
        ImVec4 button;
        ImVec4 buttonHovered;
        ImVec4 buttonActive;
        ImVec4 slider;
        ImVec4 sliderGrab;
        ImVec4 checkMark;
        ImVec4 warning;
        ImVec4 error;
        ImVec4 success;
    } m_theme;
};

} // namespace vrb