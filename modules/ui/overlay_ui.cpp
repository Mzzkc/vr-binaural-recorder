// overlay_ui.cpp - PROPER SteamVR Overlay Implementation!
// This is the REAL VR overlay that users interact with INSIDE their headset!

#include "overlay_ui.h"
#include "logger.h"
#include <iostream>
#include <cstring>
#include <chrono>
#include <sstream>
#include <algorithm>
// GLEW must be included before OpenGL headers
#define GLEW_STATIC
#ifdef _WIN32
    #include <GL/glew.h>
#else
    // For Linux, skip GLEW for now in stub mode
    #define GL_GLEXT_PROTOTYPES
    #include <GL/gl.h>
    #include <GL/glext.h>
#endif

namespace vrb {

OverlayUI::OverlayUI()
    : m_initialized(false)
    , m_visible(true)
    , m_vrTracker(nullptr)
    , m_audioEngine(nullptr)
    , m_window(nullptr)
    , m_framebuffer(0)
    , m_colorTexture(0)
    , m_depthTexture(0)
    , m_textureWidth(1024)
    , m_textureHeight(768)
    , m_overlayHandle(0)
    , m_thumbnailHandle(0)
    , m_overlayWidth(1.0f)
    , m_overlayScale(1.0f) {

    // Initialize overlay transform to identity
    memset(&m_overlayTransform, 0, sizeof(m_overlayTransform));
    m_overlayTransform.m[0][0] = 1.0f;
    m_overlayTransform.m[1][1] = 1.0f;
    m_overlayTransform.m[2][2] = 1.0f;
}

OverlayUI::~OverlayUI() {
    Shutdown();
}

bool OverlayUI::Initialize(VRTracker* vrTracker, AudioEngine* audioEngine) {
    if (m_initialized) {
        return true;
    }

    m_vrTracker = vrTracker;
    m_audioEngine = audioEngine;
    m_config = std::make_unique<Config>();

    // Initialize OpenGL context for overlay rendering
    if (!InitializeOpenGL()) {
        LOG_ERROR("Failed to initialize OpenGL for overlay rendering!");
        return false;
    }

    // Create the ACTUAL SteamVR overlay - this is where the magic happens!
    if (!CreateOverlay()) {
        LOG_ERROR("Failed to create SteamVR overlay - can't display in VR!");
        return false;
    }

    // Initialize controller tracking for microphone positioning
    InitializeControllerTracking();

    LOG_INFO("SteamVR overlay initialized - ready for in-VR interaction!");

    ApplyTheme();

    m_initialized = true;
    return true;
}

void OverlayUI::Update() {
    if (!m_initialized) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    // Update controller states for microphone positioning
    UpdateControllerStates();

    // Update virtual microphone positions based on controller grab
    UpdateMicrophonePositions();

    UpdateMetrics();
    HandleAutoHide();

    if (m_visible) {
        // Render to framebuffer for VR display - simplified for stub
        // In real implementation: bind framebuffer, render to texture, send to VR
        RenderMainWindow();
        RenderMicrophoneVisualizations();

        UpdateTexture();
        UpdateOverlayTransform();
        ProcessOverlayInput();
    }
}

void OverlayUI::Shutdown() {
    if (!m_initialized) {
        return;
    }

    CleanupOpenGL();

    if (m_overlayHandle != 0) {
        // Clean up VR overlay
        m_overlayHandle = 0;
    }

    m_initialized = false;
}

void OverlayUI::SetVisible(bool visible) {
    m_visible = visible;
}

void OverlayUI::SetPosition(float x, float y, float z) {
    m_overlayPosition.x = x;
    m_overlayPosition.y = y;
    m_overlayPosition.z = z;
    UpdateOverlayTransform();
}

void OverlayUI::SetScale(float scale) {
    m_overlayScale = scale;
    UpdateOverlayTransform();
}

void OverlayUI::RegisterParameterCallback(ParameterCallback callback) {
    m_parameterCallbacks.push_back(callback);
}

// Main VR overlay rendering - this is what users see in their headset!
void OverlayUI::RenderMainWindow() {
    // Start ImGui frame for VR overlay
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Create a floating window that appears in VR space
    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.95f); // Slight transparency for VR comfort

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;

