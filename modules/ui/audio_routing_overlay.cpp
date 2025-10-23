// audio_routing_overlay.cpp - Simple ASMRtist Microphone Positioning
// Simple tools for content creators to position their mic in VR space

#include "audio_routing_overlay.h"
#include "../../core/include/logger.h"
#include <algorithm>
#include <cmath>
#include <cstring>

// Simple OpenGL includes
#ifdef _WIN32
    #include <GL/glew.h>
#else
    #define GL_GLEXT_PROTOTYPES
    #include <GL/gl.h>
    #include <GL/glext.h>
#endif

namespace vrb {

// Simple constants for ASMRtist workflow
constexpr float PI = 3.14159265359f;
constexpr float MIC_VISUAL_SIZE = 0.05f;       // Small, non-intrusive mic representation
constexpr float DRAG_TOLERANCE = 0.2f;        // How close controller needs to be to grab mic
constexpr float AUTO_HIDE_DELAY = 3.0f;       // Hide UI after 3 seconds of no interaction

AudioRoutingOverlay::AudioRoutingOverlay() {
    // Initialize overlay transform to identity
    memset(&m_overlayTransform, 0, sizeof(m_overlayTransform));
    m_overlayTransform.m[0][0] = 1.0f;
    m_overlayTransform.m[1][1] = 1.0f;
    m_overlayTransform.m[2][2] = 1.0f;

    // Set default microphone position in front of user at head height
    m_virtualMic.position = Vec3{0, 1.5f, -1.0f};  // 1 meter in front, head height
    m_virtualMic.orientation = Vec3{0, 0, -1};      // Facing forward

    LOG_INFO("AudioRoutingOverlay initialized for ASMRtist workflow");
}

AudioRoutingOverlay::~AudioRoutingOverlay() {
    Shutdown();
}

bool AudioRoutingOverlay::Initialize(VRTracker* vrTracker, AudioEngine* audioEngine) {
    if (m_initialized.load()) {
        LOG_WARN("AudioRoutingOverlay already initialized!");
        return true;
    }

    LOG_INFO("Initializing simple VR microphone positioning interface");

    m_vrTracker = vrTracker;
    m_audioEngine = audioEngine;

    // Create basic SteamVR overlay for minimal UI
    if (!CreateOverlay()) {
        LOG_ERROR("Failed to create VR overlay");
        return false;
    }

    // Create simple render target
    if (!CreateRenderTarget()) {
        LOG_ERROR("Failed to create render target");
        return false;
    }

    m_initialized = true;
    LOG_INFO("VR microphone positioning ready for ASMRtists!");
    return true;
}

void AudioRoutingOverlay::Update() {
    if (!m_initialized.load()) {
        return;
    }

    // Simple controller tracking for microphone positioning
    UpdateControllerStates();

    // Handle microphone drag-to-position
    HandleMicrophonePositioning();

    // Update audio levels for monitoring
    UpdateAudioLevels();

    // Handle auto-hide for minimal interface
    HandleAutoHide();

    // Update visual display
    if (m_visible.load()) {
        RenderToTexture();
        UpdateOverlayTexture();
        UpdateOverlayTransform();
    }
}

void AudioRoutingOverlay::Shutdown() {
    if (!m_initialized.load()) {
        return;
    }

    DestroyRenderTarget();
    DestroyOverlay();

    m_initialized = false;
    LOG_INFO("AudioRoutingOverlay shutdown complete");
}

// === ASMR-FOCUSED FUNCTIONALITY ===

void AudioRoutingOverlay::SetMicrophonePosition(const Vec3& position) {
    m_virtualMic.position = position;

    // Update HRTF processor if connected
    if (m_hrtfProcessor) {
        // Convert microphone position to listener position for spatial processing
        m_hrtfProcessor->SetListenerPosition(position);
    }

    LOG_DEBUG("Microphone position set to ({}, {}, {})", position.x, position.y, position.z);
}

void AudioRoutingOverlay::SetMicrophoneOrientation(const Vec3& forward) {
    m_virtualMic.orientation = forward;

    if (m_hrtfProcessor) {
        m_hrtfProcessor->SetListenerOrientation(forward);
    }
}

Vec3 AudioRoutingOverlay::GetMicrophonePosition() const {
    return m_virtualMic.position;
}

void AudioRoutingOverlay::StartRecording() {
    m_isRecording = true;
    m_virtualMic.isActive = true;
    m_uiState.lastInteraction = std::chrono::steady_clock::now();  // Show UI during recording

    LOG_INFO("Recording started - microphone active at position ({}, {}, {})",
             m_virtualMic.position.x, m_virtualMic.position.y, m_virtualMic.position.z);
}

void AudioRoutingOverlay::StopRecording() {
    m_isRecording = false;
    m_virtualMic.isActive = false;

    LOG_INFO("Recording stopped");
}

bool AudioRoutingOverlay::IsRecording() const {
    return m_isRecording.load();
}

void AudioRoutingOverlay::SetMonitoring(bool enabled) {
    m_uiState.monitoring = enabled;

    if (enabled) {
        LOG_INFO("Audio monitoring enabled - you can hear spatial effects in real-time");
    } else {
        LOG_INFO("Audio monitoring disabled");
    }
}

float AudioRoutingOverlay::GetInputLevel() const {
    return m_inputLevel.load();
}

float AudioRoutingOverlay::GetSpatializedOutputLevel() const {
    return m_outputLevel.load();
}

// === SIMPLE VR INTERACTION ===

void AudioRoutingOverlay::UpdateControllerTracking(const std::vector<VRPose>& controllers) {
    // Update controller positions for simple microphone positioning
    if (controllers.size() >= 1) {
        m_leftController.position = controllers[0].position;
        m_leftController.triggerPressed = controllers[0].trigger > 0.5f;
    }

    if (controllers.size() >= 2) {
        m_rightController.position = controllers[1].position;
        m_rightController.triggerPressed = controllers[1].trigger > 0.5f;
    }
}

void AudioRoutingOverlay::UpdateMicrophoneTracking(const VRPose& hmdPose) {
    // Optional: Auto-position microphone relative to head movement
    // This could be useful for some ASMRtist workflows
}

void AudioRoutingOverlay::UpdateControllerStates() {
    // Simple state tracking for trigger press/release
    m_leftController.wasTriggered = m_leftController.triggerPressed;
    m_rightController.wasTriggered = m_rightController.triggerPressed;
}

void AudioRoutingOverlay::HandleMicrophonePositioning() {
    // Check if either controller is near the microphone and trigger is pressed
    bool leftNearMic = IsControllerNearMicrophone(m_leftController.position);
    bool rightNearMic = IsControllerNearMicrophone(m_rightController.position);

    // Start dragging microphone
    if ((leftNearMic && m_leftController.triggerPressed && !m_leftController.wasTriggered) ||
        (rightNearMic && m_rightController.triggerPressed && !m_rightController.wasTriggered)) {

        if (leftNearMic) m_leftController.isDraggingMic = true;
        if (rightNearMic) m_rightController.isDraggingMic = true;

        m_uiState.lastInteraction = std::chrono::steady_clock::now();
        LOG_DEBUG("Started dragging microphone");
    }

    // Update microphone position while dragging
    if (m_leftController.isDraggingMic && m_leftController.triggerPressed) {
        SetMicrophonePosition(m_leftController.position);
    } else if (m_rightController.isDraggingMic && m_rightController.triggerPressed) {
        SetMicrophonePosition(m_rightController.position);
    }

    // Stop dragging when trigger is released
    if (m_leftController.isDraggingMic && !m_leftController.triggerPressed) {
        m_leftController.isDraggingMic = false;
        LOG_DEBUG("Stopped dragging microphone with left controller");
    }
    if (m_rightController.isDraggingMic && !m_rightController.triggerPressed) {
        m_rightController.isDraggingMic = false;
        LOG_DEBUG("Stopped dragging microphone with right controller");
    }
}

// === SIMPLE UI RENDERING ===

void AudioRoutingOverlay::RenderToTexture() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glViewport(0, 0, m_textureWidth, m_textureHeight);

