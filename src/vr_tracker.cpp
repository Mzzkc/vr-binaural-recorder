// vr_tracker.h - OpenVR Tracking Module
// Handles VR headset and controller tracking for spatial positioning

#pragma once

#include <openvr.h>
#include <functional>
#include <atomic>
#include <thread>
#include "vr_types.h"

namespace vrb {

/**
 * @brief VR tracking interface for OpenVR/SteamVR
 * 
 * Tracks headset position and manages virtual microphone placement
 */
class VRTracker {
public:
    using TrackingCallback = std::function<void(const VRPose& head, const VRPose& mic)>;

    VRTracker();
    ~VRTracker();

    /**
     * @brief Initialize OpenVR system
     * @return true if VR system initialized successfully
     */
    bool Initialize();

    /**
     * @brief Start tracking thread
     */
    void StartTracking();

    /**
     * @brief Stop tracking thread
     */
    void StopTracking();

    /**
     * @brief Process VR events (called from main thread)
     */
    void ProcessEvents();

    /**
     * @brief Set callback for tracking updates
     * @param callback Function called when tracking data updates
     */
    void SetTrackingCallback(TrackingCallback callback);

    /**
     * @brief Get current headset pose
     * @return Current head tracking data
     */
    VRPose GetHeadPose() const;

    /**
     * @brief Get virtual microphone pose
     * @return Current microphone position in VR space
     */
    VRPose GetMicrophonePose() const;

    /**
     * @brief Set microphone position (relative to play space)
     * @param position Position in VR coordinates
     */
    void SetMicrophonePosition(const Vec3& position);

    /**
     * @brief Reset microphone to default position
     */
    void ResetMicrophonePosition();

    /**
     * @brief Check if VR system is connected
     * @return true if VR system is available
     */
    bool IsConnected() const { return m_vrSystem != nullptr; }

private:
    void TrackingThread();
    void UpdatePoses();
    VRPose ConvertMatrix(const vr::HmdMatrix34_t& matrix);

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

// overlay_ui.h - Minimal VR Overlay Interface

#pragma once

#include <openvr.h>
#include <memory>
#include <string>
#include "vr_tracker.h"
#include "audio_engine.h"

namespace vrb {

// Forward declaration
class OverlayRenderer;

/**
 * @brief Minimal VR overlay for microphone selection and visualization
 * 
 * Displays a simple panel with:
 * - Microphone selector dropdown
 * - Virtual microphone position indicator
 * - Minimal visual feedback
 */
class OverlayUI {
public:
    OverlayUI();
    ~OverlayUI();

    /**
     * @brief Initialize overlay system
     * @param tracker VR tracking system
     * @param audio Audio engine for device management
     * @return true if initialization successful
     */
    bool Initialize(VRTracker* tracker, AudioEngine* audio);

    /**
     * @brief Update overlay rendering
     */
    void Update();

    /**
     * @brief Shutdown overlay system
     */
    void Shutdown();

    /**
     * @brief Show/hide overlay
     * @param visible Visibility state
     */
    void SetVisible(bool visible);

    /**
     * @brief Check if overlay is visible
     * @return true if overlay is shown
     */
    bool IsVisible() const { return m_visible; }

private:
    bool CreateOverlay();
    void RenderOverlay();
    void HandleInput();
    void UpdateMicrophoneList();

    vr::IVROverlay* m_vrOverlay;
    vr::VROverlayHandle_t m_overlayHandle;
    vr::VROverlayHandle_t m_thumbnailHandle;
    
    VRTracker* m_vrTracker;
    AudioEngine* m_audioEngine;
    
    std::atomic<bool> m_initialized{false};
    std::atomic<bool> m_visible{true};
    
    // UI State
    int m_selectedMicIndex;
    std::vector<AudioEngine::DeviceInfo> m_microphoneList;
    