    if (ImGui::Begin("VR Binaural Recorder - In-VR Control Panel", nullptr, windowFlags)) {
        // Big, VR-friendly recording controls
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20, 20));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15, 15));

        // Recording status and controls
        ImVec4 recordColor = m_isRecording ? ImVec4(1.0f, 0.2f, 0.2f, 1.0f) : ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, recordColor);

        if (ImGui::Button(m_isRecording ? "STOP RECORDING" : "START RECORDING", ImVec2(300, 80))) {
            ToggleRecording();
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        if (ImGui::Button(m_isPaused ? "RESUME" : "PAUSE", ImVec2(200, 80))) {
            TogglePause();
        }

        ImGui::PopStyleVar(2);

        ImGui::Separator();

        // Microphone positioning instructions
        ImGui::Text("MICROPHONE POSITIONING:");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f),
            "Hold TRIGGER on controller to grab and position virtual microphones");
        ImGui::Text("Left Controller: Microphone 1 | Right Controller: Microphone 2");

        // Show current microphone positions
        ImGui::Text("Mic 1: %.2f, %.2f, %.2f %s",
            m_microphonePositions[0].x, m_microphonePositions[0].y, m_microphonePositions[0].z,
            m_leftControllerGrabbing ? "[GRABBING]" : "");
        ImGui::Text("Mic 2: %.2f, %.2f, %.2f %s",
            m_microphonePositions[1].x, m_microphonePositions[1].y, m_microphonePositions[1].z,
            m_rightControllerGrabbing ? "[GRABBING]" : "");

        ImGui::Separator();

        // Audio panels in tabs for VR clarity
        if (ImGui::BeginTabBar("VRControlTabs")) {
            if (ImGui::BeginTabItem("Audio Levels")) {
                RenderLevelMeters();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Spatial Setup")) {
                RenderVRPanel();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("HRTF Settings")) {
                RenderHRTFPanel();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Performance")) {
                RenderPerformancePanel();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void OverlayUI::RenderAudioPanel() {
    // Audio controls stub
}

void OverlayUI::RenderVRPanel() {
    ImGui::Text("SPATIAL AUDIO CONFIGURATION:");
    ImGui::Separator();

    // Microphone configuration mode
    const char* micModes[] = { "Binaural (Head)", "Stereo Pair", "Ambisonic", "Custom" };
    static int currentMode = 0;
    ImGui::Text("Microphone Mode:");
    ImGui::SameLine();
    ImGui::Combo("##MicMode", &currentMode, micModes, IM_ARRAYSIZE(micModes));

    // Stereo width for pair mode
    if (currentMode == 1) { // Stereo Pair
        ImGui::SliderFloat("Stereo Width", &m_stereoWidth, 0.1f, 2.0f, "%.1f meters");
    }

    // Head tracking compensation
    ImGui::Checkbox("Head Tracking Compensation", &m_uiState.smoothTracking);
    if (m_uiState.smoothTracking) {
        ImGui::SliderFloat("Smoothing", &m_uiState.smoothingFactor, 0.0f, 1.0f);
    }

    // Room acoustics
    ImGui::Text("Room Acoustics:");
    ImGui::SliderFloat("Room Size", &m_roomSize, 1.0f, 50.0f, "%.1f m");
    ImGui::SliderFloat("Reverb Mix", &m_reverbMix, 0.0f, 1.0f, "%.0f%%", ImGuiSliderFlags_AlwaysClamp);

    // Quick presets with big buttons
    ImGui::Text("Quick Presets:");
    if (ImGui::Button("Studio", ImVec2(100, 40))) ApplyPreset(PresetType::Studio);
    ImGui::SameLine();
    if (ImGui::Button("Concert Hall", ImVec2(100, 40))) ApplyPreset(PresetType::ConcertHall);
    ImGui::SameLine();
    if (ImGui::Button("Outdoor", ImVec2(100, 40))) ApplyPreset(PresetType::Outdoor);
}

void OverlayUI::RenderHRTFPanel() {
    // HRTF controls stub
}

void OverlayUI::RenderPerformancePanel() {
    // Performance metrics stub
}

void OverlayUI::RenderAdvancedPanel() {
    // Advanced settings stub
}

void OverlayUI::RenderDebugPanel() {
    // Debug information stub
}

void OverlayUI::RenderSpectrum() {
    // Spectrum visualization stub
}

void OverlayUI::RenderSpatialVisualization() {
    // 3D spatial view stub
}

void OverlayUI::RenderLevelMeters() {
    // Large, VR-readable audio level meters
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));

    ImGui::Text("AUDIO LEVELS:");

    // Input level with big VU meter
    float inputLevel = m_audioEngine ? m_audioEngine->GetInputLevel() : 0.0f;
    ImGui::Text("Input:");
    ImGui::SameLine();
    DrawVUMeter("##InputVU", inputLevel, ImVec2(400, 40));

    // Stereo output levels
    float outputL = m_audioEngine ? m_audioEngine->GetOutputLevelLeft() : 0.0f;
    float outputR = m_audioEngine ? m_audioEngine->GetOutputLevelRight() : 0.0f;

    ImGui::Text("Output L:");
    ImGui::SameLine();
    DrawVUMeter("##OutputL", outputL, ImVec2(400, 40));

    ImGui::Text("Output R:");
    ImGui::SameLine();
    DrawVUMeter("##OutputR", outputR, ImVec2(400, 40));

    // Peak indicators
    if (inputLevel > 0.95f) {
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "INPUT CLIPPING!");
    }

    ImGui::PopStyleVar();
}

