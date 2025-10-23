// mike_hrtf_verification.cpp - Mike Rodriguez HRTF Technical Verification
// MISSION: Verify HRTF implementation is REAL, not stub code
// Maria Santos demanded thorough verification - DO NOT APPROVE STUBS

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include "modules/audio/hrtf_processor.h"
#include "core/include/vr_types.h"

using namespace vrb;

class HRTFVerificationTest {
private:
    HRTFProcessor processor;
    static constexpr int BUFFER_SIZE = 512;
    static constexpr float SAMPLE_RATE = 48000.0f;

public:
    bool RunFullVerification() {
        std::cout << "=== MIKE RODRIGUEZ - HRTF TECHNICAL VERIFICATION ===" << std::endl;
        std::cout << "Mission: Verify REAL spatial audio implementation" << std::endl;
        std::cout << "Maria's Warning: Previous Mike fed to wolves for approving stubs" << std::endl;
        std::cout << std::endl;

        // Initialize processor
        if (!processor.Initialize("")) {
            std::cout << "❌ CRITICAL FAILURE: HRTF processor failed to initialize" << std::endl;
            return false;
        }
        std::cout << "✅ HRTF processor initialized successfully" << std::endl;

        bool allTestsPassed = true;
        allTestsPassed &= TestLeftRightDifferences();
        allTestsPassed &= TestSpatialPositioning();
        allTestsPassed &= TestConvolutionMath();
        allTestsPassed &= TestDistanceAttenuation();
        allTestsPassed &= TestStubDetection();

        std::cout << std::endl << "=== FINAL VERDICT ===" << std::endl;
        if (allTestsPassed) {
            std::cout << "✅ DEPLOYMENT APPROVED: HRTF implementation is REAL spatial audio" << std::endl;
            std::cout << "✅ Different L/R outputs verified for spatial positions" << std::endl;
            std::cout << "✅ Real convolution math confirmed, not stub code" << std::endl;
            std::cout << "✅ ASMRtist workflow will produce proper spatial audio" << std::endl;
        } else {
            std::cout << "❌ DEPLOYMENT BLOCKED: HRTF implementation has issues" << std::endl;
            std::cout << "❌ DO NOT APPROVE until all tests pass" << std::endl;
        }

        return allTestsPassed;
    }

private:
    void GenerateTestTone(float* buffer, int frames, float frequency = 440.0f) {
        for (int i = 0; i < frames; ++i) {
            buffer[i] = 0.8f * std::sin(2.0f * M_PI * frequency * i / SAMPLE_RATE);
        }
    }

    float CalculateRMSEnergy(const float* buffer, int frames) {
        float sum = 0.0f;
        for (int i = 0; i < frames; ++i) {
            sum += buffer[i] * buffer[i];
        }
        return std::sqrt(sum / frames);
    }

    bool TestLeftRightDifferences() {
        std::cout << std::endl << "TEST 1: Left/Right Channel Differences" << std::endl;

        float inputBuffer[BUFFER_SIZE];
        float leftOutput[BUFFER_SIZE * 2];
        float rightOutput[BUFFER_SIZE * 2];

        GenerateTestTone(inputBuffer, BUFFER_SIZE, 1000.0f);

        // Test far left position
        processor.SetListenerPosition(Vec3(-2.0f, 0.0f, 0.0f));
        processor.Process(inputBuffer, leftOutput, BUFFER_SIZE, 1);

        // Test far right position
        processor.SetListenerPosition(Vec3(2.0f, 0.0f, 0.0f));
        processor.Process(inputBuffer, rightOutput, BUFFER_SIZE, 1);

        // Extract channel energies
        float leftPos_LeftCh = 0.0f, leftPos_RightCh = 0.0f;
        float rightPos_LeftCh = 0.0f, rightPos_RightCh = 0.0f;

        for (int i = 0; i < BUFFER_SIZE; ++i) {
            leftPos_LeftCh += leftOutput[i * 2] * leftOutput[i * 2];
            leftPos_RightCh += leftOutput[i * 2 + 1] * leftOutput[i * 2 + 1];
            rightPos_LeftCh += rightOutput[i * 2] * rightOutput[i * 2];
            rightPos_RightCh += rightOutput[i * 2 + 1] * rightOutput[i * 2 + 1];
        }

        leftPos_LeftCh = std::sqrt(leftPos_LeftCh / BUFFER_SIZE);
        leftPos_RightCh = std::sqrt(leftPos_RightCh / BUFFER_SIZE);
        rightPos_LeftCh = std::sqrt(rightPos_LeftCh / BUFFER_SIZE);
        rightPos_RightCh = std::sqrt(rightPos_RightCh / BUFFER_SIZE);

        std::cout << std::fixed << std::setprecision(6);
        std::cout << "Left position  -> Left channel: " << leftPos_LeftCh << ", Right channel: " << leftPos_RightCh << std::endl;
        std::cout << "Right position -> Left channel: " << rightPos_LeftCh << ", Right channel: " << rightPos_RightCh << std::endl;

        // Check for spatial behavior
        bool leftPosFavorsLeft = leftPos_LeftCh > leftPos_RightCh;
        bool rightPosFavorsRight = rightPos_RightCh > rightPos_LeftCh;
        bool channelsDiffer = (leftPos_LeftCh != rightPos_LeftCh) && (leftPos_RightCh != rightPos_RightCh);

        std::cout << "✓ Left position favors left channel: " << (leftPosFavorsLeft ? "YES" : "NO") << std::endl;
        std::cout << "✓ Right position favors right channel: " << (rightPosFavorsRight ? "YES" : "NO") << std::endl;
        std::cout << "✓ Channels differ between positions: " << (channelsDiffer ? "YES" : "NO") << std::endl;

        bool passed = leftPosFavorsLeft && rightPosFavorsRight && channelsDiffer;
        std::cout << "Result: " << (passed ? "✅ PASSED" : "❌ FAILED - STUB DETECTED") << std::endl;

        return passed;
    }

