// asmr_workflow_demo.cpp - Simple demonstration of ASMRtist workflow
// Shows the core use case: position microphone → move around → create spatial audio

#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>
#include "../modules/ui/audio_routing_overlay.h"
#include "../modules/vr/vr_tracker.h"
#include "../modules/audio/audio_engine.h"
#include "../modules/audio/hrtf_processor.h"
#include "../core/include/config.h"
#include "../core/include/logger.h"

using namespace vrb;

/**
 * @brief Simple demonstration of the ASMRtist workflow
 *
 * This shows the essential use case:
 * 1. ASMRtist puts on VR headset
 * 2. Positions virtual microphone in VR space
 * 3. Moves around to create different spatial audio effects
 * 4. Records/streams the spatialized output
 */
class ASMRWorkflowDemo {
public:
    bool Initialize() {
        std::cout << "🎤 ASMRtist VR Spatial Audio Demo\n";
        std::cout << "================================\n\n";

        // Load simple configuration
        if (!m_config.Load("config/vr_binaural_config.json")) {
            std::cout << "❌ Failed to load configuration\n";
            return false;
        }

        // Initialize audio engine
        if (!m_audioEngine.Initialize(m_config, &m_hrtfProcessor)) {
            std::cout << "❌ Failed to initialize audio engine\n";
            return false;
        }

        // Initialize HRTF processor for spatial audio
        if (!m_hrtfProcessor.Initialize(m_config)) {
            std::cout << "❌ Failed to initialize HRTF processor\n";
            return false;
        }

        // Initialize VR tracking (stub mode for demo)
        if (!m_vrTracker.Initialize()) {
            std::cout << "❌ Failed to initialize VR tracking\n";
            return false;
        }

        // Initialize simple microphone positioning interface
        if (!m_audioOverlay.Initialize(&m_vrTracker, &m_audioEngine)) {
            std::cout << "❌ Failed to initialize VR interface\n";
            return false;
        }

        // Connect HRTF processor to the interface
        m_audioOverlay.SetHRTFProcessor(&m_hrtfProcessor);

        std::cout << "✅ ASMRtist VR system initialized successfully!\n\n";
        return true;
    }

    void RunDemo() {
        std::cout << "🎧 Starting ASMRtist workflow demonstration...\n\n";

        // Demo Step 1: Position the virtual microphone
        DemoMicrophonePositioning();

        // Demo Step 2: Show spatial audio effects as user moves
        DemoSpatialMovement();

        // Demo Step 3: Recording workflow
        DemoRecordingWorkflow();

        std::cout << "\n🎉 ASMRtist workflow demo complete!\n";
        std::cout << "This shows how content creators can use VR to create immersive spatial audio.\n\n";
    }

    void Shutdown() {
        m_audioOverlay.Shutdown();
        m_vrTracker.Shutdown();
        m_hrtfProcessor.Shutdown();
        m_audioEngine.Shutdown();
    }

private:
    void DemoMicrophonePositioning() {
        std::cout << "📍 STEP 1: Positioning Virtual Microphone\n";
        std::cout << "----------------------------------------\n";

        // Show different microphone positions and their effects
        struct PositionDemo {
            Vec3 position;
            std::string description;
        };

        std::vector<PositionDemo> positions = {
            {{0, 1.5f, -1.0f}, "In front of listener (standard position)"},
            {{-0.5f, 1.5f, -0.5f}, "Left side close (intimate left ear effect)"},
            {{0.5f, 1.5f, -0.5f}, "Right side close (intimate right ear effect)"},
            {{0, 1.8f, -0.8f}, "Above head (overhead whisper effect)"},
            {{0, 1.2f, -1.2f}, "Below head level (close chest voice effect)"},
            {{0, 1.5f, -2.0f}, "Far in front (distant voice effect)"}
        };

        for (const auto& pos : positions) {
            std::cout << "🎤 Setting microphone: " << pos.description << "\n";
            std::cout << "   Position: (" << pos.position.x << ", " << pos.position.y << ", " << pos.position.z << ")\n";

            // Set microphone position
            m_audioOverlay.SetMicrophonePosition(pos.position);

            // Calculate and show spatial preview
            Vec3 userPos = {0, 1.5f, 0};  // Standard head position
            auto preview = asmr_utils::CalculatePreview(pos.position, userPos);

            std::cout << "   📊 Spatial Effect: Left=" << std::fixed << std::setprecision(1)
                      << preview.leftVolume << ", Right=" << preview.rightVolume
                      << ", Distance=" << preview.distance << "m\n\n";

            // Simulate brief pause for effect
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
        }
    }