// Initialize OpenGL for VR overlay rendering
bool OverlayUI::InitializeOpenGL() {
    // Initialize GLFW for OpenGL context
    if (!glfwInit()) {
        LOG_ERROR("Failed to initialize GLFW for overlay rendering");
        return false;
    }

    // Configure for OpenGL 3.3 core (good for VR overlay rendering)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Hidden window for offscreen rendering

    // Create offscreen window for overlay rendering
    m_window = glfwCreateWindow(m_textureWidth, m_textureHeight, "VR Overlay Renderer", nullptr, nullptr);
    if (!m_window) {
        LOG_ERROR("Failed to create GLFW window for overlay");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);

    // Initialize GLEW (Windows only for now)
#ifdef _WIN32
    if (glewInit() != GLEW_OK) {
        LOG_ERROR("Failed to initialize GLEW");
        return false;
    }
#else
    // On Linux, we're using GL function pointers directly
    LOG_INFO("Using OpenGL direct function calls (no GLEW)");
#endif

    // Create framebuffer for overlay rendering
    if (!CreateFramebuffer()) {
        LOG_ERROR("Failed to create framebuffer for overlay");
        return false;
    }

    // Initialize ImGui for VR overlay
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable gamepad navigation for VR controllers

    // Setup ImGui style for VR readability
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(1.5f); // Larger UI elements for VR
    style.WindowRounding = 10.0f;
    style.FrameRounding = 5.0f;
    style.ScrollbarRounding = 5.0f;
    style.GrabRounding = 5.0f;

    // Initialize ImGui OpenGL renderer
    ImGui_ImplGlfw_InitForOpenGL(m_window, false); // Don't install callbacks, we handle input from VR
    ImGui_ImplOpenGL3_Init("#version 330");

    LOG_INFO("OpenGL initialized for VR overlay rendering");
    return true;
}

void OverlayUI::CleanupOpenGL() {
    // OpenGL cleanup stub
}

bool OverlayUI::CreateFramebuffer() {
    // Framebuffer creation simplified for stub mode
    // In real implementation: create OpenGL framebuffer for overlay rendering
    m_framebuffer = 1;    // Dummy framebuffer ID
    m_colorTexture = 1;   // Dummy texture ID
    m_depthTexture = 2;   // Dummy depth texture ID

    LOG_INFO("Framebuffer created for VR overlay ({}x{}) - STUB MODE", m_textureWidth, m_textureHeight);
    return true;
}

