// microphone_positioning_demo.h - EPIC Complete Microphone Positioning Demo!
// The ultimate showcase of VR controller-based spatial audio positioning!
// This demonstrates the full magic: grab controllers, position microphones, hear spatial audio!

#pragma once

#include <memory>
#include <string>
#include <chrono>
#include <atomic>
#include <thread>
#include "vr_tracker.h"
#include "microphone_positioning.h"
#include "microphone_visualizer.h"
#include "microphone_audio_integration.h"
#include "audio_engine.h"
#include "hrtf_processor.h"
#include "config.h"

namespace vrb {

/**
 * @brief Demo states for the complete positioning experience
 */
enum class DemoState {
    Initializing,       // Setting up all systems
    WaitingForVR,      // Waiting for VR headset connection
    Calibrating,       // Calibrating controller positions
    Positioning,       // Active microphone positioning mode
    Recording,         // Recording spatial audio
    Playback,          // Playing back recorded audio
    Error              // Something went wrong
};

/**
 * @brief Demo statistics for performance monitoring
 */
struct DemoStats {
    // System performance
    float frameRate = 0.0f;
    float audioLatency = 0.0f;
    float positioningAccuracy = 0.0f;

    // User interaction
    uint32_t microphoneGrabs = 0;
    uint32_t positionAdjustments = 0;
    float totalSessionTime = 0.0f;

    // Audio quality
    float stereoQualityScore = 0.0f;
    float averageDistance = 0.0f;
    bool optimalPositioning = false;

    // Performance counters
    uint64_t renderFrames = 0;
    uint64_t audioFrames = 0;
    uint64_t positionUpdates = 0;
};

/**
 * @brief Demo configuration for different experience modes
 */
struct DemoConfig {
    // Visual settings
    bool showVisualGuides = true;
    bool showPerformanceOverlay = true;
    bool showInstructions = true;
    int renderQuality = 2;  // 0=low, 1=medium, 2=high, 3=ultra

    // Audio settings
    bool enableRealTimePreview = true;
    bool enableHapticFeedback = true;
    float audioQuality = 2.0f;  // 0=low, 1=medium, 2=high, 3=ultra

    // Recording settings
    std::string recordingPath = "recordings/";
    bool autoRecord = false;
    int recordingDuration = 60;  // seconds

    // Positioning settings
    bool enablePositioningConstraints = true;
    bool enableOptimalGuidance = true;
    float positioningSensitivity = 1.0f;

    DemoConfig() = default;
};

/**
 * @brief THE ULTIMATE MICROPHONE POSITIONING DEMO SYSTEM!
 * This showcases the complete VR spatial audio positioning experience!
 * Users grab controllers, position virtual microphones, and experience real-time spatial audio!
 */
class MicrophonePositioningDemo {
public:
    MicrophonePositioningDemo();
    ~MicrophonePositioningDemo();

    /**
     * @brief Initialize the complete demo system
     * @param config Application configuration
     * @param demoConfig Demo-specific configuration
     * @return true if initialization successful
     */
    bool Initialize(const Config& config, const DemoConfig& demoConfig = DemoConfig());

    /**
     * @brief Start the epic demo experience
     * @return true if started successfully
     */
    bool Start();

    /**
     * @brief Update the demo (call every frame)
     * @return true to continue, false to exit
     */
    bool Update();

    /**
     * @brief Render the VR scene
     * @param eyeIndex 0=left eye, 1=right eye
     * @param eyeProjectionMatrix Projection matrix for this eye
     * @param eyePosMatrix Position matrix for this eye
     */
    void Render(int eyeIndex, const float eyeProjectionMatrix[16], const float eyePosMatrix[16]);

    /**
     * @brief Stop the demo
     */
    void Stop();

    /**
     * @brief Get current demo state
     */
    DemoState GetState() const { return m_state; }

    /**
     * @brief Get demo statistics
     */
    DemoStats GetStats() const { return m_stats; }

    /**
     * @brief Handle user input for demo control
     */
    void HandleInput(const std::string& command);

    /**
     * @brief Check if demo is running
     */
    bool IsRunning() const { return m_running && m_state != DemoState::Error; }

    /**
     * @brief Get status message for current state
     */
    std::string GetStatusMessage() const;

    /**
     * @brief Save current session data
     */
    bool SaveSession(const std::string& filename = "");