    void DemoSpatialMovement() {
        std::cout << "🚶 STEP 2: Moving Around for Spatial Effects\n";
        std::cout << "-------------------------------------------\n";

        // Fix microphone at a specific position
        Vec3 micPos = {-0.3f, 1.6f, -0.8f};  // Left side, slightly above, close
        m_audioOverlay.SetMicrophonePosition(micPos);
        std::cout << "🎤 Microphone fixed at: Left ear position for ASMR effect\n\n";

        // Simulate user moving around the microphone
        std::vector<Vec3> userPositions = {
            {0, 1.5f, 0},        // Starting position
            {0.1f, 1.5f, 0},     // Slight right lean
            {-0.1f, 1.5f, 0},    // Slight left lean
            {0, 1.4f, 0},        // Duck down slightly
            {0, 1.6f, 0},        // Stand up taller
            {0, 1.5f, 0.1f},     // Lean forward slightly
            {0, 1.5f, -0.1f}     // Lean back slightly
        };

        std::cout << "👂 Simulating listener movement to demonstrate spatial audio:\n\n";

        for (size_t i = 0; i < userPositions.size(); ++i) {
            const auto& userPos = userPositions[i];
            auto preview = asmr_utils::CalculatePreview(micPos, userPos);

            std::cout << "Position " << (i+1) << ": ";
            if (i == 0) std::cout << "Starting position";
            else if (i == 1) std::cout << "Lean right";
            else if (i == 2) std::cout << "Lean left";
            else if (i == 3) std::cout << "Duck down";
            else if (i == 4) std::cout << "Stand taller";
            else if (i == 5) std::cout << "Lean forward";
            else if (i == 6) std::cout << "Lean back";

            std::cout << "\n   🔊 Audio Effect: ";

            if (preview.leftVolume > preview.rightVolume * 1.2f) {
                std::cout << "Strong LEFT ear effect";
            } else if (preview.rightVolume > preview.leftVolume * 1.2f) {
                std::cout << "Strong RIGHT ear effect";
            } else {
                std::cout << "Balanced stereo effect";
            }

            std::cout << " (L:" << std::fixed << std::setprecision(2)
                      << preview.leftVolume << " R:" << preview.rightVolume << ")\n\n";

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

    void DemoRecordingWorkflow() {
        std::cout << "📹 STEP 3: Recording Workflow for Content Creators\n";
        std::cout << "------------------------------------------------\n";

        // Position microphone for recording
        Vec3 recordingPos = {-0.2f, 1.6f, -0.6f};  // Good ASMR position
        m_audioOverlay.SetMicrophonePosition(recordingPos);

        std::cout << "🎤 Positioning microphone for ASMR recording session...\n";
        std::cout << "   Position: Close left ear, slightly above head level\n\n";

        // Enable monitoring so ASMRtist can hear the effect
        std::cout << "🎧 Enabling real-time monitoring...\n";
        m_audioOverlay.SetMonitoring(true);
        std::cout << "   ✅ ASMRtist can now hear spatial effects in real-time\n\n";

        // Start recording
        std::cout << "🔴 Starting recording...\n";
        m_audioOverlay.StartRecording();
        std::cout << "   ✅ Recording active - all movements create spatial audio\n\n";

        // Simulate recording session with different content types
        std::vector<std::string> asmrContent = {
            "Gentle whispers in left ear",
            "Soft tapping sounds close to microphone",
            "Brushing sounds with subtle movement",
            "Page turning with spatial depth",
            "Rain stick with distance variations"
        };

        for (const auto& content : asmrContent) {
            std::cout << "🎵 Recording: " << content << "\n";

            // Show audio levels during recording
            std::cout << "   📊 Input Level: ";
            for (int i = 0; i < 10; ++i) {
                std::cout << "█";
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            std::cout << " [Active]\n\n";
        }

        // Stop recording
        std::cout << "⏹️ Stopping recording...\n";
        m_audioOverlay.StopRecording();
        std::cout << "   ✅ Recording saved with spatial audio positioning\n\n";

        // Disable monitoring
        m_audioOverlay.SetMonitoring(false);
        std::cout << "🎧 Monitoring disabled\n\n";
    }

    // System components
    Config m_config;
    AudioEngine m_audioEngine;
    HRTFProcessor m_hrtfProcessor;
    VRTracker m_vrTracker;
    AudioRoutingOverlay m_audioOverlay;
};

int main() {
    // Initialize logging
    vrb::Logger::Initialize(vrb::LogLevel::Info);

    try {
        ASMRWorkflowDemo demo;

        if (!demo.Initialize()) {
            std::cout << "❌ Failed to initialize demo\n";
            return 1;
        }

        demo.RunDemo();
        demo.Shutdown();

        std::cout << "💡 Key Benefits for ASMRtists:\n";
        std::cout << "  • Simple microphone positioning in VR space\n";
        std::cout << "  • Real-time spatial audio preview\n";
        std::cout << "  • Intuitive drag-to-position controls\n";
        std::cout << "  • Immediate feedback on spatial effects\n";
        std::cout << "  • Focus on content creation, not technical complexity\n\n";

        return 0;

    } catch (const std::exception& e) {
        std::cout << "❌ Demo failed with exception: " << e.what() << "\n";
        return 1;
    }
}