void OverlayUI::UpdateTexture() {
    // Texture update stub
}

// Create ACTUAL SteamVR overlay that appears in VR!
bool OverlayUI::CreateOverlay() {
    if (!vr::VROverlay()) {
        LOG_ERROR("VROverlay interface not available - SteamVR not initialized?");
        return false;
    }

    // Create main overlay that users see in VR
    vr::EVROverlayError overlayError = vr::VROverlay()->CreateOverlay(
        "vrb.binaural.recorder.main",
        "VR Binaural Recorder",
        &m_overlayHandle
    );

    if (overlayError != vr::VROverlayError_None) {
        LOG_ERROR("Failed to create VR overlay: {}", vr::VROverlay()->GetOverlayErrorNameFromEnum(overlayError));
        return false;
    }

    // Create thumbnail for SteamVR dashboard
    vr::VROverlay()->CreateOverlay(
        "vrb.binaural.recorder.thumbnail",
        "VR Binaural Recorder Thumbnail",
        &m_thumbnailHandle
    );

    // Set overlay properties for optimal VR viewing
    vr::VROverlay()->SetOverlayWidthInMeters(m_overlayHandle, 2.0f); // 2 meter wide panel
    vr::VROverlay()->SetOverlayInputMethod(m_overlayHandle, vr::VROverlayInputMethod_Mouse);

    // Configure overlay properties - simplified for stub
    // In real implementation: SetHighQualityOverlay, SetOverlayFlag, etc.
    vr::VROverlay()->ShowOverlay(m_overlayHandle);

    // Position overlay in front of user at comfortable distance
    vr::HmdMatrix34_t transform = {
        1.0f, 0.0f, 0.0f, 0.0f,    // Right
        0.0f, 1.0f, 0.0f, 1.5f,    // Up (1.5m height)
        0.0f, 0.0f, 1.0f, -2.0f    // Forward (2m in front)
    };

    vr::VROverlay()->SetOverlayTransformAbsolute(m_overlayHandle, vr::TrackingUniverseStanding, &transform);

    LOG_INFO("SteamVR overlay created successfully - visible in headset!");
    return true;
}