    // Clear with transparent background
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Only render if UI should be visible
    if (m_uiState.showMicrophone) {
        RenderMicrophoneVisual();
    }

    if (m_uiState.showControls) {
        RenderSimpleControls();
        RenderRecordingStatus();
    }

    if (m_uiState.showLevelMeter) {
        RenderAudioLevelMeter();
    }

    if (m_debugMode) {
        RenderDebugInfo();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void AudioRoutingOverlay::RenderMicrophoneVisual() {
    // Simple microphone representation using basic OpenGL shapes
    // Color coding: White when inactive, Green when recording, Yellow when monitoring
    Vec3 micColor = m_virtualMic.isActive ? Vec3{0, 1, 0} : Vec3{0.8f, 0.8f, 0.8f};
    if (m_uiState.monitoring) micColor = Vec3{1, 1, 0};  // Yellow when monitoring

    // Enable depth testing for proper 3D rendering
    glEnable(GL_DEPTH_TEST);

    // Set microphone color
    glColor3f(micColor.x, micColor.y, micColor.z);

    // Save current matrix state
    glPushMatrix();

    // Translate to microphone position (relative to overlay coordinate system)
    // Convert world mic position to overlay-relative position
    float relativeX = m_virtualMic.position.x - m_position.x;
    float relativeY = m_virtualMic.position.y - m_position.y;
    float relativeZ = m_virtualMic.position.z - m_position.z;

    glTranslatef(relativeX * 100.0f, relativeY * 100.0f, relativeZ * 100.0f);  // Scale for overlay space

    // Render simple microphone shape using basic OpenGL primitives
    // Main microphone body (cylinder approximation using quad strip)
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= 8; i++) {
        float angle = (float)i * PI / 4.0f;
        float x = cos(angle) * MIC_VISUAL_SIZE;
        float z = sin(angle) * MIC_VISUAL_SIZE;

        glVertex3f(x, -MIC_VISUAL_SIZE, z);  // Bottom
        glVertex3f(x, MIC_VISUAL_SIZE, z);   // Top
    }
    glEnd();

    // Microphone grille (top cap)
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0, MIC_VISUAL_SIZE, 0);  // Center top
    for (int i = 0; i <= 8; i++) {
        float angle = (float)i * PI / 4.0f;
        float x = cos(angle) * MIC_VISUAL_SIZE;
        float z = sin(angle) * MIC_VISUAL_SIZE;
        glVertex3f(x, MIC_VISUAL_SIZE, z);
    }
    glEnd();

