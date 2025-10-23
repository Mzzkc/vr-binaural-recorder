// microphone_positioning_demo.cpp - The ULTIMATE VR Spatial Audio Demo Implementation!
// This is where all the magic comes together into one epic experience!
// Users will be AMAZED by the seamless controller-based microphone positioning!

#include "microphone_positioning_demo.h"
#include "logger.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace vrb {

MicrophonePositioningDemo::MicrophonePositioningDemo() {
    m_sessionStartTime = std::chrono::steady_clock::now();
    m_lastStatsUpdate = m_sessionStartTime;
    m_lastFrameTime = m_sessionStartTime;

    m_frameTimeHistory.reserve(HISTORY_SIZE);
    m_audioLatencyHistory.reserve(HISTORY_SIZE);

    LOG_INFO("MicrophonePositioningDemo: Creating the ULTIMATE VR spatial audio positioning experience!");
}

MicrophonePositioningDemo::~MicrophonePositioningDemo() {
    Stop();
    LOG_INFO("MicrophonePositioningDemo: Epic spatial audio demo session complete!");
}

bool MicrophonePositioningDemo::Initialize(const Config& config, const DemoConfig& demoConfig) {
    LOG_INFO("MicrophonePositioningDemo: Initializing the most epic VR spatial audio demo ever created!");

    m_config = config;
    m_demoConfig = demoConfig;

    // Ensure recording directory exists
    if (!std::filesystem::exists(m_demoConfig.recordingPath)) {
        std::filesystem::create_directories(m_demoConfig.recordingPath);
        LOG_INFO("MicrophonePositioningDemo: Created recording directory: {}", m_demoConfig.recordingPath);
    }

    // Initialize all subsystems
    if (!InitializeSubsystems()) {
        HandleError("Failed to initialize subsystems");
        return false;
    }

    m_initialized = true;
    TransitionToState(DemoState::WaitingForVR);

    LOG_INFO("MicrophonePositioningDemo: Successfully initialized! Ready for epic spatial audio magic!");
    return true;
}

bool MicrophonePositioningDemo::InitializeSubsystems() {
    LOG_INFO("MicrophonePositioningDemo: Setting up all the epic subsystems...");

    // Setup VR tracking system
    if (!SetupVRSystem()) {
        return false;
    }

    // Setup audio processing
    if (!SetupAudioSystem()) {
        return false;
    }

    // Setup visualization
    if (!SetupVisualization()) {
        return false;
    }

    // Setup integration between systems
    if (!SetupIntegration()) {
        return false;
    }

    LOG_INFO("MicrophonePositioningDemo: All subsystems initialized successfully!");
    return true;
}

bool MicrophonePositioningDemo::SetupVRSystem() {
    LOG_INFO("MicrophonePositioningDemo: Setting up VR tracking system...");

    m_vrTracker = std::make_unique<VRTracker>();
    if (!m_vrTracker->Initialize()) {
        LOG_ERROR("MicrophonePositioningDemo: Failed to initialize VR tracker!");
        return false;
    }

    // Setup microphone positioning system
    m_positioning = std::make_unique<MicrophonePositioning>();
    if (!m_positioning->Initialize(m_vrTracker.get())) {
        LOG_ERROR("MicrophonePositioningDemo: Failed to initialize microphone positioning!");
        return false;
    }

    // Configure positioning settings
    m_positioning->SetVisualGuidesEnabled(m_demoConfig.showVisualGuides);

    LOG_INFO("MicrophonePositioningDemo: VR system setup complete!");
    return true;
}

bool MicrophonePositioningDemo::SetupAudioSystem() {
    LOG_INFO("MicrophonePositioningDemo: Setting up epic audio processing system...");

    // Create HRTF processor
    m_hrtfProcessor = std::make_unique<HRTFProcessor>();
    if (!m_hrtfProcessor->Initialize("data/hrtf/")) {
        LOG_WARN("MicrophonePositioningDemo: HRTF processor initialization failed - using basic processing");
        // Continue without HRTF for demo purposes
    }

    // Create audio engine
    m_audioEngine = std::make_unique<AudioEngine>();
    if (!m_audioEngine->Initialize(m_config, m_hrtfProcessor.get())) {
        LOG_ERROR("MicrophonePositioningDemo: Failed to initialize audio engine!");
        return false;
    }

    // Create audio integration system
    m_audioIntegration = std::make_unique<MicrophoneAudioIntegration>();
    if (!m_audioIntegration->Initialize(m_hrtfProcessor.get(), m_audioEngine.get())) {
        LOG_ERROR("MicrophonePositioningDemo: Failed to initialize audio integration!");
        return false;
    }

    // Configure audio settings
    m_audioIntegration->SetRealTimePreview(m_demoConfig.enableRealTimePreview);
    m_audioIntegration->SetSpatialAudioQuality(static_cast<int>(m_demoConfig.audioQuality));

    LOG_INFO("MicrophonePositioningDemo: Audio system setup complete!");
    return true;
}