    bool TestSpatialPositioning() {
        std::cout << std::endl << "TEST 2: Spatial Position Processing" << std::endl;

        float inputBuffer[BUFFER_SIZE];
        GenerateTestTone(inputBuffer, BUFFER_SIZE, 800.0f);

        struct TestPosition {
            Vec3 pos;
            std::string name;
        };

        std::vector<TestPosition> positions = {
            {{0.0f, 0.0f, -1.0f}, "Front"},
            {{0.0f, 0.0f, 1.0f}, "Behind"},
            {{0.0f, 1.0f, 0.0f}, "Above"},
            {{0.0f, -1.0f, 0.0f}, "Below"}
        };

        std::vector<float> totalEnergies;

        for (const auto& testPos : positions) {
            float outputBuffer[BUFFER_SIZE * 2];
            processor.SetListenerPosition(testPos.pos);
            processor.Process(inputBuffer, outputBuffer, BUFFER_SIZE, 1);

            float totalEnergy = 0.0f;
            for (int i = 0; i < BUFFER_SIZE * 2; ++i) {
                totalEnergy += outputBuffer[i] * outputBuffer[i];
            }
            totalEnergy = std::sqrt(totalEnergy / (BUFFER_SIZE * 2));
            totalEnergies.push_back(totalEnergy);

            std::cout << testPos.name << " position energy: " << std::fixed << std::setprecision(6) << totalEnergy << std::endl;
        }

        // Check if positions produce different outputs
        bool allDifferent = true;
        for (size_t i = 0; i < totalEnergies.size(); ++i) {
            for (size_t j = i + 1; j < totalEnergies.size(); ++j) {
                if (std::abs(totalEnergies[i] - totalEnergies[j]) < 0.0001f) {
                    allDifferent = false;
                    std::cout << "WARNING: " << positions[i].name << " and " << positions[j].name << " produce nearly identical outputs" << std::endl;
                }
            }
        }

        std::cout << "Result: " << (allDifferent ? "✅ PASSED" : "❌ FAILED - POSITIONS TOO SIMILAR") << std::endl;
        return allDifferent;
    }

    bool TestConvolutionMath() {
        std::cout << std::endl << "TEST 3: Convolution Mathematics Verification" << std::endl;

        // Test with impulse signal to verify convolution behavior
        float impulse[BUFFER_SIZE];
        std::fill_n(impulse, BUFFER_SIZE, 0.0f);
        impulse[0] = 1.0f;  // Unit impulse

        float outputBuffer[BUFFER_SIZE * 2];
        processor.SetListenerPosition(Vec3(-1.0f, 0.0f, 0.0f)); // Left position
        processor.Process(impulse, outputBuffer, BUFFER_SIZE, 1);

        // Check if we get a response that looks like an HRTF impulse response
        bool hasNonZeroResponse = false;
        bool hasReasonableLength = false;

        int nonZeroSamples = 0;
        for (int i = 0; i < BUFFER_SIZE * 2; ++i) {
            if (std::abs(outputBuffer[i]) > 0.0001f) {
                hasNonZeroResponse = true;
                nonZeroSamples++;
            }
        }

        // HRTF responses should span multiple samples (not just copy input)
        hasReasonableLength = nonZeroSamples > 10 && nonZeroSamples < BUFFER_SIZE;

        std::cout << "Non-zero output samples: " << nonZeroSamples << "/" << (BUFFER_SIZE * 2) << std::endl;
        std::cout << "✓ Has non-zero response: " << (hasNonZeroResponse ? "YES" : "NO") << std::endl;
        std::cout << "✓ Response length reasonable: " << (hasReasonableLength ? "YES" : "NO") << std::endl;

        bool passed = hasNonZeroResponse && hasReasonableLength;
        std::cout << "Result: " << (passed ? "✅ PASSED" : "❌ FAILED - CONVOLUTION ISSUES") << std::endl;

        return passed;
    }

