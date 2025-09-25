// audio_routing_overlay.cpp - The Audio Cockpit Implementation!
// 🎧 Where boring audio routing becomes VR magic! 🎧

#include "audio_routing_overlay.h"
#include "logger.h"
#include <algorithm>
#include <cmath>
#include <cstring>

// Quick OpenGL includes - we'll make this work cross-platform later!
#ifdef _WIN32
    #include <GL/glew.h>
    #include <GL/wglext.h>
#else
    #define GL_GLEXT_PROTOTYPES
    #include <GL/gl.h>
    #include <GL/glext.h>
#endif

namespace vrb {

// === CREATIVE CONSTANTS ===
constexpr float PI = 3.14159265359f;
constexpr float TWO_PI = 6.28318530718f;
constexpr float HALF_PI = 1.57079632679f;

// Magic numbers that just WORK!
constexpr float AUDIO_ORB_BASE_SIZE = 0.08f;     // Base orb size
constexpr float AUDIO_ORB_MAX_SIZE = 0.25f;      // Max orb size when loud
constexpr float PARTICLE_EMIT_RATE = 30.0f;     // Particles per second
constexpr float GESTURE_DEADZONE = 0.02f;       // Controller deadzone
constexpr float GRAB_DISTANCE = 0.15f;          // Grab tolerance

AudioRoutingOverlay::AudioRoutingOverlay() {
    // Initialize overlay transform to identity
    memset(&m_overlayTransform, 0, sizeof(m_overlayTransform));
    m_overlayTransform.m[0][0] = 1.0f;
    m_overlayTransform.m[1][1] = 1.0f;
    m_overlayTransform.m[2][2] = 1.0f;

    // Initialize HRTF presets - instant audio magic!
    InitializePresets();

    LOG_INFO("AudioRoutingOverlay constructed - ready to make audio routing AWESOME!");
}

AudioRoutingOverlay::~AudioRoutingOverlay() {
    Shutdown();
}

bool AudioRoutingOverlay::Initialize(VRTracker* vrTracker, AudioEngine* audioEngine) {
    if (m_initialized.load()) {
        LOG_WARN("AudioRoutingOverlay already initialized!");
        return true;
    }

    LOG_INFO("Initializing the Audio Cockpit - prepare for VR audio magic! 🚀");

    m_vrTracker = vrTracker;
    m_audioEngine = audioEngine;

    // Create the ACTUAL SteamVR overlay - this is where the magic happens!
    if (!CreateOverlay()) {
        LOG_ERROR("Failed to create SteamVR overlay - can't display the Audio Cockpit!");
        return false;
    }

    // Create OpenGL render target for our beautiful visuals
    if (!CreateRenderTarget()) {
        LOG_ERROR("Failed to create render target - no pretty graphics for you!");
        return false;
    }

    // Set up some default audio sources to play with
    AddAudioSource("Game Audio");
    AddAudioSource("Voice Chat");
    AddAudioSource("Music");
    AddAudioSource("System Sounds");

    // Position audio orbs in a nice circle around the user
    float angle = 0.0f;
    for (auto& [name, source] : m_audioSources) {
        source->orbPosition = Vec3{
            std::sin(angle) * 0.5f,
            0.0f,
            std::cos(angle) * 0.5f
        };
        angle += TWO_PI / m_audioSources.size();
    }

    m_initialized = true;
    LOG_INFO("Audio Cockpit initialized successfully - let the audio magic begin!");
    return true;
}

void AudioRoutingOverlay::Update() {
    if (!m_initialized.load()) {
        return;
    }

    // Sync to VR compositor timing for buttery smooth rendering
    SyncToCompositor();

    // Update controller states and process gestures
    UpdateControllerStates();
    ProcessGestures();

    // Update our magical audio orbs
    UpdateAudioOrbs();

    // Update particle system for beautiful visualization
    UpdateParticleSystem();

    // Update audio visualization data
    UpdateAudioVisualization();

    // Handle auto-hide functionality
    HandleAutoHide();

    if (m_visible.load()) {
        // Render the beautiful Audio Cockpit
        RenderToTexture();
        UpdateOverlayTexture();
    }

    // Update overlay position
    UpdateOverlayTransform();

    // Update metrics for performance monitoring
    UpdateMetrics();
}

void AudioRoutingOverlay::Shutdown() {
    if (!m_initialized.load()) {
        return;
    }

    LOG_INFO("Shutting down Audio Cockpit - audio routing magic is ending!");

    DestroyRenderTarget();
    DestroyOverlay();

    m_audioSources.clear();
    m_particles.clear();

    m_initialized = false;
}

// === CREATIVE OVERLAY CREATION ===
bool AudioRoutingOverlay::CreateOverlay() {
    if (!vr::VROverlay()) {
        LOG_ERROR("VROverlay system not available - no VR audio magic for you!");
        return false;
    }

    // Create the main overlay - our beautiful Audio Cockpit!
    vr::EVROverlayError overlayError = vr::VROverlay()->CreateOverlay(
        "vrb.audio_routing_cockpit",
        "VR Audio Cockpit",
        &m_overlayHandle
    );

    if (overlayError != vr::VROverlayError_None) {
        LOG_ERROR("Failed to create overlay: {}", (int)overlayError);
        return false;
    }

    // Set up overlay properties for maximum awesomeness
    vr::VROverlay()->SetOverlayWidthInMeters(m_overlayHandle, 2.0f);  // Nice and big!
    vr::VROverlay()->SetOverlayFlag(m_overlayHandle, vr::VROverlayFlags_SideBySide_Parallel, false);
    vr::VROverlay()->SetOverlayFlag(m_overlayHandle, vr::VROverlayFlags_NoDashboardTab, true);
    vr::VROverlay()->SetOverlayAlpha(m_overlayHandle, 0.9f);  // Slightly transparent for coolness

    // Enable input capture for gesture recognition
    vr::VROverlay()->SetOverlayFlag(m_overlayHandle, vr::VROverlayFlags_SendVRSmoothScrollEvents, true);
    vr::VROverlay()->SetOverlayFlag(m_overlayHandle, vr::VROverlayFlags_SendVRTouchpadEvents, true);

    LOG_INFO("SteamVR overlay created successfully - Audio Cockpit ready for display!");
    return true;
}

void AudioRoutingOverlay::DestroyOverlay() {
    if (m_overlayHandle != vr::k_ulOverlayHandleInvalid) {
        vr::VROverlay()->DestroyOverlay(m_overlayHandle);
        m_overlayHandle = vr::k_ulOverlayHandleInvalid;
    }

    if (m_thumbnailHandle != vr::k_ulOverlayHandleInvalid) {
        vr::VROverlay()->DestroyOverlay(m_thumbnailHandle);
        m_thumbnailHandle = vr::k_ulOverlayHandleInvalid;
    }
}

// === OPENGL RENDER TARGET MAGIC ===
bool AudioRoutingOverlay::CreateRenderTarget() {
    // Create framebuffer for rendering our beautiful UI
    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

    // Create color texture
    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_textureWidth, m_textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);