    // Render target
    std::unique_ptr<OverlayRenderer> m_renderer;
    uint32_t m_textureId;
    int m_textureWidth;
    int m_textureHeight;
};

// vr_tracker.cpp - Implementation

#include "vr_tracker.h"
#include "logger.h"
#include <chrono>

namespace vrb {

VRTracker::VRTracker() : m_vrSystem(nullptr) {
    // Default microphone position: 1 meter in front at chest height
    m_microphonePose.position = {0.0f, 1.2f, -1.0f};
    m_microphonePose.orientation = {0.0f, 0.0f, 0.0f, 1.0f};
}

VRTracker::~VRTracker() {
    StopTracking();
    
    if (m_vrSystem) {
        vr::VR_Shutdown();
        m_vrSystem = nullptr;
        LOG_INFO("OpenVR shutdown complete");
    }
}

bool VRTracker::Initialize() {
    if (m_initialized) {
        LOG_WARN("VR tracker already initialized");
        return true;
    }

    // Check if VR runtime is installed
    if (!vr::VR_IsRuntimeInstalled()) {
        LOG_ERROR("SteamVR runtime not found");
        return false;
    }

    // Check if HMD is present
    if (!vr::VR_IsHmdPresent()) {
        LOG_ERROR("No VR headset detected");
        return false;
    }

    // Initialize OpenVR
    vr::EVRInitError vrError = vr::VRInitError_None;
    m_vrSystem = vr::VR_Init(&vrError, vr::VRApplication_Overlay);
    
    if (vrError != vr::VRInitError_None) {
        m_vrSystem = nullptr;
        LOG_ERROR("Failed to initialize OpenVR: {}", vr::VR_GetVRInitErrorAsEnglishDescription(vrError));
        return false;
    }

    // Get display frequency for optimal tracking rate
    float displayFreq = 90.0f;  // Default
    m_vrSystem->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd,
                                               vr::Prop_DisplayFrequency_Float, nullptr);
    
    LOG_INFO("OpenVR initialized - Display frequency: {}Hz", displayFreq);
    