    // Recording indicator (pulsing effect when active)
    if (m_virtualMic.isActive) {
        float pulse = 1.0f + 0.3f * sin(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count() * 0.01f);

        glScalef(pulse, pulse, pulse);
        glColor3f(1.0f, 0.2f, 0.2f);  // Red pulse

        // Small pulsing sphere around mic
        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= 16; i++) {
            float angle = (float)i * PI / 8.0f;
            float x = cos(angle) * MIC_VISUAL_SIZE * 1.5f;
            float y = sin(angle) * MIC_VISUAL_SIZE * 1.5f;
            glVertex3f(x, y, 0);
            glVertex3f(x, y, MIC_VISUAL_SIZE * 0.5f);
        }
        glEnd();
    }

    // Restore matrix state
    glPopMatrix();

    // Reset color to white
    glColor3f(1.0f, 1.0f, 1.0f);

    LOG_DEBUG("Rendered microphone visual at relative position ({:.2f}, {:.2f}, {:.2f}) with color ({:.1f}, {:.1f}, {:.1f})",
              relativeX, relativeY, relativeZ, micColor.x, micColor.y, micColor.z);
}

void AudioRoutingOverlay::RenderRecordingStatus() {
    // Creative solution: Simple OpenGL-based recording indicator
    if (m_isRecording.load()) {
        glPushMatrix();

        // Position indicator in top-left of overlay
        glTranslatef(-350.0f, 300.0f, 0.0f);

        // Pulsing red "REC" indicator - creative visual feedback!
        float pulse = 1.0f + 0.5f * sin(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count() * 0.01f);

        glColor3f(pulse, 0.0f, 0.0f);  // Pulsing red

        // Draw "REC" using simple rectangles - creative ASCII art approach!
        // R shape
        glBegin(GL_QUADS);
        glVertex2f(0, 0); glVertex2f(15, 0); glVertex2f(15, 30); glVertex2f(0, 30);  // Vertical
        glVertex2f(0, 30); glVertex2f(30, 30); glVertex2f(30, 25); glVertex2f(0, 25); // Top horizontal
        glVertex2f(0, 15); glVertex2f(25, 15); glVertex2f(25, 10); glVertex2f(0, 10); // Middle horizontal
        glEnd();

        // E shape
        glTranslatef(40.0f, 0.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0); glVertex2f(15, 0); glVertex2f(15, 30); glVertex2f(0, 30);    // Vertical
        glVertex2f(0, 30); glVertex2f(30, 30); glVertex2f(30, 25); glVertex2f(0, 25); // Top
        glVertex2f(0, 15); glVertex2f(25, 15); glVertex2f(25, 10); glVertex2f(0, 10); // Middle
        glVertex2f(0, 5); glVertex2f(30, 5); glVertex2f(30, 0); glVertex2f(0, 0);     // Bottom
        glEnd();

        // C shape
        glTranslatef(40.0f, 0.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(0, 5); glVertex2f(15, 5); glVertex2f(15, 25); glVertex2f(0, 25);   // Vertical
        glVertex2f(0, 25); glVertex2f(30, 25); glVertex2f(30, 30); glVertex2f(0, 30); // Top
        glVertex2f(0, 0); glVertex2f(30, 0); glVertex2f(30, 5); glVertex2f(0, 5);     // Bottom
        glEnd();

        glPopMatrix();

        LOG_DEBUG("Rendered pulsing REC indicator with pulse factor {:.2f}", pulse);
    }

    // Reset color
    glColor3f(1.0f, 1.0f, 1.0f);
}

void AudioRoutingOverlay::RenderSimpleControls() {
    // Creative solution: Simple 3D buttons that ASMRtists can trigger press with controllers!

    glPushMatrix();

    // Position controls below microphone at comfortable reach
    glTranslatef(0.0f, -200.0f, 0.0f);

    // Record/Stop button - biggest and most important
    bool nearRecordButton = IsControllerNearPosition(Vec3{0, -2.0f, -1.0f}, 0.3f);

    glPushMatrix();
    glTranslatef(-100.0f, 0.0f, 0.0f);

    // Button color based on state and controller proximity
    if (m_isRecording.load()) {
        glColor3f(1.0f, 0.2f, 0.2f);  // Red when recording
    } else if (nearRecordButton) {
        glColor3f(0.2f, 1.0f, 0.2f);  // Bright green when controller nearby
    } else {
        glColor3f(0.5f, 0.8f, 0.5f);  // Dim green normally
    }

    // Draw 3D button using creative cube approach
    float buttonSize = nearRecordButton ? 35.0f : 30.0f;  // Grows when controller near
    glBegin(GL_QUADS);
    // Front face
    glVertex3f(-buttonSize, -buttonSize, buttonSize);
    glVertex3f(buttonSize, -buttonSize, buttonSize);
    glVertex3f(buttonSize, buttonSize, buttonSize);
    glVertex3f(-buttonSize, buttonSize, buttonSize);
    // Back face
    glVertex3f(-buttonSize, -buttonSize, -buttonSize);
    glVertex3f(-buttonSize, buttonSize, -buttonSize);
    glVertex3f(buttonSize, buttonSize, -buttonSize);
    glVertex3f(buttonSize, -buttonSize, -buttonSize);
    glEnd();

    // Check for button press - creative trigger detection!
    if (nearRecordButton && (m_leftController.triggerPressed || m_rightController.triggerPressed)) {
        if (!m_recordButtonPressed) {
            m_recordButtonPressed = true;
            ToggleRecording();  // This will call StartRecording() or StopRecording()
            LOG_INFO("ASMRtist {} recording via VR button!", m_isRecording.load() ? "started" : "stopped");
        }
    } else {
        m_recordButtonPressed = false;
    }

    glPopMatrix();

    // Monitoring toggle button
    bool nearMonitorButton = IsControllerNearPosition(Vec3{1.0f, -2.0f, -1.0f}, 0.3f);

    glPushMatrix();
    glTranslatef(100.0f, 0.0f, 0.0f);

    if (m_uiState.monitoring) {
        glColor3f(1.0f, 1.0f, 0.0f);  // Yellow when monitoring
    } else if (nearMonitorButton) {
        glColor3f(0.8f, 0.8f, 1.0f);  // Light blue when controller nearby
    } else {
        glColor3f(0.4f, 0.4f, 0.6f);  // Dim blue normally
    }

    float monitorSize = nearMonitorButton ? 25.0f : 20.0f;
    glBegin(GL_QUADS);
    // Simple speaker icon using rectangles
    glVertex3f(-monitorSize, -monitorSize, 0);
    glVertex3f(-monitorSize/2, -monitorSize, 0);
    glVertex3f(-monitorSize/2, monitorSize, 0);
    glVertex3f(-monitorSize, monitorSize, 0);

    glVertex3f(-monitorSize/2, -monitorSize/2, 0);
    glVertex3f(monitorSize/2, -monitorSize/2, 0);
    glVertex3f(monitorSize/2, monitorSize/2, 0);
    glVertex3f(-monitorSize/2, monitorSize/2, 0);
    glEnd();

    // Check for monitor button press
    if (nearMonitorButton && (m_leftController.triggerPressed || m_rightController.triggerPressed)) {
        if (!m_monitorButtonPressed) {
            m_monitorButtonPressed = true;
            SetMonitoring(!m_uiState.monitoring);
            LOG_INFO("ASMRtist {} monitoring via VR button!", m_uiState.monitoring ? "enabled" : "disabled");
        }
    } else {
        m_monitorButtonPressed = false;
    }

    glPopMatrix();
    glPopMatrix();

    // Reset color
    glColor3f(1.0f, 1.0f, 1.0f);

    LOG_DEBUG("Rendered VR controls - Record button near: {}, Monitor button near: {}", nearRecordButton, nearMonitorButton);
}

void AudioRoutingOverlay::RenderAudioLevelMeter() {
    // Creative solution: Real-time audio level bars that ASMRtists can see while recording!

    float inputLevel = GetInputLevel();
    float outputLevel = GetSpatializedOutputLevel();

    glPushMatrix();

    // Position meters at top-right of overlay
    glTranslatef(200.0f, 250.0f, 0.0f);

    // Input level meter (left side)
    glPushMatrix();
    glTranslatef(-50.0f, 0.0f, 0.0f);

    // Background bar
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(20, 0); glVertex2f(20, 200); glVertex2f(0, 200);
    glEnd();

    // Input level fill - green to red gradient based on level
    float levelHeight = inputLevel * 200.0f;
    float greenComponent = std::max(0.0f, 1.0f - inputLevel * 2.0f);  // Fade out green as level increases
    float redComponent = std::min(1.0f, inputLevel * 2.0f);           // Fade in red as level increases

    glColor3f(redComponent, greenComponent, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(2, 2); glVertex2f(18, 2); glVertex2f(18, 2 + levelHeight); glVertex2f(2, 2 + levelHeight);
    glEnd();

    // Peak indicator - creative white line at peak level
    static float peakLevel = 0.0f;
    static auto lastPeakTime = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();

    if (inputLevel > peakLevel) {
        peakLevel = inputLevel;
        lastPeakTime = now;
    } else if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastPeakTime).count() > 500) {
        peakLevel = std::max(0.0f, peakLevel - 0.01f);  // Slow decay
    }

    if (peakLevel > 0.01f) {
        glColor3f(1.0f, 1.0f, 1.0f);
        float peakY = 2 + peakLevel * 200.0f;
        glBegin(GL_LINES);
        glVertex2f(0, peakY); glVertex2f(20, peakY);
        glEnd();
    }

    glPopMatrix();

    // Output level meter (right side) - shows spatialized result
    glPushMatrix();
    glTranslatef(50.0f, 0.0f, 0.0f);

    // Background bar
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(20, 0); glVertex2f(20, 200); glVertex2f(0, 200);
    glEnd();

    // Output level fill - blue to cyan for spatial output
    float outputHeight = outputLevel * 200.0f;
    glColor3f(0.0f, 0.5f + outputLevel * 0.5f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(2, 2); glVertex2f(18, 2); glVertex2f(18, 2 + outputHeight); glVertex2f(2, 2 + outputHeight);
    glEnd();

    glPopMatrix();

    // Labels using simple line art
    glColor3f(0.8f, 0.8f, 0.8f);

    // "IN" label (left meter)
    glPushMatrix();
    glTranslatef(-55.0f, -25.0f, 0.0f);
    glBegin(GL_LINES);
    // I
    glVertex2f(0, 0); glVertex2f(0, 10);
    glVertex2f(-2, 0); glVertex2f(2, 0);
    glVertex2f(-2, 10); glVertex2f(2, 10);
    // N
    glVertex2f(5, 0); glVertex2f(5, 10);
    glVertex2f(5, 10); glVertex2f(10, 0);
    glVertex2f(10, 0); glVertex2f(10, 10);
    glEnd();
    glPopMatrix();

    // "OUT" label (right meter)
    glPushMatrix();
    glTranslatef(45.0f, -25.0f, 0.0f);
    glBegin(GL_LINES);
    // O
    glVertex2f(0, 0); glVertex2f(0, 10);
    glVertex2f(0, 10); glVertex2f(5, 10);
    glVertex2f(5, 10); glVertex2f(5, 0);
    glVertex2f(5, 0); glVertex2f(0, 0);
    // U
    glVertex2f(7, 10); glVertex2f(7, 2);
    glVertex2f(7, 2); glVertex2f(12, 2);
    glVertex2f(12, 2); glVertex2f(12, 10);
    // T
    glVertex2f(14, 10); glVertex2f(19, 10);
    glVertex2f(16.5f, 10); glVertex2f(16.5f, 0);
    glEnd();
    glPopMatrix();

    glPopMatrix();

    // Reset color
    glColor3f(1.0f, 1.0f, 1.0f);

    LOG_DEBUG("Rendered audio meters - Input: {:.2f}, Output: {:.2f}, Peak: {:.2f}", inputLevel, outputLevel, peakLevel);
}

// === HELPER FUNCTIONS ===

void AudioRoutingOverlay::UpdateAudioLevels() {
    // Get audio levels from audio engine
    if (m_audioEngine) {
        m_inputLevel = m_audioEngine->GetInputLevel();
        // Calculate spatialized output level
        m_outputLevel = (m_audioEngine->GetOutputLevelLeft() + m_audioEngine->GetOutputLevelRight()) * 0.5f;
    }
}

void AudioRoutingOverlay::HandleAutoHide() {
    if (!m_uiState.autoHide) return;

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_uiState.lastInteraction).count();

    // Hide UI after delay, but keep visible during recording
    if (elapsed > m_uiState.autoHideDelayMs && !m_isRecording.load()) {
        m_uiState.showControls = false;
        m_uiState.showLevelMeter = false;
    } else if (elapsed < 100) {  // Show UI when there's recent interaction
        m_uiState.showControls = true;
        m_uiState.showLevelMeter = true;
    }
}