    // Create depth texture for 3D coolness
    glGenTextures(1, &m_depthTexture);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_textureWidth, m_textureHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);

    // Check framebuffer completeness
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("Framebuffer not complete: {}", (int)status);
        DestroyRenderTarget();
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    LOG_INFO("OpenGL render target created - ready for beautiful visuals!");
    return true;
}

void AudioRoutingOverlay::DestroyRenderTarget() {
    if (m_framebuffer != 0) {
        glDeleteFramebuffers(1, &m_framebuffer);
        m_framebuffer = 0;
    }
    if (m_colorTexture != 0) {
        glDeleteTextures(1, &m_colorTexture);
        m_colorTexture = 0;
    }
    if (m_depthTexture != 0) {
        glDeleteTextures(1, &m_depthTexture);
        m_depthTexture = 0;
    }
}

// === GESTURE RECOGNITION MAGIC ===
void AudioRoutingOverlay::UpdateControllerStates() {
    if (!vr::VRSystem()) return;

    // Find controller indices if we haven't already
    if (m_leftControllerIndex == vr::k_unTrackedDeviceIndexInvalid ||
        m_rightControllerIndex == vr::k_unTrackedDeviceIndexInvalid) {

        for (uint32_t i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i) {
            if (vr::VRSystem()->GetTrackedDeviceClass(i) == vr::TrackedDeviceClass_Controller) {
                auto role = vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(i);
                if (role == vr::TrackedControllerRole_LeftHand) {
                    m_leftControllerIndex = i;
                } else if (role == vr::TrackedControllerRole_RightHand) {
                    m_rightControllerIndex = i;
                }
            }
        }
    }

    // Update left controller state
    if (m_leftControllerIndex != vr::k_unTrackedDeviceIndexInvalid) {
        vr::VRControllerState_t controllerState;
        vr::TrackedDevicePose_t pose;

        if (vr::VRSystem()->GetControllerStateWithPose(vr::TrackingUniverseStanding,
                                                      m_leftControllerIndex,
                                                      &controllerState,
                                                      sizeof(controllerState),
                                                      &pose)) {
            // Extract position from pose matrix for now (simplified)
            Vec3 controllerPos{pose.mDeviceToAbsoluteTracking.m[0][3],
                               pose.mDeviceToAbsoluteTracking.m[1][3],
                               pose.mDeviceToAbsoluteTracking.m[2][3]};
            m_leftController.position = ControllerToWorldSpace(controllerPos, m_leftControllerIndex);
            m_leftController.trigger = controllerState.rAxis[1][0];  // Trigger axis
            m_leftController.wasGripping = m_leftController.isGripping;
            m_leftController.isGripping = m_leftController.trigger > 0.5f;
        }
    }

    // Update right controller state (similar logic)
    if (m_rightControllerIndex != vr::k_unTrackedDeviceIndexInvalid) {
        vr::VRControllerState_t controllerState;
        vr::TrackedDevicePose_t pose;

        if (vr::VRSystem()->GetControllerStateWithPose(vr::TrackingUniverseStanding,
                                                      m_rightControllerIndex,
                                                      &controllerState,
                                                      sizeof(controllerState),
                                                      &pose)) {
            // Extract position from pose matrix for now (simplified)
            Vec3 controllerPos{pose.mDeviceToAbsoluteTracking.m[0][3],
                               pose.mDeviceToAbsoluteTracking.m[1][3],
                               pose.mDeviceToAbsoluteTracking.m[2][3]};
            m_rightController.position = ControllerToWorldSpace(controllerPos, m_rightControllerIndex);
            m_rightController.trigger = controllerState.rAxis[1][0];
            m_rightController.wasGripping = m_rightController.isGripping;
            m_rightController.isGripping = m_rightController.trigger > 0.5f;
        }
    }
}