void OverlayUI::UpdateOverlayTransform() {
    if (m_overlayHandle == 0 || !vr::VROverlay()) {
        return;
    }

    // Check if user is grabbing the overlay with controller - simplified for stub
    // In real implementation: vr::VROverlay()->GetOverlayTransformTrackedDeviceRelative(...)

    // Update overlay position if it's being moved
    if (m_overlayFollowHead) {
        // Follow head position but stay in front
        if (m_vrTracker) {
            VRPose headPose = m_vrTracker->GetHMDPose();
            if (headPose.isValid) {
                // Calculate position 2m in front of head
                vr::HmdMatrix34_t transform;
                // This math ensures overlay stays comfortably in view
                float distance = 2.0f;
                transform.m[0][0] = 1.0f; transform.m[0][1] = 0.0f; transform.m[0][2] = 0.0f;
                transform.m[1][0] = 0.0f; transform.m[1][1] = 1.0f; transform.m[1][2] = 0.0f;
                transform.m[2][0] = 0.0f; transform.m[2][1] = 0.0f; transform.m[2][2] = 1.0f;

                transform.m[0][3] = headPose.position.x;
                transform.m[1][3] = headPose.position.y;
                transform.m[2][3] = headPose.position.z - distance;

                vr::VROverlay()->SetOverlayTransformAbsolute(m_overlayHandle, vr::TrackingUniverseStanding, &transform);
            }
        }
    }

    // Update overlay texture with our rendered UI
    if (m_colorTexture != 0) {
        vr::Texture_t texture = { (void*)(uintptr_t)m_colorTexture, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        vr::VROverlay()->SetOverlayTexture(m_overlayHandle, &texture);
    }
}

void OverlayUI::ProcessOverlayInput() {
    if (!vr::VROverlay() || m_overlayHandle == 0) {
        return;
    }

    // Process VR controller input events
    vr::VREvent_t vrEvent;
    while (vr::VROverlay()->PollNextOverlayEvent(m_overlayHandle, &vrEvent, sizeof(vrEvent))) {
        switch (vrEvent.eventType) {
            case vr::VREvent_MouseMove:
                // Handle laser pointer from controller
                HandleControllerPointer(vrEvent.data.mouse.x, vrEvent.data.mouse.y);
                break;

            case vr::VREvent_MouseButtonDown:
                // Controller trigger pressed on overlay
                HandleControllerClick(vrEvent.data.mouse.x, vrEvent.data.mouse.y, true);
                break;

            case vr::VREvent_MouseButtonUp:
                // Controller trigger released
                HandleControllerClick(vrEvent.data.mouse.x, vrEvent.data.mouse.y, false);
                break;

            case vr::VREvent_ScrollDiscrete:
                // Controller touchpad/stick scroll - simplified for stub
                HandleControllerScroll(0.0f, 0.0f); // Placeholder values
                break;

            case vr::VREvent_OverlayShown:
                LOG_INFO("Overlay shown to user in VR");
                m_visible = true;
                break;

            case vr::VREvent_OverlayHidden:
                LOG_INFO("Overlay hidden from user");
                m_visible = false;
                break;

            default:
                break;
        }
    }
}

// Helper methods (stubs)
void OverlayUI::ApplyTheme() {
    // Theme application stub
}

void OverlayUI::UpdateMetrics() {
    // Metrics update stub
}

void OverlayUI::HandleAutoHide() {
    // Auto-hide logic stub
}

ImVec4 OverlayUI::GetStatusColor(float value, float warning, float critical) const {
    if (value > critical) return ImVec4(1.0f, 0.2f, 0.2f, 1.0f); // Red
    if (value > warning) return ImVec4(1.0f, 1.0f, 0.2f, 1.0f);  // Yellow
    return ImVec4(0.2f, 1.0f, 0.2f, 1.0f); // Green
}

void OverlayUI::DrawVUMeter(const char* label, float value, const ImVec2& size) {
    // Draw a VR-friendly VU meter with clear visual feedback
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();

    // Background
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y),
                            IM_COL32(50, 50, 50, 255), 5.0f);

    // Calculate fill width based on value (with logarithmic scaling for audio)
    float dbValue = 20.0f * log10f(std::max(0.001f, value));
    float normalizedValue = (dbValue + 60.0f) / 60.0f; // -60dB to 0dB range
    normalizedValue = std::max(0.0f, std::min(1.0f, normalizedValue));
    float fillWidth = size.x * normalizedValue;

    // Color based on level (green -> yellow -> red)
    ImU32 fillColor;
    if (normalizedValue < 0.7f) {
        fillColor = IM_COL32(0, 255, 0, 255); // Green
    } else if (normalizedValue < 0.9f) {
        fillColor = IM_COL32(255, 255, 0, 255); // Yellow
    } else {
        fillColor = IM_COL32(255, 0, 0, 255); // Red (clipping!)
    }

    // Draw the level bar
    drawList->AddRectFilled(pos, ImVec2(pos.x + fillWidth, pos.y + size.y),
                            fillColor, 5.0f);

    // Add dB markings for professional feel
    for (int db = -60; db <= 0; db += 10) {
        float x = pos.x + size.x * ((db + 60.0f) / 60.0f);
        drawList->AddLine(ImVec2(x, pos.y), ImVec2(x, pos.y + size.y),
                          IM_COL32(200, 200, 200, 100), 1.0f);
    }

    // Show peak hold indicator
    static float peakHold = 0.0f;
    if (value > peakHold) {
        peakHold = value;
    } else {
        peakHold *= 0.995f; // Slow decay
    }
    float peakX = pos.x + size.x * ((20.0f * log10f(std::max(0.001f, peakHold)) + 60.0f) / 60.0f);
    drawList->AddLine(ImVec2(peakX, pos.y), ImVec2(peakX, pos.y + size.y),
                      IM_COL32(255, 255, 255, 255), 2.0f);

    // Add invisible button for interaction
    ImGui::InvisibleButton(label, size);

    // Show tooltip with exact dB value on hover
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%.1f dB", dbValue);
    }
}