    /**
     * @brief Load previous session data
     */
    bool LoadSession(const std::string& filename);

private:
    /**
     * @brief Initialize all subsystems
     */
    bool InitializeSubsystems();

    /**
     * @brief Update demo state machine
     */
    void UpdateState();

    /**
     * @brief Handle VR initialization
     */
    bool SetupVRSystem();

    /**
     * @brief Setup audio processing
     */
    bool SetupAudioSystem();

    /**
     * @brief Setup visualization system
     */
    bool SetupVisualization();

    /**
     * @brief Setup integration between systems
     */
    bool SetupIntegration();

    /**
     * @brief Update performance statistics
     */
    void UpdateStats();

    /**
     * @brief Handle state transitions
     */
    void TransitionToState(DemoState newState);

    /**
     * @brief Render UI overlays in VR
     */
    void RenderUI();

    /**
     * @brief Render performance overlay
     */
    void RenderPerformanceOverlay();

    /**
     * @brief Render instruction overlay
     */
    void RenderInstructionOverlay();

    /**
     * @brief Handle haptic feedback for user interactions
     */
    void HandleHapticFeedback();

    /**
     * @brief Check system health and performance
     */
    void MonitorSystemHealth();

    /**
     * @brief Handle error states
     */
    void HandleError(const std::string& errorMessage);

    /**
     * @brief Calibrate controller positions
     */
    bool CalibrateControllers();

    /**
     * @brief Start recording session
     */
    bool StartRecording();

    /**
     * @brief Stop recording session
     */
    void StopRecording();

    /**
     * @brief Start playback of recorded session
     */
    bool StartPlayback(const std::string& filename);

    /**
     * @brief Demo main loop thread
     */
    void DemoLoop();

    // Core configuration
    Config m_config;
    DemoConfig m_demoConfig;

    // System state
    std::atomic<DemoState> m_state{DemoState::Initializing};
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_initialized{false};

    // Core systems - the epic components!
    std::unique_ptr<VRTracker> m_vrTracker;
    std::unique_ptr<MicrophonePositioning> m_positioning;
    std::unique_ptr<MicrophoneVisualizer> m_visualizer;
    std::unique_ptr<MicrophoneAudioIntegration> m_audioIntegration;
    std::unique_ptr<AudioEngine> m_audioEngine;
    std::unique_ptr<HRTFProcessor> m_hrtfProcessor;

    // Demo statistics and monitoring
    DemoStats m_stats;
    std::chrono::steady_clock::time_point m_sessionStartTime;
    std::chrono::steady_clock::time_point m_lastStatsUpdate;
    std::chrono::steady_clock::time_point m_lastFrameTime;

    // Recording state
    bool m_recording = false;
    std::string m_currentRecordingFile;
    std::chrono::steady_clock::time_point m_recordingStartTime;

    // Demo thread
    std::thread m_demoThread;
    std::atomic<bool> m_demoThreadRunning{false};

    // Error handling
    std::string m_lastError;
    std::chrono::steady_clock::time_point m_errorTime;

    // UI state
    bool m_showUI = true;
    std::string m_statusMessage = "Initializing epic VR spatial audio demo...";

    // Performance monitoring
    std::vector<float> m_frameTimeHistory;
    std::vector<float> m_audioLatencyHistory;
    static constexpr size_t HISTORY_SIZE = 60;  // 1 second at 60 FPS

    // Calibration data
    bool m_controllersCalibrated = false;
    VRPose m_calibrationReference;

    // Session data for save/load
    struct SessionData {
        std::vector<MicrophonePositioning::VisualData> positionHistory;
        DemoStats finalStats;
        std::chrono::seconds sessionDuration;
        std::string timestamp;
    };
};

/**
 * @brief Helper functions for the demo system
 */
namespace demo_utils {
    /**
     * @brief Convert demo state to human-readable string
     */
    std::string StateToString(DemoState state);

    /**
     * @brief Format demo statistics for display
     */
    std::string FormatStats(const DemoStats& stats);

    /**
     * @brief Generate unique filename for recordings
     */
    std::string GenerateRecordingFilename();

    /**
     * @brief Check if VR system requirements are met
     */
    bool CheckVRRequirements();

    /**
     * @brief Get recommended demo settings for current hardware
     */
    DemoConfig GetRecommendedSettings();
}

} // namespace vrb