    m_initialized = true;
    return true;
}

void VRTracker::StartTracking() {
    if (!m_initialized || m_tracking) {
        return;
    }

    m_tracking = true;
    m_trackingThread = std::thread(&VRTracker::TrackingThread, this);
    LOG_INFO("VR tracking started");
}

void VRTracker::StopTracking() {
    if (!m_tracking) {
        return;
    }

    m_tracking = false;
    if (m_trackingThread.joinable()) {
        m_trackingThread.join();
    }
    LOG_INFO("VR tracking stopped");
}

void VRTracker::ProcessEvents() {
    if (!m_vrSystem) {
        return;
    }

    vr::VREvent_t event;
    while (m_vrSystem->PollNextEvent(&event, sizeof(event))) {
        switch (event.eventType) {
            case vr::VREvent_TrackedDeviceActivated:
                LOG_INFO("Device {} activated", event.trackedDeviceIndex);
                break;
                
            case vr::VREvent_TrackedDeviceDeactivated:
                LOG_INFO("Device {} deactivated", event.trackedDeviceIndex);
                break;
                
            case vr::VREvent_TrackedDeviceUpdated:
                LOG_DEBUG("Device {} updated", event.trackedDeviceIndex);
                break;
                
            case vr::VREvent_DashboardActivated:
                LOG_DEBUG("Dashboard activated");
                break;
                
            case vr::VREvent_DashboardDeactivated:
                LOG_DEBUG("Dashboard deactivated");
                break;
                
            default:
                break;
        }
    }
}

void VRTracker::SetTrackingCallback(TrackingCallback callback) {
    m_callback = callback;
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
    LOG_INFO("Microphone position set to: ({}, {}, {})", position.x, position.y, position.z);
}

void VRTracker::ResetMicrophonePosition() {
    SetMicrophonePosition({0.0f, 1.2f, -1.0f});
    LOG_INFO("Microphone position reset to default");
}

void VRTracker::TrackingThread() {
    LOG_INFO("Tracking thread started");
    
    const auto trackingPeriod = std::chrono::milliseconds(11);  // ~90Hz
    auto nextUpdate = std::chrono::steady_clock::now();
    
    while (m_tracking) {
        UpdatePoses();
        
        // Call tracking callback if set
        if (m_callback) {
            std::lock_guard<std::mutex> lock(m_poseMutex);
            m_callback(m_headPose, m_microphonePose);
        }
        
        // Maintain consistent update rate
        nextUpdate += trackingPeriod;
        std::this_thread::sleep_until(nextUpdate);
    }
    
    LOG_INFO("Tracking thread ended");
}

void VRTracker::UpdatePoses() {
    if (!m_vrSystem) {
        return;
    }

    // Get all device poses
    vr::VRCompositor()->GetLastPoses(m_trackedDevicePoses, vr::k_unMaxTrackedDeviceCount,
                                     nullptr, 0);

    // Find HMD
    for (uint32_t device = 0; device < vr::k_unMaxTrackedDeviceCount; ++device) {
        if (m_vrSystem->GetTrackedDeviceClass(device) == vr::TrackedDeviceClass_HMD) {
            if (m_trackedDevicePoses[device].bPoseIsValid) {
                std::lock_guard<std::mutex> lock(m_poseMutex);
                m_headPose = ConvertMatrix(m_trackedDevicePoses[device].mDeviceToAbsoluteTracking);
            }
            break;
        }
    }
}

VRPose VRTracker::ConvertMatrix(const vr::HmdMatrix34_t& matrix) {
    VRPose pose;
    
    // Extract position
    pose.position.x = matrix.m[0][3];
    pose.position.y = matrix.m[1][3];
    pose.position.z = matrix.m[2][3];
    
    // Full robust matrix to quaternion conversion
    float m00 = matrix.m[0][0], m01 = matrix.m[0][1], m02 = matrix.m[0][2];
    float m10 = matrix.m[1][0], m11 = matrix.m[1][1], m12 = matrix.m[1][2];
    float m20 = matrix.m[2][0], m21 = matrix.m[2][1], m22 = matrix.m[2][2];
    
    float trace = m00 + m11 + m22;
    
    if (trace > 0.0f) {
        float s = 2.0f * sqrtf(trace + 1.0f);
        pose.orientation.w = 0.25f * s;
        pose.orientation.x = (m21 - m12) / s;
        pose.orientation.y = (m02 - m20) / s;
        pose.orientation.z = (m10 - m01) / s;
    } else if ((m00 > m11) && (m00 > m22)) {
        float s = 2.0f * sqrtf(1.0f + m00 - m11 - m22);
        pose.orientation.w = (m21 - m12) / s;
        pose.orientation.x = 0.25f * s;
        pose.orientation.y = (m01 + m10) / s;
        pose.orientation.z = (m02 + m20) / s;
    } else if (m11 > m22) {
        float s = 2.0f * sqrtf(1.0f + m11 - m00 - m22);
        pose.orientation.w = (m02 - m20) / s;
        pose.orientation.x = (m01 + m10) / s;
        pose.orientation.y = 0.25f * s;
        pose.orientation.z = (m12 + m21) / s;
    } else {
        float s = 2.0f * sqrtf(1.0f + m22 - m00 - m11);
        pose.orientation.w = (m10 - m01) / s;
        pose.orientation.x = (m02 + m20) / s;
        pose.orientation.y = (m12 + m21) / s;
        pose.orientation.z = 0.25f * s;
    }
    
    // Normalize quaternion
    float norm = sqrtf(pose.orientation.x * pose.orientation.x +
                      pose.orientation.y * pose.orientation.y +
                      pose.orientation.z * pose.orientation.z +
                      pose.orientation.w * pose.orientation.w);
    
    if (norm > 0.0f) {
        pose.orientation.x /= norm;
        pose.orientation.y /= norm;
        pose.orientation.z /= norm;
        pose.orientation.w /= norm;
    }
    
    return pose;
}

// overlay_ui.cpp - Implementation

#include "overlay_ui.h"
#include "logger.h"
#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image_write.h>

namespace vrb {

class OverlayRenderer {
public:
    OverlayRenderer() : m_window(nullptr), m_fbo(0), m_texture(0), m_rbo(0) {}
    
