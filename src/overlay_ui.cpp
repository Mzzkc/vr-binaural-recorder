// overlay_ui.cpp - VR Overlay UI Implementation
#include "overlay_ui.h"
#include "logger.h"
#include <GL/glew.h>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace vrb {

// Constants
constexpr float OVERLAY_WIDTH = 2.0f;
constexpr float OVERLAY_HEIGHT = 1.5f;
constexpr int TEXTURE_WIDTH = 1920;
constexpr int TEXTURE_HEIGHT = 1080;
constexpr float PI = 3.14159265358979323846f;

OverlayUI::OverlayUI()
    : m_vrTracker(nullptr)
    , m_audioEngine(nullptr)
    , m_window(nullptr)
    , m_framebuffer(0)
    , m_colorTexture(0)
    , m_depthTexture(0)
    , m_textureWidth(TEXTURE_WIDTH)
    , m_textureHeight(TEXTURE_HEIGHT)
    , m_overlayHandle(vr::k_ulOverlayHandleInvalid)
    , m_thumbnailHandle(vr::k_ulOverlayHandleInvalid)
    , m_overlayWidth(OVERLAY_WIDTH)
    , m_overlayScale(1.0f)
    , m_overlayPosition(0.0f, 1.5f, -1.0f) {

    // Initialize transform matrix
    memset(&m_overlayTransform, 0, sizeof(m_overlayTransform));
    m_overlayTransform.m[0][0] = 1.0f;
    m_overlayTransform.m[1][1] = 1.0f;
    m_overlayTransform.m[2][2] = 1.0f;

    // Set up theme colors (dark theme)
    m_theme.background = ImVec4(0.10f, 0.10f, 0.12f, 0.95f);
    m_theme.text = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    m_theme.header = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
    m_theme.button = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    m_theme.buttonHovered = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    m_theme.buttonActive = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    m_theme.slider = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    m_theme.sliderGrab = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    m_theme.checkMark = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    m_theme.warning = ImVec4(1.00f, 0.70f, 0.00f, 1.00f);
    m_theme.error = ImVec4(1.00f, 0.30f, 0.30f, 1.00f);
    m_theme.success = ImVec4(0.30f, 1.00f, 0.30f, 1.00f);
}

OverlayUI::~OverlayUI() {
    Shutdown();
}

bool OverlayUI::Initialize(VRTracker* vrTracker, AudioEngine* audioEngine) {
    if (m_initialized) {
        LOG_WARN("Overlay UI already initialized");
        return true;
    }

    m_vrTracker = vrTracker;
    m_audioEngine = audioEngine;

    // Initialize OpenGL context
    if (!InitializeOpenGL()) {
        LOG_ERROR("Failed to initialize OpenGL for overlay UI");
        return false;
    }

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr; // Disable imgui.ini file

    // Setup ImGui style
    ApplyTheme();

    // Setup Platform/Renderer backends
    if (!ImGui_ImplGlfw_InitForOpenGL(m_window, true)) {
        LOG_ERROR("Failed to initialize ImGui GLFW backend");
        return false;
    }

    if (!ImGui_ImplOpenGL3_Init("#version 130")) {
        LOG_ERROR("Failed to initialize ImGui OpenGL backend");
        return false;
    }

    // Create framebuffer for rendering
    if (!CreateFramebuffer()) {
        LOG_ERROR("Failed to create framebuffer");
        return false;
    }

    // Create VR overlay
    if (!CreateOverlay()) {
        LOG_ERROR("Failed to create VR overlay");
        return false;
    }

    // Initialize UI state
    m_uiState.lastInteraction = std::chrono::steady_clock::now();

    // Get available input devices from audio engine
    if (m_audioEngine) {
        auto devices = m_audioEngine->GetInputDevices();
        m_uiState.inputDevices.clear();
        for (const auto& device : devices) {
            m_uiState.inputDevices.push_back(device.name);
        }
    }

    m_initialized = true;
    LOG_INFO("Overlay UI initialized successfully");
    return true;
}

