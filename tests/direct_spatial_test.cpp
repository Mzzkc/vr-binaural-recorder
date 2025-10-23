// direct_spatial_test.cpp - Direct test of Maria's HRTF spatial audio
#include "modules/audio/hrtf_processor.h"
#include <iostream>
#include <cmath>

using namespace vrb;

int main() {
    std::cout << "=== DIRECT SPATIAL AUDIO TEST ===" << std::endl;

    // Create HRTF processor
    HRTFProcessor processor;

    // Initialize with synthetic HRTF
    if (!processor.Initialize("")) {
        std::cout << "FAILED: Could not initialize HRTF processor" << std::endl;
        return 1;
    }

    const int FRAMES = 512;
    float input[FRAMES];
    float outputLeft[FRAMES * 2];
    float outputRight[FRAMES * 2];

    // Generate test tone
    for (int i = 0; i < FRAMES; ++i) {
        input[i] = 0.5f * std::sin(2.0f * M_PI * 440.0f * i / 48000.0f);
    }

    // Test 1: Far LEFT position
    std::cout << "Testing FAR LEFT position (-2, 0, 0)" << std::endl;
    processor.SetListenerPosition(Vec3(-2.0f, 0.0f, 0.0f));
    processor.Process(input, outputLeft, FRAMES, 1);

    // Calculate channel energies
    float leftEnergyL = 0.0f, rightEnergyL = 0.0f;
    for (int i = 0; i < FRAMES; ++i) {
        leftEnergyL += outputLeft[i*2] * outputLeft[i*2];
        rightEnergyL += outputLeft[i*2+1] * outputLeft[i*2+1];
    }

    std::cout << "Left position - Left channel energy: " << leftEnergyL << std::endl;
    std::cout << "Left position - Right channel energy: " << rightEnergyL << std::endl;

    // Test 2: Far RIGHT position
    std::cout << "Testing FAR RIGHT position (2, 0, 0)" << std::endl;
    processor.SetListenerPosition(Vec3(2.0f, 0.0f, 0.0f));
    processor.Process(input, outputRight, FRAMES, 1);

    // Calculate channel energies
    float leftEnergyR = 0.0f, rightEnergyR = 0.0f;
    for (int i = 0; i < FRAMES; ++i) {
        leftEnergyR += outputRight[i*2] * outputRight[i*2];
        rightEnergyR += outputRight[i*2+1] * outputRight[i*2+1];
    }

    std::cout << "Right position - Left channel energy: " << leftEnergyR << std::endl;
    std::cout << "Right position - Right channel energy: " << rightEnergyR << std::endl;

    // Analysis
    std::cout << "\n=== SPATIAL AUDIO ANALYSIS ===" << std::endl;

    // Check if left position favors left channel
    bool leftCorrect = leftEnergyL > rightEnergyL;
    std::cout << "Left position favors left channel: " << (leftCorrect ? "YES" : "NO") << std::endl;

    // Check if right position favors right channel
    bool rightCorrect = rightEnergyR > leftEnergyR;
    std::cout << "Right position favors right channel: " << (rightCorrect ? "YES" : "NO") << std::endl;

    // Check for identical outputs (previous failure pattern)
    bool identical = (leftEnergyL == leftEnergyR) && (rightEnergyL == rightEnergyR);
    std::cout << "Outputs identical: " << (identical ? "YES (STUB CODE)" : "NO (WORKING)") << std::endl;

    // Final verdict
    if (leftCorrect && rightCorrect && !identical) {
        std::cout << "\n✅ SPATIAL AUDIO IS WORKING!" << std::endl;
        std::cout << "ASMRtist left/right ear positioning will work correctly" << std::endl;
        return 0;
    } else {
        std::cout << "\n❌ SPATIAL AUDIO IS BROKEN!" << std::endl;
        std::cout << "Deployment must be BLOCKED until fixed" << std::endl;
        return 1;
    }
}