float AudioRoutingOverlay::CalculateDistance(const Vec3& a, const Vec3& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

bool AudioRoutingOverlay::IsControllerNearMicrophone(const Vec3& controllerPos, float tolerance) {
    return CalculateDistance(controllerPos, m_virtualMic.position) <= tolerance;
}

void AudioRoutingOverlay::SetHRTFProcessor(HRTFProcessor* processor) {
    m_hrtfProcessor = processor;

    // Update processor with current microphone position
    if (processor) {
        processor->SetListenerPosition(m_virtualMic.position);
        processor->SetListenerOrientation(m_virtualMic.orientation);
    }
}

void AudioRoutingOverlay::SetVisible(bool visible) {
    m_visible = visible;
    if (visible) {
        m_uiState.lastInteraction = std::chrono::steady_clock::now();
    }
}

// === STUB IMPLEMENTATIONS FOR VR OVERLAY ===

bool AudioRoutingOverlay::CreateOverlay() {
    LOG_INFO("Creating real OpenVR overlay for ASMRtist microphone positioning");

    // Check if OpenVR is available
    if (!vr::VRSystem()) {
        LOG_ERROR("OpenVR system not initialized - cannot create overlay");
        return false;
    }

    // Create the main overlay for microphone positioning UI
    vr::EVROverlayError overlayError = vr::VROverlay()->CreateOverlay("vrb.asmr.mic_position", "ASMR Microphone", &m_overlayHandle);
    if (overlayError != vr::VROverlayError_None) {
        LOG_ERROR("Failed to create OpenVR overlay: {}", vr::VROverlay()->GetOverlayErrorNameFromEnum(overlayError));
        return false;
    }

    // Configure overlay properties for ASMRtist workflow
    vr::VROverlay()->SetOverlayWidthInMeters(m_overlayHandle, 0.8f);  // 80cm wide UI panel
    vr::VROverlay()->SetOverlayAlpha(m_overlayHandle, 0.9f);          // Slightly transparent
    vr::VROverlay()->SetOverlayColor(m_overlayHandle, 1.0f, 1.0f, 1.0f);  // White base color

    // Position overlay in front of user at comfortable distance
    vr::HmdMatrix34_t overlayTransform = {};
    overlayTransform.m[0][0] = 1.0f;
    overlayTransform.m[1][1] = 1.0f;
    overlayTransform.m[2][2] = 1.0f;
    overlayTransform.m[0][3] = m_position.x;
    overlayTransform.m[1][3] = m_position.y;
    overlayTransform.m[2][3] = m_position.z;

    vr::EVROverlayError transformError = vr::VROverlay()->SetOverlayTransformAbsolute(m_overlayHandle, vr::TrackingUniverseStanding, &overlayTransform);
    if (transformError != vr::VROverlayError_None) {
        LOG_WARN("Failed to set overlay transform: {}", vr::VROverlay()->GetOverlayErrorNameFromEnum(transformError));
        // Continue anyway - transform can be updated later
    }

    // Create thumbnail overlay for dashboard
    vr::EVROverlayError thumbError = vr::VROverlay()->CreateOverlay("vrb.asmr.mic_thumb", "ASMR Mic Thumb", &m_thumbnailHandle);
    if (thumbError != vr::VROverlayError_None) {
        LOG_WARN("Failed to create thumbnail overlay: {}", vr::VROverlay()->GetOverlayErrorNameFromEnum(thumbError));
        // Thumbnail is optional - continue without it
        m_thumbnailHandle = vr::k_ulOverlayHandleInvalid;
    } else {
        // Set thumbnail properties
        vr::VROverlay()->SetOverlayWidthInMeters(m_thumbnailHandle, 0.15f);  // Small thumbnail
    }

    LOG_INFO("✅ Real OpenVR overlay created successfully!");
    LOG_DEBUG("  - Main overlay handle: {}", m_overlayHandle);
    LOG_DEBUG("  - Thumbnail handle: {}", m_thumbnailHandle);
    LOG_DEBUG("  - Position: ({}, {}, {})", m_position.x, m_position.y, m_position.z);

    return true;
}

void AudioRoutingOverlay::DestroyOverlay() {
    LOG_DEBUG("Destroying OpenVR overlay resources");

    // Clean up thumbnail overlay
    if (m_thumbnailHandle != vr::k_ulOverlayHandleInvalid) {
        vr::EVROverlayError thumbError = vr::VROverlay()->DestroyOverlay(m_thumbnailHandle);
        if (thumbError != vr::VROverlayError_None) {
            LOG_WARN("Failed to destroy thumbnail overlay: {}", vr::VROverlay()->GetOverlayErrorNameFromEnum(thumbError));
        } else {
            LOG_DEBUG("  - Destroyed thumbnail overlay");
        }
        m_thumbnailHandle = vr::k_ulOverlayHandleInvalid;
    }

    // Clean up main overlay
    if (m_overlayHandle != vr::k_ulOverlayHandleInvalid) {
        vr::EVROverlayError overlayError = vr::VROverlay()->DestroyOverlay(m_overlayHandle);
        if (overlayError != vr::VROverlayError_None) {
            LOG_WARN("Failed to destroy main overlay: {}", vr::VROverlay()->GetOverlayErrorNameFromEnum(overlayError));
        } else {
            LOG_DEBUG("  - Destroyed main overlay");
        }
        m_overlayHandle = vr::k_ulOverlayHandleInvalid;
    }

    LOG_INFO("✅ OpenVR overlay cleanup complete - no VR resource leaks!");
}

bool AudioRoutingOverlay::CreateRenderTarget() {
    LOG_INFO("Creating real OpenGL render target {}x{}", m_textureWidth, m_textureHeight);

    // Generate framebuffer object
    glGenFramebuffers(1, &m_framebuffer);
    if (m_framebuffer == 0) {
        LOG_ERROR("Failed to generate OpenGL framebuffer");
        return false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

    // Create color texture for rendering
    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_textureWidth, m_textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);

    // Create depth texture for proper 3D rendering
    glGenTextures(1, &m_depthTexture);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_textureWidth, m_textureHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);

    // Check framebuffer completeness
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("OpenGL framebuffer incomplete! Status: 0x{:X}", status);
        DestroyRenderTarget();  // Clean up partial resources
        return false;
    }

    // Unbind framebuffer to restore default
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    LOG_INFO("✅ Real OpenGL render target created successfully!");
    LOG_DEBUG("  - Framebuffer ID: {}", m_framebuffer);
    LOG_DEBUG("  - Color texture ID: {}", m_colorTexture);
    LOG_DEBUG("  - Depth texture ID: {}", m_depthTexture);

    return true;
}