void OverlayUI::Update() {
    if (!m_initialized || !m_vrTracker || !m_audioEngine) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    // Process VR overlay input
    ProcessOverlayInput();

    // Update overlay transform
    UpdateOverlayTransform();

    // Update metrics
    UpdateMetrics();

    // Handle auto-hide
    HandleAutoHide();

    // Skip rendering if not visible
    if (!m_visible) {
        return;
    }

    // Make OpenGL context current
    glfwMakeContextCurrent(m_window);

    // Bind framebuffer for rendering
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glViewport(0, 0, m_textureWidth, m_textureHeight);

    // Clear framebuffer
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Render UI
    RenderMainWindow();

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update overlay texture
    UpdateTexture();

    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Swap buffers (though we don't display this window)
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void OverlayUI::Shutdown() {
    if (!m_initialized) {
        return;
    }

    // Destroy VR overlay
    if (m_overlayHandle != vr::k_ulOverlayHandleInvalid) {
        vr::VROverlay()->DestroyOverlay(m_overlayHandle);
        m_overlayHandle = vr::k_ulOverlayHandleInvalid;
    }

    if (m_thumbnailHandle != vr::k_ulOverlayHandleInvalid) {
        vr::VROverlay()->DestroyOverlay(m_thumbnailHandle);
        m_thumbnailHandle = vr::k_ulOverlayHandleInvalid;
    }

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Cleanup OpenGL
    CleanupOpenGL();

    m_initialized = false;
    LOG_INFO("Overlay UI shutdown complete");
}

void OverlayUI::SetVisible(bool visible) {
    m_visible = visible;

    if (m_overlayHandle != vr::k_ulOverlayHandleInvalid) {
        vr::VROverlay()->SetOverlayFlag(m_overlayHandle, vr::VROverlayFlags_VisibleInDashboard, visible);
        vr::VROverlay()->SetOverlayFlag(m_overlayHandle, vr::VROverlayFlags_NoDashboardTab, !visible);

        if (visible) {
            vr::VROverlay()->ShowOverlay(m_overlayHandle);
        } else {
            vr::VROverlay()->HideOverlay(m_overlayHandle);
        }
    }
}

void OverlayUI::SetPosition(float x, float y, float z) {
    m_overlayPosition = Vec3(x, y, z);
}

void OverlayUI::SetScale(float scale) {
    m_overlayScale = scale;
    if (m_overlayHandle != vr::k_ulOverlayHandleInvalid) {
        vr::VROverlay()->SetOverlayWidthInMeters(m_overlayHandle, m_overlayWidth * scale);
    }
}

void OverlayUI::RegisterParameterCallback(ParameterCallback callback) {
    m_parameterCallbacks.push_back(callback);
}

// ===== PRIVATE METHODS =====

bool OverlayUI::InitializeOpenGL() {
    // Initialize GLFW
    if (!glfwInit()) {
        LOG_ERROR("Failed to initialize GLFW");
        return false;
    }

    // Set OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Hidden window

    // Create hidden window for OpenGL context
    m_window = glfwCreateWindow(m_textureWidth, m_textureHeight, "VR Overlay", nullptr, nullptr);
    if (!m_window) {
        LOG_ERROR("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        LOG_ERROR("Failed to initialize GLEW");
        return false;
    }

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    LOG_INFO("OpenGL initialized - Version: {}", glGetString(GL_VERSION));
    return true;
}

void OverlayUI::CleanupOpenGL() {
    if (m_framebuffer) {
        glDeleteFramebuffers(1, &m_framebuffer);
        m_framebuffer = 0;
    }

    if (m_colorTexture) {
        glDeleteTextures(1, &m_colorTexture);
        m_colorTexture = 0;
    }

    if (m_depthTexture) {
        glDeleteTextures(1, &m_depthTexture);
        m_depthTexture = 0;
    }

    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }

    glfwTerminate();
}

bool OverlayUI::CreateFramebuffer() {
    // Generate framebuffer
    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

    // Create color texture
    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_textureWidth, m_textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);

    // Create depth texture
    glGenTextures(1, &m_depthTexture);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_textureWidth, m_textureHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("Framebuffer is not complete");
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

void OverlayUI::UpdateTexture() {
    if (m_overlayHandle != vr::k_ulOverlayHandleInvalid && m_colorTexture) {
        vr::Texture_t vrTexture = { (void*)(uintptr_t)m_colorTexture, vr::TextureType_OpenGL, vr::ColorSpace_Auto };
        vr::VROverlay()->SetOverlayTexture(m_overlayHandle, &vrTexture);
    }
}

bool OverlayUI::CreateOverlay() {
    vr::VROverlayError error = vr::VROverlay()->CreateOverlay("vrb.overlay.main", "VR Binaural Recorder", &m_overlayHandle);
    if (error != vr::VROverlayError_None) {
        LOG_ERROR("Failed to create VR overlay: {}", vr::VROverlay()->GetOverlayErrorNameFromEnum(error));
        return false;
    }

    // Configure overlay properties
    vr::VROverlay()->SetOverlayWidthInMeters(m_overlayHandle, m_overlayWidth);
    vr::VROverlay()->SetOverlayFlag(m_overlayHandle, vr::VROverlayFlags_VisibleInDashboard, true);
    vr::VROverlay()->SetOverlayFlag(m_overlayHandle, vr::VROverlayFlags_SendVRScrollEvents, true);
    vr::VROverlay()->SetOverlayFlag(m_overlayHandle, vr::VROverlayFlags_WantsModalBehavior, false);

    // Set input methods
    vr::VROverlay()->SetOverlayInputMethod(m_overlayHandle, vr::VROverlayInputMethod_Mouse);

    // Create thumbnail overlay for dashboard
    error = vr::VROverlay()->CreateOverlay("vrb.overlay.thumbnail", "VR Binaural Recorder", &m_thumbnailHandle);
    if (error == vr::VROverlayError_None) {
        vr::VROverlay()->SetOverlayFlag(m_thumbnailHandle, vr::VROverlayFlags_IsPremultiplied, true);
    }

    return true;
}

void OverlayUI::UpdateOverlayTransform() {
    if (m_overlayHandle == vr::k_ulOverlayHandleInvalid) {
        return;
    }

    // Set position in world space
    vr::HmdMatrix34_t transform;
    transform.m[0][0] = 1.0f; transform.m[0][1] = 0.0f; transform.m[0][2] = 0.0f; transform.m[0][3] = m_overlayPosition.x;
    transform.m[1][0] = 0.0f; transform.m[1][1] = 1.0f; transform.m[1][2] = 0.0f; transform.m[1][3] = m_overlayPosition.y;
    transform.m[2][0] = 0.0f; transform.m[2][1] = 0.0f; transform.m[2][2] = 1.0f; transform.m[2][3] = m_overlayPosition.z;

    vr::VROverlay()->SetOverlayTransformAbsolute(m_overlayHandle, vr::TrackingUniverseStanding, &transform);
}

void OverlayUI::ProcessOverlayInput() {
    if (m_overlayHandle == vr::k_ulOverlayHandleInvalid) {
        return;
    }

    vr::VREvent_t event;
    while (vr::VROverlay()->PollNextOverlayEvent(m_overlayHandle, &event, sizeof(event))) {
        switch (event.eventType) {
        case vr::VREvent_MouseMove:
            ImGui_ImplGlfw_CursorPosCallback(m_window, event.data.mouse.x, event.data.mouse.y);
            m_uiState.lastInteraction = std::chrono::steady_clock::now();
            break;

        case vr::VREvent_MouseButtonDown:
            ImGui_ImplGlfw_MouseButtonCallback(m_window, event.data.mouse.button, GLFW_PRESS, 0);
            m_uiState.lastInteraction = std::chrono::steady_clock::now();
            break;

        case vr::VREvent_MouseButtonUp:
            ImGui_ImplGlfw_MouseButtonCallback(m_window, event.data.mouse.button, GLFW_RELEASE, 0);
            break;

        case vr::VREvent_ScrollDiscrete:
            ImGui_ImplGlfw_ScrollCallback(m_window, event.data.scroll.xdelta, event.data.scroll.ydelta);
            m_uiState.lastInteraction = std::chrono::steady_clock::now();
            break;
        }
    }
}

void OverlayUI::ApplyTheme() {
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowPadding = ImVec2(15, 15);
    style.WindowRounding = 8.0f;
    style.FramePadding = ImVec2(8, 4);
    style.FrameRounding = 4.0f;
    style.ItemSpacing = ImVec2(12, 8);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 15.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabMinSize = 10.0f;
    style.GrabRounding = 3.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = m_theme.text;
    colors[ImGuiCol_WindowBg] = m_theme.background;
    colors[ImGuiCol_Header] = m_theme.header;
    colors[ImGuiCol_Button] = m_theme.button;
    colors[ImGuiCol_ButtonHovered] = m_theme.buttonHovered;
    colors[ImGuiCol_ButtonActive] = m_theme.buttonActive;
    colors[ImGuiCol_SliderGrab] = m_theme.sliderGrab;
    colors[ImGuiCol_CheckMark] = m_theme.checkMark;
}

void OverlayUI::UpdateMetrics() {
    if (!m_audioEngine) return;

    // Update audio metrics
    auto stats = m_audioEngine->GetStats();
    m_uiState.cpuUsage = stats.cpuLoad * 100.0f;
    m_uiState.bufferUnderruns = stats.underruns;

    // Calculate audio latency (rough estimate)
    // TODO: Get actual latency measurement from audio engine
    m_uiState.audioLatency = (float)GetBufferSize() / GetSampleRate() * 1000.0f;

    // VR tracking rate
    if (m_vrTracker && m_vrTracker->IsConnected()) {
        // TODO: Get actual tracking rate from VR tracker
        m_uiState.trackingRate = 90.0f; // Placeholder
    }

    // Update spectrum data
    // TODO: Get actual spectrum data from audio engine
    m_uiState.spectrumData.resize(128);
    for (size_t i = 0; i < m_uiState.spectrumData.size(); ++i) {
        m_uiState.spectrumData[i] = std::sin(i * 0.1f) * 0.5f + 0.5f;
    }
}

void OverlayUI::HandleAutoHide() {
    if (!m_uiState.autoHide) return;

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - m_uiState.lastInteraction).count();

    if (elapsed > m_uiState.autoHideDelay && m_visible) {
        SetVisible(false);
    }
}

ImVec4 OverlayUI::GetStatusColor(float value, float warning, float critical) const {
    if (value >= critical) return m_theme.error;
    if (value >= warning) return m_theme.warning;
    return m_theme.success;
}

void OverlayUI::DrawVUMeter(const char* label, float value, const ImVec2& size) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImVec2 canvas_size = size;

    // Background
    draw_list->AddRectFilled(canvas_pos,
                            ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
                            IM_COL32(50, 50, 50, 255));

    // Level bar
    float level_width = canvas_size.x * std::clamp(value, 0.0f, 1.0f);
    ImU32 level_color = value > 0.8f ? IM_COL32(255, 100, 100, 255) :
                       value > 0.6f ? IM_COL32(255, 255, 100, 255) :
                                     IM_COL32(100, 255, 100, 255);

    draw_list->AddRectFilled(canvas_pos,
                            ImVec2(canvas_pos.x + level_width, canvas_pos.y + canvas_size.y),
                            level_color);

    // Label
    draw_list->AddText(ImVec2(canvas_pos.x + 5, canvas_pos.y + 5), IM_COL32(255, 255, 255, 255), label);

    ImGui::Dummy(canvas_size);
}

