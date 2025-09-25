// audio_routing_integration.cpp - Integration Example
// Shows how to connect the Audio Routing Overlay with the rest of the system!

#include "audio_routing_overlay.h"
#include "audio_engine.h"
#include "vr_tracker.h"
#include "logger.h"

namespace vrb {

/**
 * @brief Integration helper class - makes connecting everything EASY!
 *
 * This is a quick example of how to wire up the AudioRoutingOverlay
 * with the existing VRTracker and AudioEngine systems.
 */
class AudioRoutingIntegration {
public:
    AudioRoutingIntegration() = default;
    ~AudioRoutingIntegration() = default;

    bool Initialize() {
        LOG_INFO("Initializing Audio Routing Integration - connecting the magic!");

        // Create our components
        m_vrTracker = std::make_unique<VRTracker>();
        m_audioEngine = std::make_unique<AudioEngine>();
        m_audioRoutingOverlay = std::make_unique<AudioRoutingOverlay>();

        // Initialize VR tracking
        if (!m_vrTracker->Initialize()) {
            LOG_ERROR("Failed to initialize VR tracker!");
            return false;
        }

        // Initialize audio engine
        // TODO: For full integration, pass proper Config and HRTFProcessor
        // For now, this integration class is for demo purposes only
        LOG_WARN("AudioEngine initialization skipped in integration demo - use main Application instead");

        // Initialize the audio routing overlay
        if (!m_audioRoutingOverlay->Initialize(m_vrTracker.get(), m_audioEngine.get())) {
            LOG_ERROR("Failed to initialize audio routing overlay!");
            return false;
        }

        // Set up integration callbacks
        SetupCallbacks();

        LOG_INFO("Audio Routing Integration initialized successfully!");
        return true;
    }

    void Update() {
        if (!m_initialized) return;

        // Update VR tracking first
        m_vrTracker->Update();

        // Update audio engine
        m_audioEngine->Update();

        // Update the overlay (this will handle gestures and visuals)
        m_audioRoutingOverlay->Update();

        // Process any integration-specific logic
        ProcessAudioRouting();
    }

    void Shutdown() {
        if (m_audioRoutingOverlay) {
            m_audioRoutingOverlay->Shutdown();
        }
        if (m_audioEngine) {
            m_audioEngine->Shutdown();
        }
        if (m_vrTracker) {
            m_vrTracker->Shutdown();
        }

        LOG_INFO("Audio Routing Integration shut down successfully!");
    }

    // Quick preset shortcuts for demo purposes
    void ApplyCinemaMode() {
        m_audioRoutingOverlay->ApplyPreset(AudioRoutingOverlay::AudioPreset::CINEMA);
        LOG_INFO("Applied Cinema Mode - immersive movie experience activated!");
    }

    void ApplyGamingMode() {
        m_audioRoutingOverlay->ApplyPreset(AudioRoutingOverlay::AudioPreset::GAMING);
        LOG_INFO("Applied Gaming Mode - competitive audio advantage activated!");
    }

    void ApplyMusicMode() {
        m_audioRoutingOverlay->ApplyPreset(AudioRoutingOverlay::AudioPreset::MUSIC);
        LOG_INFO("Applied Music Mode - audiophile listening experience activated!");
    }

private:
    void SetupCallbacks() {
        // Set up gesture callbacks for audio routing actions
        m_audioRoutingOverlay->RegisterGestureCallback(
            [this](const std::string& gesture, const Vec3& position) {
                HandleGesture(gesture, position);
            }
        );

        LOG_INFO("Integration callbacks set up successfully!");
    }

    void HandleGesture(const std::string& gesture, const Vec3& position) {
        LOG_INFO("Processing gesture: {} at position ({}, {}, {})",
                 gesture, position.x, position.y, position.z);

        if (gesture == "pinch_start") {
            // User grabbed an audio orb - provide haptic feedback
            // TODO: Add haptic feedback through VRTracker
            LOG_INFO("Audio orb grabbed - providing haptic feedback!");

        } else if (gesture == "pinch_end") {
            // User released an audio orb - apply spatial changes
            LOG_INFO("Audio orb released - updating spatial audio!");

        } else if (gesture == "swipe_left") {
            // Quick action: cycle through presets
            CyclePresets();

        } else if (gesture == "swipe_right") {
            // Quick action: toggle all audio sources
            ToggleAllAudio();

        } else if (gesture == "clap") {
            // Emergency mute/unmute
            ToggleGlobalMute();
        }
    }