    bool Initialize(int width, int height) {
        // Initialize GLFW for offscreen rendering
        if (!glfwInit()) {
            LOG_ERROR("Failed to initialize GLFW");
            return false;
        }
        
        // Create hidden window for OpenGL context
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        
        m_window = glfwCreateWindow(width, height, "VR Overlay Renderer", nullptr, nullptr);
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
        
        // Create framebuffer for rendering
        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        
        // Create texture
        glGenTextures(1, &m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
        
        // Create render buffer for depth
        glGenRenderbuffers(1, &m_rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            LOG_ERROR("Framebuffer not complete");
            return false;
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        m_width = width;
        m_height = height;
        
        return true;
    }
    
    void BeginFrame() {
        glfwMakeContextCurrent(m_window);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glViewport(0, 0, m_width, m_height);
        glClearColor(0.1f, 0.1f, 0.1f, 0.95f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    
    void EndFrame() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    GLuint GetTexture() const { return m_texture; }
    
    void Shutdown() {
        if (m_fbo) glDeleteFramebuffers(1, &m_fbo);
        if (m_texture) glDeleteTextures(1, &m_texture);
        if (m_rbo) glDeleteRenderbuffers(1, &m_rbo);
        
        if (m_window) {
            glfwDestroyWindow(m_window);
            glfwTerminate();
        }
    }
    
private:
    GLFWwindow* m_window;
    GLuint m_fbo;
    GLuint m_texture;
    GLuint m_rbo;
    int m_width;
    int m_height;
};

OverlayUI::OverlayUI() 
    : m_vrOverlay(nullptr)
    , m_overlayHandle(vr::k_ulOverlayHandleInvalid)
    , m_thumbnailHandle(vr::k_ulOverlayHandleInvalid)
    , m_vrTracker(nullptr)
    , m_audioEngine(nullptr)
    , m_selectedMicIndex(-1)
    , m_textureId(0)
    , m_textureWidth(400)
    , m_textureHeight(200) {
    m_renderer = std::make_unique<OverlayRenderer>();
}

OverlayUI::~OverlayUI() {
    Shutdown();
}

bool OverlayUI::Initialize(VRTracker* tracker, AudioEngine* audio) {
    if (m_initialized) {
        LOG_WARN("Overlay UI already initialized");
        return true;
    }

    m_vrTracker = tracker;
    m_audioEngine = audio;

    if (!m_vrTracker || !m_audioEngine) {
        LOG_ERROR("Invalid tracker or audio engine pointer");
        return false;
    }

    // Get OpenVR overlay interface
    m_vrOverlay = vr::VROverlay();
    if (!m_vrOverlay) {
        LOG_ERROR("Failed to get VROverlay interface");
        return false;
    }

    // Initialize OpenGL renderer
    if (!m_renderer->Initialize(m_textureWidth, m_textureHeight)) {
        LOG_ERROR("Failed to initialize overlay renderer");
        return false;
    }
    
    m_textureId = m_renderer->GetTexture();

    // Create overlay
    if (!CreateOverlay()) {
        LOG_ERROR("Failed to create overlay");
        return false;
    }

    // Initialize ImGui for rendering
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.DisplaySize = ImVec2((float)m_textureWidth, (float)m_textureHeight);
    
    // Initialize ImGui OpenGL3 backend
    ImGui_ImplOpenGL3_Init("#version 330");
    
    // Setup minimal dark style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.95f);

    UpdateMicrophoneList();

    m_initialized = true;
    LOG_INFO("Overlay UI initialized");
    return true;
}

void OverlayUI::Update() {
    if (!m_initialized || !m_visible) {
        return;
    }

    RenderOverlay();
    HandleInput();
}

void OverlayUI::Shutdown() {
    if (m_vrOverlay && m_overlayHandle != vr::k_ulOverlayHandleInvalid) {
        m_vrOverlay->DestroyOverlay(m_overlayHandle);
        m_overlayHandle = vr::k_ulOverlayHandleInvalid;
    }

    if (m_vrOverlay && m_thumbnailHandle != vr::k_ulOverlayHandleInvalid) {
        m_vrOverlay->DestroyOverlay(m_thumbnailHandle);
        m_thumbnailHandle = vr::k_ulOverlayHandleInvalid;
    }

    if (ImGui::GetCurrentContext()) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui::DestroyContext();
    }
    
    if (m_renderer) {
        m_renderer->Shutdown();
    }

    LOG_INFO("Overlay UI shutdown");
}

void OverlayUI::SetVisible(bool visible) {
    m_visible = visible;
    
    if (m_vrOverlay && m_overlayHandle != vr::k_ulOverlayHandleInvalid) {
        if (visible) {
            m_vrOverlay->ShowOverlay(m_overlayHandle);
        } else {
            m_vrOverlay->HideOverlay(m_overlayHandle);
        }
    }
}

bool OverlayUI::CreateOverlay() {
    // Create main overlay
    vr::EVROverlayError error = m_vrOverlay->CreateOverlay(
        "vrb.binaural.recorder.overlay",
        "VR Binaural Recorder",
        &m_overlayHandle
    );

    if (error != vr::VROverlayError_None) {
        LOG_ERROR("Failed to create overlay: {}", m_vrOverlay->GetOverlayErrorNameFromEnum(error));
        return false;
    }

    // Set overlay properties
    m_vrOverlay->SetOverlayWidthInMeters(m_overlayHandle, 0.5f);
    m_vrOverlay->SetOverlayAlpha(m_overlayHandle, 1.0f);
    m_vrOverlay->SetOverlayInputMethod(m_overlayHandle, vr::VROverlayInputMethod_Mouse);
    
    // Position overlay in front of user
    vr::HmdMatrix34_t transform = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 1.5f,
        0.0f, 0.0f, 1.0f, -1.0f
    };
    m_vrOverlay->SetOverlayTransformAbsolute(m_overlayHandle, vr::TrackingUniverseStanding, &transform);

    // Create thumbnail for dashboard
    error = m_vrOverlay->CreateOverlay(
        "vrb.binaural.recorder.thumbnail",
        "VR Binaural",
        &m_thumbnailHandle
    );

    if (error == vr::VROverlayError_None) {
        m_vrOverlay->SetOverlayWidthInMeters(m_thumbnailHandle, 0.1f);
        m_vrOverlay->SetOverlayFromFile(m_thumbnailHandle, "./icon.png");
        m_vrOverlay->SetOverlayIsDashboardOverlay(m_thumbnailHandle, true);
    }

    LOG_INFO("Overlay created successfully");
    return true;
}

void OverlayUI::RenderOverlay() {
    // Begin rendering to texture
    m_renderer->BeginFrame();
    
    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    // Main window
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(380, 180), ImGuiCond_Always);
    ImGui::Begin("VR Binaural Recorder", nullptr, 
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | 
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

    // Title
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::Text("VR BINAURAL RECORDER");
    ImGui::PopFont();
    ImGui::Separator();
    ImGui::Spacing();

    // Microphone selector
    ImGui::Text("Input Microphone:");
    
    const char* currentMicName = m_selectedMicIndex >= 0 ? 
                                 m_microphoneList[m_selectedMicIndex].name.c_str() : 
                                 "Select Microphone";
    
    if (ImGui::BeginCombo("##MicSelect", currentMicName)) {
        for (size_t i = 0; i < m_microphoneList.size(); ++i) {
            bool isSelected = (m_selectedMicIndex == static_cast<int>(i));
            if (ImGui::Selectable(m_microphoneList[i].name.c_str(), isSelected)) {
                m_selectedMicIndex = static_cast<int>(i);
                m_audioEngine->SelectInputDevice(m_microphoneList[i].index);
                LOG_INFO("User selected microphone: {}", m_microphoneList[i].name);
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Virtual microphone position display
    VRPose micPose = m_vrTracker->GetMicrophonePose();
    ImGui::Text("Virtual Mic Position:");
    ImGui::Text("  X: %.2f  Y: %.2f  Z: %.2f", 
                micPose.position.x, micPose.position.y, micPose.position.z);

    ImGui::Spacing();

    // Reset button
    if (ImGui::Button("Reset Mic Position")) {
        m_vrTracker->ResetMicrophonePosition();
        LOG_INFO("User reset microphone position");
    }
    
    ImGui::SameLine();
    
    // Hide overlay button
    if (ImGui::Button("Hide Overlay")) {
        SetVisible(false);
    }

    ImGui::End();

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    // End rendering
    m_renderer->EndFrame();
    
    // Update overlay texture
    vr::Texture_t vrTexture;
    vrTexture.handle = (void*)(uintptr_t)m_textureId;
    vrTexture.eType = vr::TextureType_OpenGL;
    vrTexture.eColorSpace = vr::ColorSpace_Gamma;
    
    vr::EVROverlayError error = m_vrOverlay->SetOverlayTexture(m_overlayHandle, &vrTexture);
    if (error != vr::VROverlayError_None) {
        LOG_ERROR("Failed to set overlay texture: {}", 
                  m_vrOverlay->GetOverlayErrorNameFromEnum(error));
    }
}

void OverlayUI::HandleInput() {
    // Handle VR controller input for overlay interaction
    vr::VREvent_t event;
    while (m_vrOverlay->PollNextOverlayEvent(m_overlayHandle, &event, sizeof(event))) {
        switch (event.eventType) {
            case vr::VREvent_MouseMove: {
                float x = event.data.mouse.x * m_textureWidth;
                float y = (1.0f - event.data.mouse.y) * m_textureHeight;
                ImGuiIO& io = ImGui::GetIO();
                io.MousePos = ImVec2(x, y);
                break;
            }
            
            case vr::VREvent_MouseButtonDown: {
                if (event.data.mouse.button == vr::VRMouseButton_Left) {
                    ImGuiIO& io = ImGui::GetIO();
                    io.MouseDown[0] = true;
                }
                break;
            }
            
            case vr::VREvent_MouseButtonUp: {
                if (event.data.mouse.button == vr::VRMouseButton_Left) {
                    ImGuiIO& io = ImGui::GetIO();
                    io.MouseDown[0] = false;
                }
                break;
            }
            
            case vr::VREvent_OverlayShown: {
                LOG_DEBUG("Overlay shown");
                break;
            }
            
            case vr::VREvent_OverlayHidden: {
                LOG_DEBUG("Overlay hidden");
                break;
            }
            
            default:
                break;
        }
    }
    
    // Check for dashboard activation
    if (m_thumbnailHandle != vr::k_ulOverlayHandleInvalid) {
        while (m_vrOverlay->PollNextOverlayEvent(m_thumbnailHandle, &event, sizeof(event))) {
            if (event.eventType == vr::VREvent_DashboardActivated) {
                SetVisible(true);
            }
        }
    }
}

void OverlayUI::UpdateMicrophoneList() {
    m_microphoneList = m_audioEngine->GetInputDevices();
    
    // Find current device in list
    std::string currentDevice = m_audioEngine->GetCurrentInputDevice();
    for (size_t i = 0; i < m_microphoneList.size(); ++i) {
        if (m_microphoneList[i].name == currentDevice) {
            m_selectedMicIndex = static_cast<int>(i);
            break;
        }
    }
    
    LOG_INFO("Updated microphone list - {} devices found", m_microphoneList.size());
}

} // namespace vrb