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
    // UI rendering methods
    void RenderMainWindow();
    void RenderAudioPanel();
    void RenderVRPanel();
    void RenderHRTFPanel();
    void RenderPerformancePanel();
    void RenderAdvancedPanel();
    void RenderDebugPanel();
    void RenderSpectrum();
    void RenderSpatialVisualization();
    void RenderLevelMeters();

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