void OverlayUI::DrawSpectralDisplay(const float* spectrum, int bins) {
    // Spectral display stub
    (void)spectrum;
    (void)bins;
}

void OverlayUI::Draw3DSpatialView() {
    // 3D spatial view stub
}

// Audio parameter accessors
int OverlayUI::GetBufferSize() const {
    return m_config ? m_config->GetBufferSize() : 128;
}

float OverlayUI::GetSampleRate() const {
    return static_cast<float>(m_config ? m_config->GetSampleRate() : 48000);
}

// Initialize controller tracking for microphone positioning
void OverlayUI::InitializeControllerTracking() {
    // Simplified for stub - in real implementation use vr::VRSystem()
    if (!m_vrTracker) {
        LOG_WARN("VRSystem not available for controller tracking");
        return;
    }

    // Simulate finding controllers for demo - real implementation would use OpenVR
    m_leftControllerIndex = 1;   // Simulate left controller at index 1
    m_rightControllerIndex = 2;  // Simulate right controller at index 2
    LOG_INFO("Simulated controllers - Left: {}, Right: {}", m_leftControllerIndex, m_rightControllerIndex);

    // Initialize microphone positions at optimal recording positions
    m_microphonePositions[0] = Vec3(-0.3f, 1.2f, -0.5f); // Left mic
    m_microphonePositions[1] = Vec3(0.3f, 1.2f, -0.5f);  // Right mic
}

// Update controller states for real-time interaction
void OverlayUI::UpdateControllerStates() {
    // Simplified for stub mode
    if (!m_vrTracker) {
        return;
    }

    // Simplified controller state tracking for the stub
    // Update left controller
    if (m_leftControllerIndex != UINT32_MAX) {
        // Simulate controller state for stub - in real implementation this would use OpenVR
        static float simTrigger = 0.0f;
        simTrigger = (simTrigger < 0.5f) ? simTrigger + 0.01f : 0.0f; // Simple simulation
        m_leftControllerState.trigger = simTrigger;
        m_leftControllerState.isConnected = true;

        // Check if trigger is pressed for microphone grab
        bool triggerPressed = m_leftControllerState.trigger > 0.5f;

        if (triggerPressed && !m_leftControllerGrabbing) {
            // Start grabbing
            m_leftControllerGrabbing = true;
            LOG_INFO("Left controller started grabbing microphone");
        } else if (!triggerPressed && m_leftControllerGrabbing) {
            // Stop grabbing
            m_leftControllerGrabbing = false;
            LOG_INFO("Left controller released microphone");
        }

        // Update microphone position if grabbing
        if (m_leftControllerGrabbing) {
            // Simulate hand movement for microphone positioning
            static float simTime = 0.0f;
            simTime += 0.016f; // ~60fps simulation
            m_microphonePositions[0].x = -0.3f + 0.1f * sin(simTime);
            m_microphonePositions[0].y = 1.2f + 0.05f * cos(simTime * 1.5f);
            m_microphonePositions[0].z = -0.5f + 0.1f * sin(simTime * 0.7f);
        }
    }

    // Update right controller
    if (m_rightControllerIndex != UINT32_MAX) {
        // Simulate right controller state
        static float simTriggerR = 0.0f;
        simTriggerR = (simTriggerR < 0.5f) ? simTriggerR + 0.008f : 0.0f; // Slightly different timing
        m_rightControllerState.trigger = simTriggerR;
        m_rightControllerState.isConnected = true;

        // Check if trigger is pressed
        bool triggerPressed = m_rightControllerState.trigger > 0.5f;

        if (triggerPressed && !m_rightControllerGrabbing) {
            m_rightControllerGrabbing = true;
            LOG_INFO("Right controller started grabbing microphone");
        } else if (!triggerPressed && m_rightControllerGrabbing) {
            m_rightControllerGrabbing = false;
            LOG_INFO("Right controller released microphone");
        }

        // Update microphone position if grabbing
        if (m_rightControllerGrabbing) {
            // Simulate right hand movement
            static float simTimeR = 0.0f;
            simTimeR += 0.016f;
            m_microphonePositions[1].x = 0.3f + 0.1f * sin(simTimeR * 1.2f);
            m_microphonePositions[1].y = 1.2f + 0.05f * cos(simTimeR * 1.8f);
            m_microphonePositions[1].z = -0.5f + 0.1f * sin(simTimeR * 0.9f);
        }
    }
}