void OverlayUI::RenderMainWindow() {
    // Set up main window to fill the overlay
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)m_textureWidth, (float)m_textureHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                            ImGuiWindowFlags_NoMove |
                            ImGuiWindowFlags_NoResize |
                            ImGuiWindowFlags_NoScrollWithMouse;

    if (ImGui::Begin("VR Binaural Recorder", nullptr, flags)) {
        // Title bar with status
        ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "VR Binaural Recorder v1.0.0");
        ImGui::SameLine();

        // Connection status
        bool vrConnected = m_vrTracker && m_vrTracker->IsConnected();
        ImGui::TextColored(vrConnected ? m_theme.success : m_theme.error,
                          vrConnected ? "VR Connected" : "VR Disconnected");

        ImGui::Separator();

        // Panel toggles
        if (ImGui::CollapsingHeader("Panels", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Audio", &m_uiState.showAudioPanel);
            ImGui::SameLine();
            ImGui::Checkbox("VR", &m_uiState.showVRPanel);
            ImGui::SameLine();
            ImGui::Checkbox("HRTF", &m_uiState.showHRTFPanel);
            ImGui::SameLine();
            ImGui::Checkbox("Performance", &m_uiState.showPerformance);
            ImGui::SameLine();
            ImGui::Checkbox("Advanced", &m_uiState.showAdvanced);
            ImGui::SameLine();
            ImGui::Checkbox("Debug", &m_uiState.showDebug);
        }

        // Render panels
        if (m_uiState.showAudioPanel) RenderAudioPanel();
        if (m_uiState.showVRPanel) RenderVRPanel();
        if (m_uiState.showHRTFPanel) RenderHRTFPanel();
        if (m_uiState.showPerformance) RenderPerformancePanel();
        if (m_uiState.showAdvanced) RenderAdvancedPanel();
        if (m_uiState.showDebug) RenderDebugPanel();
    }
    ImGui::End();
}