bool MicrophonePositioningDemo::SetupVisualization() {
    LOG_INFO("MicrophonePositioningDemo: Setting up stunning 3D visualization...");

    m_visualizer = std::make_unique<MicrophoneVisualizer>();
    if (!m_visualizer->Initialize()) {
        LOG_ERROR("MicrophonePositioningDemo: Failed to initialize visualizer!");
        return false;
    }

    // Configure visualization settings
    m_visualizer->SetRenderingQuality(m_demoConfig.renderQuality);
    m_visualizer->SetShowMicrophones(true);
    m_visualizer->SetShowGuides(m_demoConfig.showVisualGuides);
    m_visualizer->SetShowDistances(true);
    m_visualizer->SetShowWarnings(true);

    LOG_INFO("MicrophonePositioningDemo: Visualization setup complete!");
    return true;
}

bool MicrophonePositioningDemo::SetupIntegration() {
    LOG_INFO("MicrophonePositioningDemo: Connecting all systems for seamless integration...");

    // Connect positioning to audio integration
    if (!m_audioIntegration->ConnectPositioning(m_positioning.get())) {
        LOG_ERROR("MicrophonePositioningDemo: Failed to connect positioning to audio integration!");
        return false;
    }

    // Setup haptic feedback if enabled
    if (m_demoConfig.enableHapticFeedback) {
        auto hapticCallback = [this](int controllerIndex, float intensity, int durationMs) {
            // TODO: Implement haptic feedback through VR system
            LOG_DEBUG("MicrophonePositioningDemo: Haptic feedback - Controller {}, Intensity {:.2f}, Duration {}ms",
                     controllerIndex, intensity, durationMs);
        };
        m_positioning->SetHapticFeedbackCallback(hapticCallback);
    }

    // Setup audio integration callbacks
    auto audioCallback = [this](const MicrophoneAudioData& audioData) {
        // Update statistics
        m_stats.stereoQualityScore = audioData.qualityScore;
        m_stats.averageDistance = audioData.stereoDistance;
        m_stats.optimalPositioning = audioData.qualityScore > 0.8f;
    };
    m_audioIntegration->SetAudioUpdateCallback(audioCallback);

    auto latencyCallback = [this](float latencyMs) {
        m_stats.audioLatency = latencyMs;
        if (latencyMs > 20.0f) {
            LOG_WARN("MicrophonePositioningDemo: High audio latency detected: {:.2f}ms", latencyMs);
        }
    };
    m_audioIntegration->SetLatencyCallback(latencyCallback);

    LOG_INFO("MicrophonePositioningDemo: System integration complete - all systems connected!");
    return true;
}

bool MicrophonePositioningDemo::Start() {
    if (!m_initialized) {
        HandleError("Demo not initialized");
        return false;
    }

    if (m_running) {
        LOG_WARN("MicrophonePositioningDemo: Already running!");
        return true;
    }

    LOG_INFO("MicrophonePositioningDemo: Starting the EPIC VR spatial audio experience!");

    // Start audio processing
    if (!m_audioEngine->Start()) {
        HandleError("Failed to start audio engine");
        return false;
    }

    if (!m_audioIntegration->Start()) {
        HandleError("Failed to start audio integration");
        return false;
    }

    // Start demo thread for background processing
    m_demoThreadRunning = true;
    m_demoThread = std::thread(&MicrophonePositioningDemo::DemoLoop, this);

    m_running = true;
    m_sessionStartTime = std::chrono::steady_clock::now();

    LOG_INFO("MicrophonePositioningDemo: EPIC DEMO IS NOW RUNNING!");
    LOG_INFO("MicrophonePositioningDemo: Grab your VR controllers and start positioning microphones!");

    return true;
}