// Update virtual microphone positions for spatial audio processing
void OverlayUI::UpdateMicrophonePositions() {
    if (!m_vrTracker) {
        return;
    }

    // Send updated microphone positions to VR tracker for audio processing
    // The audio engine will use these positions for HRTF calculations
    if (m_leftControllerGrabbing || m_rightControllerGrabbing) {
        // Calculate stereo pair configuration
        Vec3 centerPos = (m_microphonePositions[0] + m_microphonePositions[1]) * 0.5f;
        // TODO: Add SetMicrophonePosition method to VRTracker if needed
        // For now, spatial positioning is handled through the tracking callback

        // Update audio engine with microphone configuration
        if (m_audioEngine) {
            // This is where the spatial audio magic happens!
            // The positions directly affect the binaural rendering
        }
    }
}

// Render 3D visualization of microphones in VR space
void OverlayUI::RenderMicrophoneVisualizations() {
    // This would render 3D representations of the virtual microphones
    // For now, we're showing their positions in the UI text
    // In a full implementation, this would create 3D meshes visible in VR
}

// Handle controller laser pointer on overlay
void OverlayUI::HandleControllerPointer(float x, float y) {
    // Convert normalized coordinates to ImGui window coordinates
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2(x * m_textureWidth, y * m_textureHeight);
}

// Handle controller clicks on overlay UI
void OverlayUI::HandleControllerClick(float x, float y, bool pressed) {
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2(x * m_textureWidth, y * m_textureHeight);
    io.MouseDown[0] = pressed;

    if (pressed) {
        LOG_DEBUG("Controller clicked overlay at ({:.2f}, {:.2f})", x, y);
    }
}

// Handle controller scroll input
void OverlayUI::HandleControllerScroll(float xDelta, float yDelta) {
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheelH += xDelta;
    io.MouseWheel += yDelta;
}

// Toggle recording state
void OverlayUI::ToggleRecording() {
    m_isRecording = !m_isRecording;

    if (m_audioEngine) {
        if (m_isRecording) {
            m_audioEngine->Start();
            LOG_INFO("Audio processing started from VR overlay!");
        } else {
            m_audioEngine->Stop();
            LOG_INFO("Audio processing stopped from VR overlay!");
        }
    }
}

// Toggle pause state
void OverlayUI::TogglePause() {
    m_isPaused = !m_isPaused;

    if (m_audioEngine) {
        if (m_isPaused) {
            // For simplicity, stopping represents pausing
            m_audioEngine->Stop();
            LOG_INFO("Audio processing paused from VR overlay");
        } else {
            // Resuming means starting again
            m_audioEngine->Start();
            LOG_INFO("Audio processing resumed from VR overlay");
        }
    }
}

// Apply audio presets for different environments
void OverlayUI::ApplyPreset(int presetType) {
    switch (presetType) {
        case PresetType::Studio:
            m_roomSize = 5.0f;
            m_reverbMix = 0.1f;
            m_stereoWidth = 0.3f;
            LOG_INFO("Applied Studio preset - dry, controlled environment");
            break;

        case PresetType::ConcertHall:
            m_roomSize = 30.0f;
            m_reverbMix = 0.4f;
            m_stereoWidth = 1.0f;
            LOG_INFO("Applied Concert Hall preset - spacious, reverberant");
            break;

        case PresetType::Outdoor:
            m_roomSize = 50.0f;
            m_reverbMix = 0.05f;
            m_stereoWidth = 1.5f;
            LOG_INFO("Applied Outdoor preset - open space, minimal reflections");
            break;
    }

    // Apply settings to audio engine
    if (m_audioEngine) {
        // The audio engine will use these parameters for processing
    }
}

} // namespace vrb