void AudioRoutingOverlay::ProcessGestures() {
    // Process pinch gestures for grabbing audio orbs
    DetectPinchGesture(m_leftController);
    DetectPinchGesture(m_rightController);

    // Process swipe gestures for quick actions
    DetectSwipeGesture(m_leftController);
    DetectSwipeGesture(m_rightController);

    // Process twist gestures for parameter adjustment
    DetectTwistGesture(m_leftController);
    DetectTwistGesture(m_rightController);

    // Process clap gesture for global mute/unmute
    DetectClapGesture();
}

void AudioRoutingOverlay::DetectPinchGesture(ControllerState& controller) {
    // Check if controller just started gripping
    if (controller.isGripping && !controller.wasGripping) {
        // Look for audio orbs near the controller
        AudioSource* nearbyOrb = GetOrbAtPosition(controller.position, GRAB_DISTANCE);
        if (nearbyOrb) {
            controller.grabbedSource = nearbyOrb;
            HandleOrbGrab(nearbyOrb, controller.position);

            // Notify gesture callbacks
            for (auto& callback : m_gestureCallbacks) {
                callback("pinch_start", controller.position);
            }
        }
    }
    // Check if controller just stopped gripping
    else if (!controller.isGripping && controller.wasGripping) {
        if (controller.grabbedSource) {
            HandleOrbRelease(controller.grabbedSource);
            controller.grabbedSource = nullptr;

            for (auto& callback : m_gestureCallbacks) {
                callback("pinch_end", controller.position);
            }
        }
    }
    // If currently grabbing, update orb position
    else if (controller.isGripping && controller.grabbedSource) {
        controller.grabbedSource->orbPosition = controller.position;
    }
}