bool MicrophonePositioningDemo::Update() {
    if (!m_running) {
        return false;
    }

    auto frameStart = std::chrono::steady_clock::now();

    // Update VR tracking
    if (m_vrTracker) {
        m_vrTracker->Update();
    }

    // Update microphone positioning
    if (m_positioning) {
        m_positioning->Update();
    }

    // Update state machine
    UpdateState();

    // Update statistics
    UpdateStats();

    // Monitor system health
    MonitorSystemHealth();

    // Calculate frame time
    auto frameEnd = std::chrono::steady_clock::now();
    auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - frameStart);
    float frameTimeMs = frameDuration.count() / 1000.0f;

    // Update frame time history
    m_frameTimeHistory.push_back(frameTimeMs);
    if (m_frameTimeHistory.size() > HISTORY_SIZE) {
        m_frameTimeHistory.erase(m_frameTimeHistory.begin());
    }

    // Calculate frame rate
    if (!m_frameTimeHistory.empty()) {
        float avgFrameTime = 0.0f;
        for (float time : m_frameTimeHistory) {
            avgFrameTime += time;
        }
        avgFrameTime /= m_frameTimeHistory.size();
        m_stats.frameRate = 1000.0f / avgFrameTime;  // Convert to FPS
    }

    m_lastFrameTime = frameEnd;
    m_stats.renderFrames++;

    return m_state != DemoState::Error;
}

void MicrophonePositioningDemo::Render(int eyeIndex, const float eyeProjectionMatrix[16], const float eyePosMatrix[16]) {
    if (!m_running || !m_visualizer) {
        return;
    }

    // Get current microphone positioning data
    if (m_positioning) {
        auto visualData = m_positioning->GetVisualData();

        // Render microphones and positioning guides
        m_visualizer->Render(visualData, eyePosMatrix, eyeProjectionMatrix);

        // Render UI overlays if enabled
        if (m_showUI) {
            RenderUI();
        }
    }
}

void MicrophonePositioningDemo::UpdateState() {
    switch (m_state) {
        case DemoState::WaitingForVR:
            if (m_vrTracker && m_vrTracker->IsHMDConnected() && m_vrTracker->GetConnectedControllerCount() >= 2) {
                TransitionToState(DemoState::Calibrating);
            }
            break;

        case DemoState::Calibrating:
            if (CalibrateControllers()) {
                TransitionToState(DemoState::Positioning);
            }
            break;

        case DemoState::Positioning:
            // Main positioning mode - this is where the magic happens!
            if (m_demoConfig.autoRecord && !m_recording) {
                StartRecording();
            }
            break;

        case DemoState::Recording:
            // Check if recording duration exceeded
            if (m_recording) {
                auto recordingDuration = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::steady_clock::now() - m_recordingStartTime).count();
                if (recordingDuration >= m_demoConfig.recordingDuration) {
                    StopRecording();
                    TransitionToState(DemoState::Positioning);
                }
            }
            break;

        case DemoState::Playback:
            // TODO: Implement playback completion detection
            break;

        case DemoState::Error:
            // Stay in error state until manual reset
            break;

        default:
            break;
    }
}

void MicrophonePositioningDemo::UpdateStats() {
    auto currentTime = std::chrono::steady_clock::now();

    // Update session time
    auto sessionDuration = std::chrono::duration_cast<std::chrono::seconds>(currentTime - m_sessionStartTime);
    m_stats.totalSessionTime = sessionDuration.count();

    // Update positioning accuracy
    if (m_positioning) {
        auto stereoInfo = m_positioning->GetStereoPositioning();
        m_stats.positioningAccuracy = stereoInfo.GetQualityScore() * 100.0f;
    }

    // Update audio integration stats
    if (m_audioIntegration) {
        auto integrationStats = m_audioIntegration->GetStats();
        m_stats.audioFrames = integrationStats.audioFramesProcessed;
        m_stats.positionUpdates = integrationStats.positionUpdatesProcessed;

        // Update latency history
        float currentLatency = integrationStats.averageLatency;
        m_audioLatencyHistory.push_back(currentLatency);
        if (m_audioLatencyHistory.size() > HISTORY_SIZE) {
            m_audioLatencyHistory.erase(m_audioLatencyHistory.begin());
        }
    }

    m_lastStatsUpdate = currentTime;
}