void AudioRoutingOverlay::DestroyRenderTarget() {
    LOG_DEBUG("Destroying OpenGL render target resources");

    // Clean up depth texture
    if (m_depthTexture != 0) {
        glDeleteTextures(1, &m_depthTexture);
        m_depthTexture = 0;
        LOG_DEBUG("  - Deleted depth texture");
    }

    // Clean up color texture
    if (m_colorTexture != 0) {
        glDeleteTextures(1, &m_colorTexture);
        m_colorTexture = 0;
        LOG_DEBUG("  - Deleted color texture");
    }

    // Clean up framebuffer
    if (m_framebuffer != 0) {
        glDeleteFramebuffers(1, &m_framebuffer);
        m_framebuffer = 0;
        LOG_DEBUG("  - Deleted framebuffer");
    }

    LOG_INFO("✅ OpenGL render target cleanup complete - no resource leaks!");
}

void AudioRoutingOverlay::UpdateOverlayTexture() {
    // Check if overlay exists
    if (m_overlayHandle == vr::k_ulOverlayHandleInvalid) {
        LOG_WARN("Cannot update texture - overlay not created");
        return;
    }

    // Check if we have a valid color texture from rendering
    if (m_colorTexture == 0) {
        LOG_WARN("Cannot update texture - no color texture available");
        return;
    }

    // Create OpenVR texture structure pointing to our OpenGL texture
    vr::Texture_t eyeTexture = {};
    eyeTexture.handle = reinterpret_cast<void*>(static_cast<uintptr_t>(m_colorTexture));
    eyeTexture.eType = vr::TextureType_OpenGL;
    eyeTexture.eColorSpace = vr::ColorSpace_Gamma;

    // Set texture on the overlay
    vr::EVROverlayError error = vr::VROverlay()->SetOverlayTexture(m_overlayHandle, &eyeTexture);
    if (error != vr::VROverlayError_None) {
        LOG_ERROR("Failed to set overlay texture: {}", vr::VROverlay()->GetOverlayErrorNameFromEnum(error));
        return;
    }

    // Update thumbnail if it exists
    if (m_thumbnailHandle != vr::k_ulOverlayHandleInvalid) {
        vr::EVROverlayError thumbError = vr::VROverlay()->SetOverlayTexture(m_thumbnailHandle, &eyeTexture);
        if (thumbError != vr::VROverlayError_None) {
            LOG_DEBUG("Failed to set thumbnail texture: {}", vr::VROverlay()->GetOverlayErrorNameFromEnum(thumbError));
            // Thumbnail failure is not critical
        }
    }

    LOG_DEBUG("✅ OpenVR overlay texture updated with GL texture ID {}", m_colorTexture);
}