    bool TestDistanceAttenuation() {
        std::cout << std::endl << "TEST 4: Distance Attenuation" << std::endl;

        float inputBuffer[BUFFER_SIZE];
        GenerateTestTone(inputBuffer, BUFFER_SIZE, 1000.0f);

        // Test near position
        float nearOutput[BUFFER_SIZE * 2];
        processor.SetListenerPosition(Vec3(0.2f, 0.0f, 0.0f));
        processor.Process(inputBuffer, nearOutput, BUFFER_SIZE, 1);

        // Test far position
        float farOutput[BUFFER_SIZE * 2];
        processor.SetListenerPosition(Vec3(3.0f, 0.0f, 0.0f));
        processor.Process(inputBuffer, farOutput, BUFFER_SIZE, 1);

        // Calculate total energies
        float nearEnergy = 0.0f, farEnergy = 0.0f;
        for (int i = 0; i < BUFFER_SIZE * 2; ++i) {
            nearEnergy += nearOutput[i] * nearOutput[i];
            farEnergy += farOutput[i] * farOutput[i];
        }
        nearEnergy = std::sqrt(nearEnergy / (BUFFER_SIZE * 2));
        farEnergy = std::sqrt(farEnergy / (BUFFER_SIZE * 2));

        std::cout << "Near position energy: " << std::fixed << std::setprecision(6) << nearEnergy << std::endl;
        std::cout << "Far position energy: " << farEnergy << std::endl;

        bool correctAttenuation = nearEnergy > farEnergy;
        float attenuationRatio = nearEnergy / (farEnergy + 0.0001f);
        bool sufficientAttenuation = attenuationRatio > 1.5f;

        std::cout << "Attenuation ratio: " << attenuationRatio << std::endl;
        std::cout << "✓ Near louder than far: " << (correctAttenuation ? "YES" : "NO") << std::endl;
        std::cout << "✓ Sufficient attenuation: " << (sufficientAttenuation ? "YES" : "NO") << std::endl;

        bool passed = correctAttenuation && sufficientAttenuation;
        std::cout << "Result: " << (passed ? "✅ PASSED" : "❌ FAILED - DISTANCE ISSUES") << std::endl;

        return passed;
    }

    bool TestStubDetection() {
        std::cout << std::endl << "TEST 5: Stub Implementation Detection" << std::endl;
        std::cout << "(This is the test that caught previous failures)" << std::endl;

        float inputBuffer[BUFFER_SIZE];
        GenerateTestTone(inputBuffer, BUFFER_SIZE, 500.0f);

        // Get stats from different positions
        processor.SetListenerPosition(Vec3(-1.0f, 0.0f, 0.0f));
        processor.Process(inputBuffer, nullptr, BUFFER_SIZE, 1); // Just update stats
        auto stats1 = processor.GetStats();

        processor.SetListenerPosition(Vec3(1.0f, 0.0f, 0.0f));
        processor.Process(inputBuffer, nullptr, BUFFER_SIZE, 1);
        auto stats2 = processor.GetStats();

        std::cout << "Position 1 stats: Az=" << stats1.azimuth << "°, El=" << stats1.elevation << "°, HRTF=" << stats1.hrtfIndex << std::endl;
        std::cout << "Position 2 stats: Az=" << stats2.azimuth << "°, El=" << stats2.elevation << "°, HRTF=" << stats2.hrtfIndex << std::endl;

        // Check if stats actually change
        bool anglesDiffer = (stats1.azimuth != stats2.azimuth) || (stats1.elevation != stats2.elevation);
        bool filtersDiffer = stats1.hrtfIndex != stats2.hrtfIndex;
        bool reasonable = (std::abs(stats1.azimuth) > 0.1f) && (std::abs(stats2.azimuth) > 0.1f);

        std::cout << "✓ Angles change between positions: " << (anglesDiffer ? "YES" : "NO") << std::endl;
        std::cout << "✓ HRTF filters differ: " << (filtersDiffer ? "YES" : "NO") << std::endl;
        std::cout << "✓ Angle calculations reasonable: " << (reasonable ? "YES" : "NO") << std::endl;

        bool passed = anglesDiffer && filtersDiffer && reasonable;
        std::cout << "Result: " << (passed ? "✅ PASSED" : "❌ FAILED - STUB BEHAVIOR DETECTED") << std::endl;

        return passed;
    }
};

int main() {
    HRTFVerificationTest test;
    bool success = test.RunFullVerification();
    return success ? 0 : 1;
}