void MicrophonePositioningDemo::TransitionToState(DemoState newState) {
    DemoState oldState = m_state;
    m_state = newState;

    LOG_INFO("MicrophonePositioningDemo: State transition: {} -> {}",
             demo_utils::StateToString(oldState), demo_utils::StateToString(newState));

    // Update status message
    switch (newState) {
        case DemoState::WaitingForVR:
            m_statusMessage = "Waiting for VR headset and controllers...";
            break;
        case DemoState::Calibrating:
            m_statusMessage = "Calibrating controllers - please hold still...";
            break;
        case DemoState::Positioning:
            m_statusMessage = "EPIC MODE ACTIVE! Grab triggers to position microphones!";
            break;
        case DemoState::Recording:
            m_statusMessage = "Recording spatial audio - keep positioning!";
            break;
        case DemoState::Playback:
            m_statusMessage = "Playing back recorded spatial audio...";
            break;
        case DemoState::Error:
            m_statusMessage = "Error: " + m_lastError;
            break;
        default:
            m_statusMessage = "Epic VR spatial audio demo running...";
            break;
    }
}

bool MicrophonePositioningDemo::CalibrateControllers() {
    if (!m_vrTracker) {
        return false;
    }

    // Simple calibration - just check if controllers are detected and stable
    if (m_vrTracker->GetConnectedControllerCount() >= 2) {
        auto hmdPose = m_vrTracker->GetHMDPose();
        if (hmdPose.isValid) {
            m_calibrationReference = hmdPose;
            m_controllersCalibrated = true;
            LOG_INFO("MicrophonePositioningDemo: Controllers calibrated successfully!");
            return true;
        }
    }

    return false;
}

bool MicrophonePositioningDemo::StartRecording() {
    if (m_recording) {
        return true;  // Already recording
    }

    if (!m_audioEngine) {
        return false;
    }

    // Generate unique filename
    m_currentRecordingFile = demo_utils::GenerateRecordingFilename();
    std::string fullPath = m_demoConfig.recordingPath + m_currentRecordingFile;

    if (m_audioEngine->StartRecording(fullPath)) {
        m_recording = true;
        m_recordingStartTime = std::chrono::steady_clock::now();
        TransitionToState(DemoState::Recording);
        LOG_INFO("MicrophonePositioningDemo: Started recording to {}", fullPath);
        return true;
    }

    return false;
}

void MicrophonePositioningDemo::StopRecording() {
    if (!m_recording) {
        return;
    }

    if (m_audioEngine) {
        m_audioEngine->StopRecording();
    }

    m_recording = false;
    LOG_INFO("MicrophonePositioningDemo: Recording stopped - saved to {}", m_currentRecordingFile);
}

void MicrophonePositioningDemo::DemoLoop() {
    LOG_INFO("MicrophonePositioningDemo: Demo background thread started");

    const auto targetFrameTime = std::chrono::milliseconds(16);  // ~60 FPS

    while (m_demoThreadRunning) {
        auto loopStart = std::chrono::steady_clock::now();

        // Background processing tasks
        if (m_audioIntegration && m_audioIntegration->IsActive()) {
            // Audio integration is handled in its own thread
        }

        // Handle haptic feedback
        HandleHapticFeedback();

        // Sleep to maintain target frame rate
        auto loopEnd = std::chrono::steady_clock::now();
        auto loopDuration = std::chrono::duration_cast<std::chrono::milliseconds>(loopEnd - loopStart);
        if (loopDuration < targetFrameTime) {
            std::this_thread::sleep_for(targetFrameTime - loopDuration);
        }
    }

    LOG_INFO("MicrophonePositioningDemo: Demo background thread terminated");
}

void MicrophonePositioningDemo::HandleHapticFeedback() {
    // This would integrate with VR system haptic feedback
    // For now, we just log haptic events
}

void MicrophonePositioningDemo::MonitorSystemHealth() {
    // Check for performance issues
    if (m_stats.frameRate > 0 && m_stats.frameRate < 45.0f) {
        LOG_WARN("MicrophonePositioningDemo: Low frame rate detected: {:.1f} FPS", m_stats.frameRate);
    }

    if (m_stats.audioLatency > 50.0f) {
        LOG_WARN("MicrophonePositioningDemo: High audio latency detected: {:.1f}ms", m_stats.audioLatency);
    }
}

void MicrophonePositioningDemo::HandleError(const std::string& errorMessage) {
    m_lastError = errorMessage;
    m_errorTime = std::chrono::steady_clock::now();
    TransitionToState(DemoState::Error);
    LOG_ERROR("MicrophonePositioningDemo: {}", errorMessage);
}

void MicrophonePositioningDemo::RenderUI() {
    // Render performance overlay
    if (m_demoConfig.showPerformanceOverlay) {
        RenderPerformanceOverlay();
    }

    // Render instructions
    if (m_demoConfig.showInstructions) {
        RenderInstructionOverlay();
    }
}