void AudioRoutingOverlay::UpdateOverlayTransform() {
    // Check if overlay exists
    if (m_overlayHandle == vr::k_ulOverlayHandleInvalid) {
        LOG_WARN("Cannot update transform - overlay not created");
        return;
    }

    // Build transform matrix from current position and scale
    vr::HmdMatrix34_t transform = {};

    // Set rotation matrix (identity for now - overlay faces forward)
    transform.m[0][0] = m_scale;  // X scale
    transform.m[1][1] = m_scale;  // Y scale
    transform.m[2][2] = m_scale;  // Z scale

    // Set translation (position in VR space)
    transform.m[0][3] = m_position.x;
    transform.m[1][3] = m_position.y;
    transform.m[2][3] = m_position.z;

    // Apply transform to overlay in tracking universe
    vr::EVROverlayError error = vr::VROverlay()->SetOverlayTransformAbsolute(
        m_overlayHandle,
        vr::TrackingUniverseStanding,
        &transform
    );

    if (error != vr::VROverlayError_None) {
        LOG_ERROR("Failed to update overlay transform: {}", vr::VROverlay()->GetOverlayErrorNameFromEnum(error));
        return;
    }

    // Update overlay visibility based on current state
    bool shouldBeVisible = m_visible.load() && (m_uiState.showControls || m_uiState.showMicrophone || m_isRecording.load());

    vr::EVROverlayError visError = vr::VROverlay()->SetOverlayFlag(
        m_overlayHandle,
        vr::VROverlayFlags_VisibleInDashboard,
        shouldBeVisible
    );

    if (visError != vr::VROverlayError_None) {
        LOG_DEBUG("Failed to set overlay visibility: {}", vr::VROverlay()->GetOverlayErrorNameFromEnum(visError));
        // Non-critical error, continue
    }

    LOG_DEBUG("✅ Overlay transform updated - Position: ({:.2f}, {:.2f}, {:.2f}), Scale: {:.2f}, Visible: {}",
              m_position.x, m_position.y, m_position.z, m_scale, shouldBeVisible);
}

