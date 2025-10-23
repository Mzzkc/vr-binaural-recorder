// Exact reproduction of failing distance attenuation test
// Mike Rodriguez - Debug the actual test failure

#include <iostream>
#include <cmath>
#include <vector>
#include <memory>

// Minimal Vec3 implementation
struct Vec3 {
    float x, y, z;
    Vec3(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {}
};

// Mock HRTFProcessor to test the distance attenuation logic
class MockHRTFProcessor {
private:
    float currentDistance = 1.0f;

public:
    void SetListenerPosition(const Vec3& position) {
        // Calculate distance exactly like the real implementation
        currentDistance = std::sqrt(position.x * position.x +
                                  position.y * position.y +
                                  position.z * position.z);
        std::cout << "Position (" << position.x << ", " << position.y << ", " << position.z << ") -> Distance: " << currentDistance << std::endl;
    }

    void Process(const float* input, float* output, size_t frames, int inputChannels) {
        // Apply distance attenuation exactly like the real implementation
        float attenuation = 1.0f;
        if (currentDistance > 0.1f) {
            attenuation = std::min(1.0f, 1.0f / (currentDistance * currentDistance * 0.1f + 0.1f));
        }

        std::cout << "Distance: " << currentDistance << " -> Attenuation: " << attenuation << std::endl;

        // Simple processing: copy input to stereo output with attenuation
        for (size_t i = 0; i < frames; ++i) {
            output[i * 2] = input[i] * attenuation;     // Left channel
            output[i * 2 + 1] = input[i] * attenuation; // Right channel
        }
    }
};

void GenerateTestTone(float* buffer, size_t frames, float frequency, float amplitude) {
    for (size_t i = 0; i < frames; ++i) {
        buffer[i] = amplitude * std::sin(2.0f * M_PI * frequency * i / 48000.0f);
    }
}

int main() {
    const size_t BUFFER_SIZE = 512;

    std::cout << "=== REPRODUCING DISTANCE ATTENUATION TEST FAILURE ===" << std::endl;

    MockHRTFProcessor processor;

    float inputBuffer[BUFFER_SIZE];
    float outputNear[BUFFER_SIZE * 2];
    float outputFar[BUFFER_SIZE * 2];

    // Generate test signal exactly like the failing test
    GenerateTestTone(inputBuffer, BUFFER_SIZE, 1000.0f, 1.0f);

    std::cout << "\n--- Test Position 1: Close distance (0.1, 0, 0) ---" << std::endl;
    processor.SetListenerPosition(Vec3(0.1f, 0.0f, 0.0f));
    processor.Process(inputBuffer, outputNear, BUFFER_SIZE, 1);

    std::cout << "\n--- Test Position 2: Far distance (5.0, 0, 0) ---" << std::endl;
    processor.SetListenerPosition(Vec3(5.0f, 0.0f, 0.0f));
    processor.Process(inputBuffer, outputFar, BUFFER_SIZE, 1);

    // Calculate total energies exactly like the failing test
    float energyNear = 0.0f, energyFar = 0.0f;
    for (int i = 0; i < BUFFER_SIZE * 2; ++i) {
        energyNear += outputNear[i] * outputNear[i];
        energyFar += outputFar[i] * outputFar[i];
    }

    std::cout << "\n=== ENERGY COMPARISON ===" << std::endl;
    std::cout << "Near energy: " << energyNear << std::endl;
    std::cout << "Far energy:  " << energyFar << std::endl;
    std::cout << "Ratio (near/far): " << (energyNear / (energyFar + 0.0001f)) << std::endl;

    // Test the exact assertions from the failing test
    std::cout << "\n=== ASSERTIONS ===" << std::endl;

    if (energyNear > energyFar) {
        std::cout << "✅ PASS: energyNear > energyFar (" << energyNear << " > " << energyFar << ")" << std::endl;
    } else {
        std::cout << "❌ FAIL: energyNear <= energyFar (" << energyNear << " <= " << energyFar << ")" << std::endl;
        std::cout << "   This would cause: DEPLOYMENT BLOCKED: Distance attenuation not working" << std::endl;
    }

    float attenuationRatio = energyNear / (energyFar + 0.0001f);
    if (attenuationRatio > 2.0f) {
        std::cout << "✅ PASS: attenuationRatio > 2.0 (" << attenuationRatio << " > 2.0)" << std::endl;
    } else {
        std::cout << "❌ FAIL: attenuationRatio <= 2.0 (" << attenuationRatio << " <= 2.0)" << std::endl;
        std::cout << "   This would cause: DEPLOYMENT BLOCKED: Distance attenuation insufficient" << std::endl;
    }

    // Show what the real test would report
    if (energyNear > energyFar && attenuationRatio > 2.0f) {
        std::cout << "\n🎯 CONCLUSION: Distance attenuation test should PASS" << std::endl;
    } else {
        std::cout << "\n🚨 CONCLUSION: Distance attenuation test should FAIL" << std::endl;
        std::cout << "   This explains the deployment blocking issue reported by Jordan" << std::endl;
    }

    return 0;
}