// === AUDIO ORB MANAGEMENT ===
void AudioRoutingOverlay::UpdateAudioOrbs() {
    for (auto& [name, source] : m_audioSources) {
        // Update orb size based on audio level (fake it for now!)
        // TODO: Get real audio levels from AudioEngine
        float fakeAudioLevel = 0.5f + 0.5f * std::sin(m_uiState.pulseTimer * 2.0f +
                                                      std::hash<std::string>{}(name) % 100);
        source->orbSize = AUDIO_ORB_BASE_SIZE + (AUDIO_ORB_MAX_SIZE - AUDIO_ORB_BASE_SIZE) * fakeAudioLevel;

        // Update orb color based on frequency content (also fake for now!)
        float hue = (std::hash<std::string>{}(name) % 360) / 360.0f;
        source->orbColor = HSVToRGB(hue, 0.8f, 0.8f + 0.2f * fakeAudioLevel);

        // Pulse animation
        source->orbPulse = std::sin(m_uiState.pulseTimer * 4.0f) * 0.1f;
    }

    m_uiState.pulseTimer += 0.016f;  // Assuming ~60 FPS
}

AudioRoutingOverlay::AudioSource* AudioRoutingOverlay::GetOrbAtPosition(const Vec3& position, float tolerance) {
    for (auto& [name, source] : m_audioSources) {
        float distance = CalculateDistance(position, source->orbPosition);
        if (distance <= tolerance) {
            return source.get();
        }
    }
    return nullptr;
}

void AudioRoutingOverlay::HandleOrbGrab(AudioSource* source, const Vec3& controllerPos) {
    LOG_INFO("Grabbed audio orb: {}", source->name);
    // TODO: Add haptic feedback
    // TODO: Notify audio engine about orb interaction
}

void AudioRoutingOverlay::HandleOrbRelease(AudioSource* source) {
    LOG_INFO("Released audio orb: {}", source->name);
    // TODO: Apply spatial position to audio engine
    // TODO: Add haptic feedback
}

// === CREATIVE SHORTCUTS & INITIALIZATION ===
void AudioRoutingOverlay::InitializePresets() {
    m_hrtfPresets[AudioPreset::CINEMA] = {
        .roomSize = 15.0f,
        .reverbMix = 0.3f,
        .distanceAttenuation = 1.2f,
        .nearFieldCompensation = true,
        .displayName = "Cinema Experience"
    };

    m_hrtfPresets[AudioPreset::CONCERT] = {
        .roomSize = 50.0f,
        .reverbMix = 0.6f,
        .distanceAttenuation = 1.5f,
        .nearFieldCompensation = false,
        .displayName = "Live Concert"
    };

    m_hrtfPresets[AudioPreset::PODCAST] = {
        .roomSize = 5.0f,
        .reverbMix = 0.1f,
        .distanceAttenuation = 0.8f,
        .nearFieldCompensation = true,
        .displayName = "Intimate Voice"
    };

    m_hrtfPresets[AudioPreset::GAMING] = {
        .roomSize = 10.0f,
        .reverbMix = 0.2f,
        .distanceAttenuation = 1.0f,
        .nearFieldCompensation = true,
        .displayName = "Competitive Gaming"
    };

    m_hrtfPresets[AudioPreset::MUSIC] = {
        .roomSize = 20.0f,
        .reverbMix = 0.25f,
        .distanceAttenuation = 1.1f,
        .nearFieldCompensation = true,
        .displayName = "Hi-Fi Listening"
    };

    LOG_INFO("Initialized {} HRTF presets for instant audio magic!", m_hrtfPresets.size());
}

// === QUICK STUB IMPLEMENTATIONS ===
// These are the "make it work" implementations - we'll polish later!

void AudioRoutingOverlay::SetVisible(bool visible) {
    m_visible = visible;
    if (m_overlayHandle != vr::k_ulOverlayHandleInvalid) {
        vr::VROverlay()->SetOverlayFlag(m_overlayHandle, vr::VROverlayFlags_HideLaserIntersection, !visible);
    }
}