void AudioRoutingOverlay::RenderDebugInfo() {
    if (!m_debugMode) return;

    LOG_DEBUG("Mic Position: ({}, {}, {})", m_virtualMic.position.x, m_virtualMic.position.y, m_virtualMic.position.z);
    LOG_DEBUG("Recording: {}, Monitoring: {}", m_isRecording.load(), m_uiState.monitoring);
    LOG_DEBUG("Input Level: {}, Output Level: {}", m_inputLevel.load(), m_outputLevel.load());
}

// Gesture detection compatibility methods for AudioCockpit integration
void AudioRoutingOverlay::UpdateGestureDetection(const std::vector<VRPose>& controllers) {
    // Simple stub implementation - delegate to controller tracking
    UpdateControllerTracking(controllers);
    // TODO: Add actual gesture detection if needed for advanced features
}

void AudioRoutingOverlay::UpdateAudioOrbPositions(const VRPose& hmdPose) {
    // Simple stub implementation - delegate to microphone tracking
    UpdateMicrophoneTracking(hmdPose);
    // TODO: Add orb visualization if needed for advanced features
}

void AudioRoutingOverlay::RegisterGestureCallback(std::function<void(const std::string&, const Vec3&)> callback) {
    m_gestureCallback = callback;
}

// === CREATIVE SOLUTION: VR RECORDING CONTROLS ===