void OverlayUI::RenderAudioPanel() {
    if (ImGui::CollapsingHeader("Audio Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        // Input device selection
        if (!m_uiState.inputDevices.empty()) {
            const char* current_device = m_uiState.selectedInput < m_uiState.inputDevices.size() ?
                m_uiState.inputDevices[m_uiState.selectedInput].c_str() : "Unknown";

            if (ImGui::BeginCombo("Input Device", current_device)) {
                for (size_t i = 0; i < m_uiState.inputDevices.size(); ++i) {
                    bool is_selected = (m_uiState.selectedInput == i);
                    if (ImGui::Selectable(m_uiState.inputDevices[i].c_str(), is_selected)) {
                        m_uiState.selectedInput = static_cast<int>(i);
                        // TODO: Actually change input device
                    }
                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
        }

        // Gain controls
        if (ImGui::SliderFloat("Input Gain", &m_uiState.inputGain, 0.0f, 2.0f, "%.2f")) {
            for (auto& callback : m_parameterCallbacks) {
                callback("inputGain", m_uiState.inputGain);
            }
        }

        if (ImGui::SliderFloat("Output Gain", &m_uiState.outputGain, 0.0f, 2.0f, "%.2f")) {
            for (auto& callback : m_parameterCallbacks) {
                callback("outputGain", m_uiState.outputGain);
            }
        }

        // Mute controls
        ImGui::Checkbox("Mute Input", &m_uiState.muteInput);
        ImGui::SameLine();
        ImGui::Checkbox("Mute Output", &m_uiState.muteOutput);
        ImGui::SameLine();
        ImGui::Checkbox("Bypass HRTF", &m_uiState.bypass);

        // Level meters
        if (m_uiState.showMeters) {
            ImGui::Text("Input Level:");
            DrawVUMeter("INPUT", m_uiState.inputLevel, ImVec2(200, 20));

            ImGui::Text("Output Level:");
            DrawVUMeter("LEFT", m_uiState.outputLevelL, ImVec2(200, 20));
            DrawVUMeter("RIGHT", m_uiState.outputLevelR, ImVec2(200, 20));
        }
    }
}

void OverlayUI::RenderVRPanel() {
    if (ImGui::CollapsingHeader("VR Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        // Microphone position controls
        ImGui::Text("Virtual Microphone Position:");

        if (ImGui::SliderFloat("Distance", &m_uiState.micDistance, 0.1f, 5.0f, "%.2fm")) {
            for (auto& callback : m_parameterCallbacks) {
                callback("micDistance", m_uiState.micDistance);
            }
        }

        if (ImGui::SliderFloat("Height", &m_uiState.micHeight, 0.0f, 3.0f, "%.2fm")) {
            for (auto& callback : m_parameterCallbacks) {
                callback("micHeight", m_uiState.micHeight);
            }
        }

        if (ImGui::SliderFloat("Angle", &m_uiState.micAngle, -180.0f, 180.0f, "%.1f°")) {
            for (auto& callback : m_parameterCallbacks) {
                callback("micAngle", m_uiState.micAngle);
            }
        }

        ImGui::Checkbox("Lock Position", &m_uiState.lockMicPosition);

        if (ImGui::Button("Reset Position")) {
            m_uiState.micDistance = 1.0f;
            m_uiState.micHeight = 1.2f;
            m_uiState.micAngle = 0.0f;
        }

        // Tracking settings
        ImGui::Separator();
        ImGui::Text("Tracking:");

        ImGui::Checkbox("Smooth Tracking", &m_uiState.smoothTracking);

        if (ImGui::SliderFloat("Smoothing", &m_uiState.smoothingFactor, 0.0f, 0.99f, "%.3f")) {
            for (auto& callback : m_parameterCallbacks) {
                callback("smoothingFactor", m_uiState.smoothingFactor);
            }
        }

        ImGui::Checkbox("Show Chaperone", &m_uiState.showChaperone);

        // 3D visualization
        if (m_uiState.show3DView) {
            ImGui::Separator();
            Draw3DSpatialView();
        }
    }
}

void OverlayUI::RenderHRTFPanel() {
    if (ImGui::CollapsingHeader("HRTF Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Enable HRTF", &m_uiState.enableHRTF);

        // HRTF preset selection
        const char* presets[] = { "Default", "Small Head", "Large Head", "Custom" };
        ImGui::Combo("HRTF Preset", &m_uiState.hrtfPreset, presets, IM_ARRAYSIZE(presets));

        // Distance attenuation
        if (ImGui::SliderFloat("Distance Attenuation", &m_uiState.distanceAttenuation, 0.0f, 2.0f, "%.2f")) {
            for (auto& callback : m_parameterCallbacks) {
                callback("distanceAttenuation", m_uiState.distanceAttenuation);
            }
        }

        ImGui::Checkbox("Near-field Compensation", &m_uiState.nearFieldCompensation);

        // Crossfeed for headphones
        if (ImGui::SliderFloat("Crossfeed", &m_uiState.crossfeedLevel, 0.0f, 1.0f, "%.2f")) {
            for (auto& callback : m_parameterCallbacks) {
                callback("crossfeedLevel", m_uiState.crossfeedLevel);
            }
        }

        // Spectrum visualization
        if (m_uiState.showSpectrum) {
            ImGui::Separator();
            RenderSpectrum();
        }
    }
}

void OverlayUI::RenderPerformancePanel() {
    if (ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen)) {
        // CPU usage
        ImGui::Text("CPU Usage: %.1f%%", m_uiState.cpuUsage);
        ImGui::ProgressBar(m_uiState.cpuUsage / 100.0f, ImVec2(0.0f, 0.0f));

        // Audio latency
        ImGui::Text("Audio Latency: %.1f ms", m_uiState.audioLatency);
        ImVec4 latencyColor = GetStatusColor(m_uiState.audioLatency, 10.0f, 20.0f);
        ImGui::TextColored(latencyColor, "%.1f ms", m_uiState.audioLatency);

        // Buffer underruns
        ImGui::Text("Buffer Underruns: %d", m_uiState.bufferUnderruns);

        // VR tracking rate
        if (m_vrTracker && m_vrTracker->IsConnected()) {
            ImGui::Text("Tracking Rate: %.1f Hz", m_uiState.trackingRate);
        }
    }
}

void OverlayUI::RenderAdvancedPanel() {
    if (ImGui::CollapsingHeader("Advanced Settings")) {
        ImGui::Text("Advanced audio and VR settings will go here");
        // TODO: Implement advanced settings
    }
}

void OverlayUI::RenderDebugPanel() {
    if (ImGui::CollapsingHeader("Debug Information")) {
        ImGui::Text("Debug information and diagnostics will go here");
        // TODO: Implement debug panel
    }
}

void OverlayUI::RenderSpectrum() {
    if (!m_uiState.spectrumData.empty()) {
        ImGui::PlotHistogram("Spectrum", m_uiState.spectrumData.data(),
                           static_cast<int>(m_uiState.spectrumData.size()),
                           0, nullptr, 0.0f, 1.0f, ImVec2(0, 80));
    }
}

void OverlayUI::RenderSpatialVisualization() {
    // TODO: Implement 3D spatial visualization
}

void OverlayUI::RenderLevelMeters() {
    // Already implemented in RenderAudioPanel
}

void OverlayUI::DrawSpectralDisplay(const float* spectrum, int bins) {
    // TODO: Implement spectral display
}

void OverlayUI::Draw3DSpatialView() {
    // TODO: Implement 3D spatial view showing head and microphone positions
    ImGui::Text("[3D Spatial View - TODO]");
}

// Helper method implementations

int OverlayUI::GetBufferSize() const {
    if (m_audioEngine) {
        // TODO: Connect to actual audio engine buffer size
        // For now, return a sensible default
        return 256;
    }
    return 256;
}

float OverlayUI::GetSampleRate() const {
    if (m_audioEngine) {
        // TODO: Connect to actual audio engine sample rate
        // For now, return a sensible default
        return 48000.0f;
    }
    return 48000.0f;
}

} // namespace vrb