void AudioRoutingOverlay::SetPosition(const Vec3& position) {
    m_position = position;
    UpdateOverlayTransform();
}

void AudioRoutingOverlay::SetScale(float scale) {
    m_scale = scale;
    if (m_overlayHandle != vr::k_ulOverlayHandleInvalid) {
        vr::VROverlay()->SetOverlayWidthInMeters(m_overlayHandle, 2.0f * scale);
    }
}

void AudioRoutingOverlay::AddAudioSource(const std::string& name) {
    auto source = std::make_unique<AudioSource>();
    source->name = name;
    m_audioSources[name] = std::move(source);
    LOG_INFO("Added audio source: {}", name);
}

void AudioRoutingOverlay::RemoveAudioSource(const std::string& name) {
    m_audioSources.erase(name);
    LOG_INFO("Removed audio source: {}", name);
}

AudioRoutingOverlay::AudioSource* AudioRoutingOverlay::GetAudioSource(const std::string& name) {
    auto it = m_audioSources.find(name);
    return it != m_audioSources.end() ? it->second.get() : nullptr;
}

void AudioRoutingOverlay::ApplyPreset(AudioPreset preset) {
    auto it = m_hrtfPresets.find(preset);
    if (it != m_hrtfPresets.end()) {
        const auto& presetData = it->second;
        LOG_INFO("Applying preset: {}", presetData.displayName);

        // TODO: Apply preset to audio engine
        // m_audioEngine->SetRoomSize(presetData.roomSize);
        // m_audioEngine->SetReverbMix(presetData.reverbMix);
        // etc.

        m_uiState.currentPreset = preset;
    }
}

// === HELPER METHODS ===
float AudioRoutingOverlay::CalculateDistance(const Vec3& a, const Vec3& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

Vec3 AudioRoutingOverlay::HSVToRGB(float h, float s, float v) {
    // Quick HSV to RGB conversion for pretty colors!
    float c = v * s;
    float x = c * (1 - std::abs(std::fmod(h * 6, 2) - 1));
    float m = v - c;

    float r, g, b;
    if (h < 1.0f/6.0f) { r = c; g = x; b = 0; }
    else if (h < 2.0f/6.0f) { r = x; g = c; b = 0; }
    else if (h < 3.0f/6.0f) { r = 0; g = c; b = x; }
    else if (h < 4.0f/6.0f) { r = 0; g = x; b = c; }
    else if (h < 5.0f/6.0f) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }

    return {r + m, g + m, b + m};
}

Vec3 AudioRoutingOverlay::ControllerToWorldSpace(const Vec3& position, uint32_t deviceIndex) {
    // Get controller pose and convert to world space
    vr::TrackedDevicePose_t pose;
    vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, 0, &pose, 1);

    // Extract position from transformation matrix (simplified!)
    return Vec3{
        pose.mDeviceToAbsoluteTracking.m[0][3],
        pose.mDeviceToAbsoluteTracking.m[1][3],
        pose.mDeviceToAbsoluteTracking.m[2][3]
    };
}

// === RENDERING STUBS ===
// These are placeholder implementations - we'll make them beautiful later!

