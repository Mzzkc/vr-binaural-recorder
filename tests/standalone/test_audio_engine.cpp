// test_audio_engine.cpp - Standalone test for the audio engine
// This tests the audio engine without VR dependencies

#include "audio_engine.h"
#include "config.h"
#include "logger.h"
#include "hrtf_processor.h"

#include <iostream>
#include <thread>
#include <chrono>

using namespace vrb;

int main() {
    std::cout << "VR Binaural Recorder - Audio Engine Test\n";
    std::cout << "=========================================\n";

    // Initialize logging
    Logger::Initialize("debug", "./logs", true);
    LOG_INFO("Starting audio engine test");

    // Create configuration
    Config config;

    // Create a simple HRTF processor (we'll just use a stub for testing)
    HRTFProcessor hrtf;

    // Create audio engine
    AudioEngine audioEngine;

    // Initialize the engine
    if (!audioEngine.Initialize(config, &hrtf)) {
        LOG_ERROR("Failed to initialize audio engine");
        return 1;
    }

    // Get input devices
    auto devices = audioEngine.GetInputDevices();
    std::cout << "\nFound " << devices.size() << " input devices:\n";
    for (size_t i = 0; i < devices.size(); ++i) {
        const auto& device = devices[i];
        std::cout << "  [" << device.index << "] " << device.name
                  << " (" << device.maxInputChannels << " channels"
                  << ", " << device.defaultSampleRate << "Hz"
                  << ", Host API: " << static_cast<int>(device.hostAPI) << ")\n";
    }

    // Select first device if available
    if (!devices.empty()) {
        std::cout << "\nSelecting device: " << devices[0].name << "\n";
        if (!audioEngine.SelectInputDevice(devices[0].index)) {
            LOG_ERROR("Failed to select input device");
            return 1;
        }
    }

    // Try to start the engine
    std::cout << "\nStarting audio engine...\n";
    if (!audioEngine.Start()) {
        LOG_ERROR("Failed to start audio engine");
        return 1;
    }

    std::cout << "Audio engine running successfully!\n";

    // Let it run for a few seconds to collect stats
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Get and display statistics
    auto stats = audioEngine.GetStats();
    std::cout << "\nAudio Engine Statistics:\n";
    std::cout << "  CPU Load: " << (stats.cpuLoad * 100.0f) << "%\n";
    std::cout << "  Frames Processed: " << stats.framesProcessed << "\n";
    std::cout << "  Input Latency: " << (stats.inputLatency * 1000.0) << "ms\n";
    std::cout << "  Output Latency: " << (stats.outputLatency * 1000.0) << "ms\n";
    std::cout << "  Underruns: " << stats.underruns << "\n";
    std::cout << "  Overruns: " << stats.overruns << "\n";
    std::cout << "  Peak Input Level: " << stats.peakInputLevel << "\n";
    std::cout << "  Peak Output Level: " << stats.peakOutputLevel << "\n";
    std::cout << "  Callback Duration: " << stats.callbackDuration.count() << "μs\n";

    // Get stream info
    auto streamInfo = audioEngine.GetStreamInfo();
    std::cout << "\nStream Information:\n";
    std::cout << "  Active: " << (streamInfo.isActive ? "Yes" : "No") << "\n";
    std::cout << "  Sample Rate: " << streamInfo.sampleRate << "Hz\n";
    std::cout << "  Buffer Size: " << streamInfo.bufferSize << " samples\n";
    std::cout << "  Total XRuns: " << streamInfo.xruns << "\n";

    // Stop the engine
    std::cout << "\nStopping audio engine...\n";
    audioEngine.Stop();

    std::cout << "Audio engine test completed successfully!\n";

    Logger::Shutdown();
    return 0;
}