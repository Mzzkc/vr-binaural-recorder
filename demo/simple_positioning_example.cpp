// simple_positioning_example.cpp - Simple Example of Epic Microphone Positioning!
// This shows how easy it is to use the controller-based microphone positioning system!
// Perfect for integration into existing VR applications!

#include <iostream>
#include <chrono>
#include <thread>
#include "vr_tracker.h"
#include "microphone_positioning.h"
#include "microphone_visualizer.h"
#include "microphone_audio_integration.h"
#include "audio_engine.h"
#include "hrtf_processor.h"
#include "config.h"
#include "logger.h"

using namespace vrb;

/**
 * @brief Simple example demonstrating the epic microphone positioning system
 * This shows the basic integration pattern for VR applications
 */
int main() {
    std::cout << "=== EPIC VR MICROPHONE POSITIONING EXAMPLE ===" << std::endl;
    std::cout << "This demonstrates controller-based spatial audio positioning!" << std::endl;
    std::cout << "Grab your VR controllers and position virtual microphones!" << std::endl;

    // Initialize logging
    LOG_INFO("Simple Positioning Example: Starting epic demonstration!");

    // Load configuration
    Config config;
    // Use default configuration for this example

    // 1. CREATE AND INITIALIZE VR TRACKER
    std::cout << "\n1. Setting up VR tracking..." << std::endl;
    auto vrTracker = std::make_unique<VRTracker>();
    if (!vrTracker->Initialize()) {
        std::cerr << "ERROR: Failed to initialize VR tracker!" << std::endl;
        std::cerr << "Make sure SteamVR is running and headset is connected." << std::endl;
        return -1;
    }
    std::cout << "✓ VR tracker initialized successfully!" << std::endl;

    // 2. CREATE MICROPHONE POSITIONING SYSTEM
    std::cout << "\n2. Setting up microphone positioning..." << std::endl;
    auto positioning = std::make_unique<MicrophonePositioning>();
    if (!positioning->Initialize(vrTracker.get())) {
        std::cerr << "ERROR: Failed to initialize microphone positioning!" << std::endl;
        return -1;
    }
    std::cout << "✓ Microphone positioning system ready!" << std::endl;

    // 3. CREATE HRTF PROCESSOR FOR SPATIAL AUDIO
    std::cout << "\n3. Setting up spatial audio processing..." << std::endl;
    auto hrtfProcessor = std::make_unique<HRTFProcessor>();
    if (!hrtfProcessor->Initialize("data/hrtf/")) {
        std::cout << "⚠ HRTF processor not available - using basic audio processing" << std::endl;
    } else {
        std::cout << "✓ HRTF processor initialized!" << std::endl;
    }

    // 4. CREATE AUDIO ENGINE
    std::cout << "\n4. Setting up audio engine..." << std::endl;
    auto audioEngine = std::make_unique<AudioEngine>();
    if (!audioEngine->Initialize(config, hrtfProcessor.get())) {
        std::cout << "⚠ Audio engine initialization failed - continuing without audio" << std::endl;
    } else {
        std::cout << "✓ Audio engine ready!" << std::endl;
    }

    // 5. CREATE AUDIO INTEGRATION SYSTEM
    std::cout << "\n5. Setting up audio integration..." << std::endl;
    auto audioIntegration = std::make_unique<MicrophoneAudioIntegration>();
    if (!audioIntegration->Initialize(hrtfProcessor.get(), audioEngine.get())) {
        std::cout << "⚠ Audio integration failed - positioning will work without audio" << std::endl;
    } else {
        // Connect positioning to audio integration
        if (audioIntegration->ConnectPositioning(positioning.get())) {
            std::cout << "✓ Audio integration connected!" << std::endl;
        }
    }

    // 6. START AUDIO PROCESSING
    std::cout << "\n6. Starting audio processing..." << std::endl;
    if (audioEngine && audioEngine->Start()) {
        std::cout << "✓ Audio engine started!" << std::endl;
    }
    if (audioIntegration && audioIntegration->Start()) {
        std::cout << "✓ Audio integration started!" << std::endl;
    }

    // 7. SET UP POSITIONING CALLBACKS FOR FEEDBACK
    std::cout << "\n7. Setting up feedback callbacks..." << std::endl;
    positioning->SetPositionUpdateCallback([](const VirtualMicrophone& leftMic,
                                             const VirtualMicrophone& rightMic,
                                             const StereoPositioning& stereoInfo) {
        // Print positioning feedback
        static int updateCount = 0;
        updateCount++;

        if (updateCount % 30 == 0) {  // Print every ~0.5 seconds at 60 FPS
            std::cout << "\r🎤 Distance: " << std::fixed << std::setprecision(1)
                     << (stereoInfo.distance * 100.0f) << "cm | "
                     << "Quality: " << (stereoInfo.GetQualityScore() * 100.0f) << "% | "
                     << "Left: " << (leftMic.isGrabbed ? "GRABBED" : (leftMic.isLocked ? "LOCKED" : "FREE")) << " | "
                     << "Right: " << (rightMic.isGrabbed ? "GRABBED" : (rightMic.isLocked ? "LOCKED" : "FREE"))
                     << std::flush;
        }
    });

    // 8. MAIN POSITIONING LOOP
    std::cout << "\n\n=== EPIC POSITIONING MODE ACTIVE! ===" << std::endl;
    std::cout << "🎮 INSTRUCTIONS:" << std::endl;
    std::cout << "  • Pull TRIGGERS to grab and position microphones" << std::endl;
    std::cout << "  • Release TRIGGERS to lock microphones in place" << std::endl;
    std::cout << "  • Press GRIP buttons to reset microphones to controllers" << std::endl;
    std::cout << "  • Optimal distance: 10-30cm apart (17cm ideal)" << std::endl;
    std::cout << "  • Press Ctrl+C to exit" << std::endl;
    std::cout << "\nPositioning feedback:" << std::endl;

    auto startTime = std::chrono::steady_clock::now();
    int frameCount = 0;

    try {
        while (true) {
            // Update VR tracking
            vrTracker->Update();

            // Update microphone positioning (this is where the magic happens!)
            positioning->Update();

            // Simple frame rate limiting
            std::this_thread::sleep_for(std::chrono::milliseconds(16));  // ~60 FPS
            frameCount++;

            // Print status every 5 seconds
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);
            if (elapsed.count() > 0 && elapsed.count() % 5 == 0 && frameCount % 300 == 0) {
                std::cout << "\n📊 Session stats: " << elapsed.count() << "s elapsed, "
                         << "~" << (frameCount / elapsed.count()) << " FPS" << std::endl;

                // Get current microphone states
                auto leftMic = positioning->GetLeftMicrophone();
                auto rightMic = positioning->GetRightMicrophone();
                auto stereoInfo = positioning->GetStereoPositioning();

                std::cout << "🎤 Current state:" << std::endl;
                std::cout << "   Left mic:  pos(" << leftMic.pose.position.x << ", "
                         << leftMic.pose.position.y << ", " << leftMic.pose.position.z << ") "
                         << (leftMic.isGrabbed ? "[GRABBED]" : (leftMic.isLocked ? "[LOCKED]" : "[FREE]")) << std::endl;
                std::cout << "   Right mic: pos(" << rightMic.pose.position.x << ", "
                         << rightMic.pose.position.y << ", " << rightMic.pose.position.z << ") "
                         << (rightMic.isGrabbed ? "[GRABBED]" : (rightMic.isLocked ? "[LOCKED]" : "[FREE]")) << std::endl;
                std::cout << "   Quality: " << (stereoInfo.GetQualityScore() * 100.0f) << "% | "
                         << "Distance: " << (stereoInfo.distance * 100.0f) << "cm" << std::endl;
                std::cout << "\nPositioning feedback:" << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cout << "\nCaught exception: " << e.what() << std::endl;
    }

    // 9. CLEANUP
    std::cout << "\n\n=== SHUTTING DOWN ===" << std::endl;

    if (audioIntegration) {
        audioIntegration->Stop();
        std::cout << "✓ Audio integration stopped" << std::endl;
    }

    if (audioEngine) {
        audioEngine->Stop();
        std::cout << "✓ Audio engine stopped" << std::endl;
    }

    std::cout << "✓ Epic positioning session complete!" << std::endl;
    std::cout << "Thanks for trying the amazing VR microphone positioning system!" << std::endl;

    return 0;
}

/*
COMPILATION INSTRUCTIONS:

To compile this example:

1. Make sure all dependencies are available:
   - OpenVR SDK
   - PortAudio
   - OpenGL/GLEW
   - Your VR-binaural-recorder libraries

2. Compile with:
   g++ -std=c++17 simple_positioning_example.cpp \
       -I../modules/vr -I../modules/audio -I../core/include \
       -L../build -lvr_tracker -lmicrophone_positioning -laudio_engine \
       -lopenvr_api -lportaudio -lGL -lGLEW \
       -pthread -o simple_positioning_example

3. Run with VR headset connected:
   ./simple_positioning_example

USAGE NOTES:

- Ensure SteamVR is running before starting
- Connect VR headset and at least 2 controllers
- The example will provide real-time feedback on microphone positioning
- Use triggers to grab and position virtual microphones
- Optimal microphone distance is 17cm (human head width)
- Audio integration provides spatial audio preview if HRTF data is available

This example demonstrates the core functionality and can be easily integrated
into larger VR applications or used as a starting point for custom implementations.
*/