void AudioRoutingOverlay::RenderToTexture() {
    if (m_framebuffer == 0) return;

    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glViewport(0, 0, m_textureWidth, m_textureHeight);

    // Clear to a nice dark background
    glClearColor(0.1f, 0.1f, 0.2f, 0.9f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: Render the beautiful Audio Cockpit UI here!
    // For now, just render some debug info
    if (m_debugMode) {
        RenderDebugInfo();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void AudioRoutingOverlay::UpdateOverlayTexture() {
    if (m_overlayHandle == vr::k_ulOverlayHandleInvalid || m_colorTexture == 0) return;

    vr::Texture_t eyeTexture = { (void*)(uintptr_t)m_colorTexture, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
    vr::VROverlay()->SetOverlayTexture(m_overlayHandle, &eyeTexture);
}

void AudioRoutingOverlay::UpdateOverlayTransform() {
    if (m_overlayHandle == vr::k_ulOverlayHandleInvalid) return;

    // Set overlay position in world space
    m_overlayTransform.m[0][3] = m_position.x;
    m_overlayTransform.m[1][3] = m_position.y;
    m_overlayTransform.m[2][3] = m_position.z;

    vr::VROverlay()->SetOverlayTransformAbsolute(m_overlayHandle, vr::TrackingUniverseStanding, &m_overlayTransform);
}

// === MORE CREATIVE STUBS ===
// These methods will be implemented as we iterate and refine!

void AudioRoutingOverlay::DetectSwipeGesture(ControllerState& controller) {
    // TODO: Implement swipe detection for quick actions
}

void AudioRoutingOverlay::DetectTwistGesture(ControllerState& controller) {
    // TODO: Implement twist detection for parameter adjustment
}

void AudioRoutingOverlay::DetectClapGesture() {
    // TODO: Implement two-handed clap detection
}

void AudioRoutingOverlay::UpdateParticleSystem() {
    // TODO: Update beautiful particle visualization
}

void AudioRoutingOverlay::UpdateAudioVisualization() {
    // TODO: Update real-time audio visualization data
}

void AudioRoutingOverlay::UpdateMetrics() {
    // TODO: Update performance metrics
}

void AudioRoutingOverlay::HandleAutoHide() {
    // TODO: Implement auto-hide functionality
}

void AudioRoutingOverlay::SyncToCompositor() {
    // TODO: Sync timing to VR compositor for smooth rendering
}

void AudioRoutingOverlay::RegisterGestureCallback(GestureCallback callback) {
    m_gestureCallbacks.push_back(callback);
}

void AudioRoutingOverlay::AttachToController(bool enabled) {
    m_attachToController = enabled;
    // TODO: Implement controller attachment
}

void AudioRoutingOverlay::RenderDebugInfo() {
    // TODO: Render debug information overlay
}

std::vector<AudioRoutingOverlay::AudioSource*> AudioRoutingOverlay::GetAllAudioSources() {
    std::vector<AudioRoutingOverlay::AudioSource*> sources;
    for (auto& [name, source] : m_audioSources) {
        sources.push_back(source.get());
    }
    return sources;
}

void AudioRoutingOverlay::UpdateGestureDetection(const std::vector<VRPose>& controllers) {
    if (!m_initialized.load()) {
        return;
    }

    // Update controller states from VR tracking data
    // This integrates the Audio Cockpit with the 90Hz VR tracking thread
    if (controllers.size() >= 1) {
        m_leftController.position = controllers[0].position;
        m_leftController.wasGripping = m_leftController.isGripping;
        // Simple trigger detection based on controller validity for now
        m_leftController.isGripping = controllers[0].isValid;
    }

    if (controllers.size() >= 2) {
        m_rightController.position = controllers[1].position;
        m_rightController.wasGripping = m_rightController.isGripping;
        m_rightController.isGripping = controllers[1].isValid;
    }

    // Process gesture detection with updated controller states
    ProcessGestures();
}

void AudioRoutingOverlay::UpdateAudioOrbPositions(const VRPose& hmdPose) {
    if (!m_initialized.load()) {
        return;
    }

    // Update audio orb positions relative to HMD for spatial audio
    // This creates a dynamic audio field around the user's head
    float angle = 0.0f;
    float radius = 0.5f; // Half meter circle around user

    for (auto& [name, source] : m_audioSources) {
        // Check if this source is currently grabbed by either controller
        bool isGrabbed = (m_leftController.grabbedSource == source.get()) ||
                        (m_rightController.grabbedSource == source.get());

        if (!isGrabbed) { // Only update if not grabbed by user
            // Position orbs in a circle around the HMD
            source->orbPosition = Vec3{
                hmdPose.position.x + std::sin(angle) * radius,
                hmdPose.position.y, // Keep at head level
                hmdPose.position.z + std::cos(angle) * radius
            };
        }
        angle += TWO_PI / m_audioSources.size();
    }
}

void AudioRoutingOverlay::SetHRTFProcessor(HRTFProcessor* processor) {
    m_hrtfProcessor = processor;
    LOG_INFO("Audio Routing Overlay connected to HRTF processor - spatial audio magic enabled!");
}

} // namespace vrb