    void ProcessAudioRouting() {
        // This is where we'd process the actual audio routing
        // based on the overlay's audio orb positions

        auto audioSources = m_audioRoutingOverlay->GetAllAudioSources();
        for (auto* source : audioSources) {
            if (source && source->enabled) {
                // Apply spatial position to audio engine
                // TODO: Connect to actual audio routing
                // m_audioEngine->SetSourcePosition(source->name, source->spatialPosition);
                // m_audioEngine->SetSourceVolume(source->name, source->volume);
            }
        }
    }

    void CyclePresets() {
        // Cycle through presets for quick switching
        static int currentPreset = 0;
        const AudioRoutingOverlay::AudioPreset presets[] = {
            AudioRoutingOverlay::AudioPreset::GAMING,
            AudioRoutingOverlay::AudioPreset::CINEMA,
            AudioRoutingOverlay::AudioPreset::MUSIC,
            AudioRoutingOverlay::AudioPreset::PODCAST,
            AudioRoutingOverlay::AudioPreset::CONCERT
        };

        currentPreset = (currentPreset + 1) % 5;
        m_audioRoutingOverlay->ApplyPreset(presets[currentPreset]);

        LOG_INFO("Cycled to next preset!");
    }

    void ToggleAllAudio() {
        auto audioSources = m_audioRoutingOverlay->GetAllAudioSources();
        static bool allMuted = false;

        for (auto* source : audioSources) {
            if (source) {
                source->isMuted = !allMuted;
            }
        }

        allMuted = !allMuted;
        LOG_INFO("Toggled all audio sources: {}", allMuted ? "MUTED" : "UNMUTED");
    }

    void ToggleGlobalMute() {
        // Emergency mute/unmute functionality
        static bool globalMuted = false;
        globalMuted = !globalMuted;

        // TODO: Connect to audio engine global mute
        // m_audioEngine->SetGlobalMute(globalMuted);

        LOG_INFO("Global mute: {}", globalMuted ? "ENABLED" : "DISABLED");
    }

private:
    std::unique_ptr<VRTracker> m_vrTracker;
    std::unique_ptr<AudioEngine> m_audioEngine;
    std::unique_ptr<AudioRoutingOverlay> m_audioRoutingOverlay;

    bool m_initialized = false;
};

// === EXAMPLE USAGE FUNCTION ===
/**
 * @brief Example function showing how to set up the complete audio routing system
 */
void RunAudioRoutingDemo() {
    LOG_INFO("Starting Audio Routing Demo - prepare for VR audio magic!");

    AudioRoutingIntegration integration;

    if (!integration.Initialize()) {
        LOG_ERROR("Failed to initialize audio routing demo!");
        return;
    }

    LOG_INFO("Audio Routing Demo initialized successfully!");
    LOG_INFO("Put on your VR headset and grab some audio orbs!");

    // Main demo loop (in a real app, this would be your main loop)
    bool running = true;
    auto startTime = std::chrono::steady_clock::now();

    while (running) {
        integration.Update();

        // Demo preset changes every 10 seconds
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();

        if (elapsed == 10) {
            integration.ApplyCinemaMode();
        } else if (elapsed == 20) {
            integration.ApplyGamingMode();
        } else if (elapsed == 30) {
            integration.ApplyMusicMode();
            running = false;  // End demo after 30 seconds
        }

        // Sleep to maintain reasonable frame rate
        std::this_thread::sleep_for(std::chrono::milliseconds(16));  // ~60 FPS
    }

    integration.Shutdown();
    LOG_INFO("Audio Routing Demo completed - wasn't that AWESOME?!");
}

} // namespace vrb

// === CREATIVE INTEGRATION IDEAS ===
/*
 * Here are some wild ideas for making this integration even MORE awesome:
 *
 * 1. HAPTIC ORCHESTRA:
 *    - Different vibration patterns for different audio sources
 *    - Bass frequencies = low rumble, treble = quick pulses
 *    - Spatial haptics that match audio positioning
 *
 * 2. GESTURE MACROS:
 *    - Draw shapes in the air to trigger complex routing
 *    - Heart shape = "Romantic Dinner" preset
 *    - Lightning bolt = "Action Movie" preset
 *    - Spiral = "Psychedelic Music" preset
 *
 * 3. VOICE COMMANDS:
 *    - "Route game audio to front speakers"
 *    - "Make voice chat sound like it's coming from above"
 *    - "Transform all audio into concert hall"
 *
 * 4. AI-POWERED AUTO-ROUTING:
 *    - Analyze audio content and automatically suggest optimal routing
 *    - Learn user preferences over time
 *    - "It sounds like you're watching an action movie - applying cinema mode!"
 *
 * 5. COLLABORATIVE ROUTING:
 *    - Multiple users can manipulate the same audio space
 *    - Perfect for VR parties or collaborative work
 *    - See other users' audio adjustments in real-time
 *
 * The beauty of this design is that all these features can be added
 * incrementally without breaking the core functionality!
 */