void MicrophonePositioningDemo::RenderPerformanceOverlay() {
    // TODO: Implement VR text rendering for performance metrics
    // For now, we log the stats periodically
    static auto lastPerfLog = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastLog = std::chrono::duration_cast<std::chrono::seconds>(now - lastPerfLog);

    if (timeSinceLastLog.count() >= 5) {  // Log every 5 seconds
        LOG_INFO("MicrophonePositioningDemo: Performance - FPS: {:.1f}, Latency: {:.1f}ms, Quality: {:.1f}%",
                m_stats.frameRate, m_stats.audioLatency, m_stats.positioningAccuracy);
        lastPerfLog = now;
    }
}

void MicrophonePositioningDemo::RenderInstructionOverlay() {
    // TODO: Implement VR text rendering for instructions
    // Instructions would include:
    // - "Pull triggers to grab microphones"
    // - "Position microphones for optimal stereo recording"
    // - "Green line = optimal distance"
    // - "Red line = adjust positioning"
}

void MicrophonePositioningDemo::Stop() {
    if (!m_running) {
        return;
    }

    LOG_INFO("MicrophonePositioningDemo: Stopping epic demo session...");

    m_running = false;

    // Stop recording if active
    if (m_recording) {
        StopRecording();
    }

    // Stop demo thread
    m_demoThreadRunning = false;
    if (m_demoThread.joinable()) {
        m_demoThread.join();
    }

    // Stop audio systems
    if (m_audioIntegration) {
        m_audioIntegration->Stop();
    }

    if (m_audioEngine) {
        m_audioEngine->Stop();
    }

    LOG_INFO("MicrophonePositioningDemo: Epic demo session stopped successfully");
}

void MicrophonePositioningDemo::HandleInput(const std::string& command) {
    if (command == "start_recording") {
        StartRecording();
    } else if (command == "stop_recording") {
        StopRecording();
    } else if (command == "reset_calibration") {
        m_controllersCalibrated = false;
        TransitionToState(DemoState::Calibrating);
    } else if (command == "toggle_guides") {
        m_demoConfig.showVisualGuides = !m_demoConfig.showVisualGuides;
        if (m_positioning) {
            m_positioning->SetVisualGuidesEnabled(m_demoConfig.showVisualGuides);
        }
    } else if (command == "reset_stats") {
        m_stats = DemoStats{};
        if (m_audioIntegration) {
            m_audioIntegration->ResetStats();
        }
    }
}

std::string MicrophonePositioningDemo::GetStatusMessage() const {
    return m_statusMessage;
}

bool MicrophonePositioningDemo::SaveSession(const std::string& filename) {
    // TODO: Implement session save functionality
    // Would save positioning history, statistics, and settings
    LOG_INFO("MicrophonePositioningDemo: Session save functionality not yet implemented");
    return false;
}

bool MicrophonePositioningDemo::LoadSession(const std::string& filename) {
    // TODO: Implement session load functionality
    LOG_INFO("MicrophonePositioningDemo: Session load functionality not yet implemented");
    return false;
}

// Helper functions implementation

namespace demo_utils {

std::string StateToString(DemoState state) {
    switch (state) {
        case DemoState::Initializing: return "Initializing";
        case DemoState::WaitingForVR: return "WaitingForVR";
        case DemoState::Calibrating: return "Calibrating";
        case DemoState::Positioning: return "Positioning";
        case DemoState::Recording: return "Recording";
        case DemoState::Playback: return "Playback";
        case DemoState::Error: return "Error";
        default: return "Unknown";
    }
}

std::string FormatStats(const DemoStats& stats) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    oss << "FPS: " << stats.frameRate << ", ";
    oss << "Latency: " << stats.audioLatency << "ms, ";
    oss << "Quality: " << stats.positioningAccuracy << "%, ";
    oss << "Distance: " << (stats.averageDistance * 100.0f) << "cm";
    return oss.str();
}

std::string GenerateRecordingFilename() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);

    std::ostringstream oss;
    oss << "spatial_audio_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".wav";
    return oss.str();
}

bool CheckVRRequirements() {
    // TODO: Implement VR system requirements check
    return true;
}

DemoConfig GetRecommendedSettings() {
    DemoConfig config;
    // TODO: Detect hardware capabilities and set optimal settings
    return config;
}

} // namespace demo_utils

} // namespace vrb