bool AudioRoutingOverlay::IsControllerNearPosition(const Vec3& targetPos, float tolerance) {
    // Check if either controller is near the target position
    bool leftNear = CalculateDistance(m_leftController.position, targetPos) <= tolerance;
    bool rightNear = CalculateDistance(m_rightController.position, targetPos) <= tolerance;

    return leftNear || rightNear;
}

void AudioRoutingOverlay::ToggleRecording() {
    // Creative toggle between recording states
    if (m_isRecording.load()) {
        StopRecording();
        LOG_INFO("✅ ASMRtist STOPPED recording via VR control - creative content creation complete!");
    } else {
        StartRecording();
        LOG_INFO("🎤 ASMRtist STARTED recording via VR control - spatial magic begins!");
    }

    // Update UI interaction timestamp to keep overlay visible during recording changes
    m_uiState.lastInteraction = std::chrono::steady_clock::now();

    // Optional: Trigger haptic feedback on controllers if VR system supports it
    if (vr::VRSystem() && m_leftControllerIndex != k_unTrackedDeviceIndexInvalid) {
        // Brief haptic pulse to confirm action - creative tactile feedback!
        vr::VRSystem()->TriggerHapticPulse(m_leftControllerIndex, 0, 1000);  // 1ms pulse
    }
    if (vr::VRSystem() && m_rightControllerIndex != k_unTrackedDeviceIndexInvalid) {
        vr::VRSystem()->TriggerHapticPulse(m_rightControllerIndex, 0, 1000);  // 1ms pulse
    }
}

} // namespace vrb

// === ASMR UTILITY FUNCTIONS ===

namespace vrb::asmr_utils {

Vec3 CalculateOptimalMicPosition(const Vec3& userPos, float distance) {
    // Simple optimal positioning - in front of user at head height
    return Vec3{userPos.x, userPos.y, userPos.z - distance};
}

SpatialPreview CalculatePreview(const Vec3& micPos, const Vec3& userPos) {
    // Calculate simple left/right volume based on microphone position relative to user
    float dx = micPos.x - userPos.x;
    float distance = std::sqrt(dx*dx + (micPos.y - userPos.y)*(micPos.y - userPos.y) + (micPos.z - userPos.z)*(micPos.z - userPos.z));

    // Simple panning based on left/right offset
    float pan = std::max(-1.0f, std::min(1.0f, dx / 2.0f));  // Normalize to [-1, 1]

    SpatialPreview preview;
    preview.distance = distance;
    preview.leftVolume = (1.0f - pan) * 0.5f;   // More volume on left when mic is on left
    preview.rightVolume = (1.0f + pan) * 0.5f;  // More volume on right when mic is on right

    // Apply distance attenuation
    float attenuation = 1.0f / (1.0f + distance * distance);
    preview.leftVolume *= attenuation;
    preview.rightVolume *= attenuation;

    return preview;
}

} // namespace vrb::asmr_utils