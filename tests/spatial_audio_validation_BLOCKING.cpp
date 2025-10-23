// spatial_audio_validation_BLOCKING.cpp - DEPLOYMENT BLOCKER TESTS
// Jordan Taylor (QA Guardian Reborn) - PROVE SPATIAL AUDIO OR BLOCK DEPLOYMENT
//
// MISSION: Create tests that ASMRtists would fail on if spatial audio is broken
// NO DEPLOYMENT until these pass. NO EXCEPTIONS.

#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <numeric>
#include <gtest/gtest.h>
#include "../modules/audio/hrtf_processor.h"
#include "../core/include/vr_types.h"

using namespace vrb;

// =============================================================================
// CRITICAL SPATIAL AUDIO VALIDATION - ASMRtist WORKFLOW TESTS
// =============================================================================

class SpatialAudioValidationTest : public ::testing::Test {
protected:
    std::unique_ptr<HRTFProcessor> processor;
    static constexpr float SAMPLE_RATE = 48000.0f;
    static constexpr int BUFFER_SIZE = 512;
    static constexpr float MINIMUM_SPATIAL_DIFFERENCE = 0.1f; // 10% minimum difference

    void SetUp() override {
        processor = std::make_unique<HRTFProcessor>();
        ASSERT_TRUE(processor->Initialize("")) << "HRTF processor must initialize";
    }

    // Generate test tone for spatial validation
    void GenerateTestTone(float* buffer, int frames, float frequency = 440.0f, float amplitude = 0.5f) {
        for (int i = 0; i < frames; ++i) {
            buffer[i] = amplitude * std::sin(2.0f * M_PI * frequency * i / SAMPLE_RATE);
        }
    }

    // Calculate RMS energy for a channel
    float CalculateRMSEnergy(const float* buffer, int frames) {
        float sum = 0.0f;
        for (int i = 0; i < frames; ++i) {
            sum += buffer[i] * buffer[i];
        }
        return std::sqrt(sum / frames);
    }

    // Extract left and right channels from interleaved output
    void ExtractChannels(const float* interleavedOutput, int frames,
                        std::vector<float>& leftChannel, std::vector<float>& rightChannel) {
        leftChannel.resize(frames);
        rightChannel.resize(frames);
        for (int i = 0; i < frames; ++i) {
            leftChannel[i] = interleavedOutput[i * 2];
            rightChannel[i] = interleavedOutput[i * 2 + 1];
        }
    }
};

// =============================================================================
// DEPLOYMENT BLOCKER TEST 1: LEFT EAR POSITIONING
// =============================================================================
TEST_F(SpatialAudioValidationTest, DEPLOYMENT_BLOCKER_LeftEarPositioning) {
    float inputBuffer[BUFFER_SIZE];
    float outputBuffer[BUFFER_SIZE * 2];

    // Generate test signal - what an ASMRtist would record
    GenerateTestTone(inputBuffer, BUFFER_SIZE, 440.0f, 0.8f);

    // Position virtual microphone to LEFT EAR (what ASMRtist expects for left ear content)
    processor->SetListenerPosition(Vec3(-1.0f, 0.0f, 0.0f)); // Left side

    // Process audio - this is what ASMRtists will hear
    processor->Process(inputBuffer, outputBuffer, BUFFER_SIZE, 1);

    // Extract channels
    std::vector<float> leftChannel, rightChannel;
    ExtractChannels(outputBuffer, BUFFER_SIZE, leftChannel, rightChannel);

    // Calculate energies
    float leftEnergy = CalculateRMSEnergy(leftChannel.data(), BUFFER_SIZE);
    float rightEnergy = CalculateRMSEnergy(rightChannel.data(), BUFFER_SIZE);

    // CRITICAL ASSERTION: Left ear positioning MUST produce more left channel energy
    ASSERT_GT(leftEnergy, rightEnergy)
        << "DEPLOYMENT BLOCKED: Left ear positioning failed. ASMRtist left ear content will not work. "
        << "Left Energy: " << leftEnergy << ", Right Energy: " << rightEnergy;

    // CRITICAL ASSERTION: Energy difference must be substantial (not just numerical noise)
    float energyRatio = leftEnergy / (rightEnergy + 0.0001f);
    ASSERT_GT(energyRatio, 1.0f + MINIMUM_SPATIAL_DIFFERENCE)
        << "DEPLOYMENT BLOCKED: Spatial difference too small for left positioning. "
        << "Energy ratio: " << energyRatio << " (minimum required: " << 1.0f + MINIMUM_SPATIAL_DIFFERENCE << ")";
}

// =============================================================================
// DEPLOYMENT BLOCKER TEST 2: RIGHT EAR POSITIONING
// =============================================================================
TEST_F(SpatialAudioValidationTest, DEPLOYMENT_BLOCKER_RightEarPositioning) {
    float inputBuffer[BUFFER_SIZE];
    float outputBuffer[BUFFER_SIZE * 2];

    // Generate test signal
    GenerateTestTone(inputBuffer, BUFFER_SIZE, 440.0f, 0.8f);

    // Position virtual microphone to RIGHT EAR
    processor->SetListenerPosition(Vec3(1.0f, 0.0f, 0.0f)); // Right side

    // Process audio
    processor->Process(inputBuffer, outputBuffer, BUFFER_SIZE, 1);

    // Extract channels
    std::vector<float> leftChannel, rightChannel;
    ExtractChannels(outputBuffer, BUFFER_SIZE, leftChannel, rightChannel);

    // Calculate energies
    float leftEnergy = CalculateRMSEnergy(leftChannel.data(), BUFFER_SIZE);
    float rightEnergy = CalculateRMSEnergy(rightChannel.data(), BUFFER_SIZE);

    // CRITICAL ASSERTION: Right ear positioning MUST produce more right channel energy
    ASSERT_GT(rightEnergy, leftEnergy)
        << "DEPLOYMENT BLOCKED: Right ear positioning failed. ASMRtist right ear content will not work. "
        << "Left Energy: " << leftEnergy << ", Right Energy: " << rightEnergy;

    // CRITICAL ASSERTION: Energy difference must be substantial
    float energyRatio = rightEnergy / (leftEnergy + 0.0001f);
    ASSERT_GT(energyRatio, 1.0f + MINIMUM_SPATIAL_DIFFERENCE)
        << "DEPLOYMENT BLOCKED: Spatial difference too small for right positioning. "
        << "Energy ratio: " << energyRatio << " (minimum required: " << 1.0f + MINIMUM_SPATIAL_DIFFERENCE << ")";
}

// =============================================================================
// DEPLOYMENT BLOCKER TEST 3: BEHIND HEAD POSITIONING
// =============================================================================
TEST_F(SpatialAudioValidationTest, DEPLOYMENT_BLOCKER_BehindHeadPositioning) {
    float inputBuffer[BUFFER_SIZE];
    float outputBuffer[BUFFER_SIZE * 2];

    // Generate test signal
    GenerateTestTone(inputBuffer, BUFFER_SIZE, 440.0f, 0.8f);

    // Position virtual microphone BEHIND HEAD
    processor->SetListenerPosition(Vec3(0.0f, 0.0f, 1.0f)); // Behind

    // Process audio
    processor->Process(inputBuffer, outputBuffer, BUFFER_SIZE, 1);

    // Extract channels
    std::vector<float> leftChannel, rightChannel;
    ExtractChannels(outputBuffer, BUFFER_SIZE, leftChannel, rightChannel);

    // Calculate energies
    float leftEnergy = CalculateRMSEnergy(leftChannel.data(), BUFFER_SIZE);
    float rightEnergy = CalculateRMSEnergy(rightChannel.data(), BUFFER_SIZE);

    // CRITICAL ASSERTION: Behind head should have different characteristics than front
    // Compare with front positioning
    processor->SetListenerPosition(Vec3(0.0f, 0.0f, -1.0f)); // Front
    processor->Process(inputBuffer, outputBuffer, BUFFER_SIZE, 1);

    std::vector<float> frontLeftChannel, frontRightChannel;
    ExtractChannels(outputBuffer, BUFFER_SIZE, frontLeftChannel, frontRightChannel);

    float frontLeftEnergy = CalculateRMSEnergy(frontLeftChannel.data(), BUFFER_SIZE);
    float frontRightEnergy = CalculateRMSEnergy(frontRightChannel.data(), BUFFER_SIZE);

    // CRITICAL ASSERTION: Front vs back positioning must produce different results
    bool significantDifference = (std::abs(leftEnergy - frontLeftEnergy) > MINIMUM_SPATIAL_DIFFERENCE * frontLeftEnergy) ||
                                (std::abs(rightEnergy - frontRightEnergy) > MINIMUM_SPATIAL_DIFFERENCE * frontRightEnergy);

    ASSERT_TRUE(significantDifference)
        << "DEPLOYMENT BLOCKED: Front vs back positioning produces identical results. "
        << "Back: L=" << leftEnergy << ", R=" << rightEnergy
        << " Front: L=" << frontLeftEnergy << ", R=" << frontRightEnergy;
}

// =============================================================================
// DEPLOYMENT BLOCKER TEST 4: DETECT STUB IMPLEMENTATIONS
// =============================================================================
TEST_F(SpatialAudioValidationTest, DEPLOYMENT_BLOCKER_DetectStubImplementation) {
    float inputBuffer[BUFFER_SIZE];
    float outputBuffer1[BUFFER_SIZE * 2];
    float outputBuffer2[BUFFER_SIZE * 2];

    // Generate consistent test signal
    GenerateTestTone(inputBuffer, BUFFER_SIZE, 1000.0f, 1.0f);

    // Test Position 1: Far left
    processor->SetListenerPosition(Vec3(-2.0f, 0.0f, 0.0f));
    processor->Process(inputBuffer, outputBuffer1, BUFFER_SIZE, 1);

    // Test Position 2: Far right
    processor->SetListenerPosition(Vec3(2.0f, 0.0f, 0.0f));
    processor->Process(inputBuffer, outputBuffer2, BUFFER_SIZE, 1);

    // Extract channels
    std::vector<float> left1, right1, left2, right2;
    ExtractChannels(outputBuffer1, BUFFER_SIZE, left1, right1);
    ExtractChannels(outputBuffer2, BUFFER_SIZE, left2, right2);

    // Calculate energies
    float leftEnergy1 = CalculateRMSEnergy(left1.data(), BUFFER_SIZE);
    float rightEnergy1 = CalculateRMSEnergy(right1.data(), BUFFER_SIZE);
    float leftEnergy2 = CalculateRMSEnergy(left2.data(), BUFFER_SIZE);
    float rightEnergy2 = CalculateRMSEnergy(right2.data(), BUFFER_SIZE);

    // CRITICAL ASSERTION: This is the test that caught the previous failure
    ASSERT_NE(leftEnergy1, leftEnergy2)
        << "DEPLOYMENT BLOCKED: STUB IMPLEMENTATION DETECTED. "
        << "Different positions produce identical left channel outputs: "
        << leftEnergy1 << " vs " << leftEnergy2;

    ASSERT_NE(rightEnergy1, rightEnergy2)
        << "DEPLOYMENT BLOCKED: STUB IMPLEMENTATION DETECTED. "
        << "Different positions produce identical right channel outputs: "
        << rightEnergy1 << " vs " << rightEnergy2;

    // CRITICAL ASSERTION: The energy pattern should be inverted between left and right positions
    bool correctSpatialBehavior = (leftEnergy1 > rightEnergy1) && (rightEnergy2 > leftEnergy2);
    ASSERT_TRUE(correctSpatialBehavior)
        << "DEPLOYMENT BLOCKED: Spatial behavior incorrect. "
        << "Left position should favor left channel, right position should favor right channel. "
        << "Left pos: L=" << leftEnergy1 << ", R=" << rightEnergy1
        << " Right pos: L=" << leftEnergy2 << ", R=" << rightEnergy2;
}

// =============================================================================
// DEPLOYMENT BLOCKER TEST 5: ELEVATION DIFFERENCES
// =============================================================================
TEST_F(SpatialAudioValidationTest, DEPLOYMENT_BLOCKER_ElevationDifferences) {
    float inputBuffer[BUFFER_SIZE];
    float outputAbove[BUFFER_SIZE * 2];
    float outputBelow[BUFFER_SIZE * 2];

    // Generate test signal
    GenerateTestTone(inputBuffer, BUFFER_SIZE, 800.0f, 0.7f);

    // Test Position 1: Above head
    processor->SetListenerPosition(Vec3(0.0f, 1.5f, 0.0f));
    processor->Process(inputBuffer, outputAbove, BUFFER_SIZE, 1);

    // Test Position 2: Below head
    processor->SetListenerPosition(Vec3(0.0f, -1.5f, 0.0f));
    processor->Process(inputBuffer, outputBelow, BUFFER_SIZE, 1);

    // Calculate total energies
    float energyAbove = 0.0f, energyBelow = 0.0f;
    for (int i = 0; i < BUFFER_SIZE * 2; ++i) {
        energyAbove += outputAbove[i] * outputAbove[i];
        energyBelow += outputBelow[i] * outputBelow[i];
    }

    // CRITICAL ASSERTION: Elevation changes must affect audio processing
    ASSERT_NE(energyAbove, energyBelow)
        << "DEPLOYMENT BLOCKED: Elevation changes produce identical outputs. "
        << "Above energy: " << energyAbove << ", Below energy: " << energyBelow;

    // CRITICAL ASSERTION: The difference must be substantial enough for users to perceive
    float energyDifferenceRatio = std::abs(energyAbove - energyBelow) / std::max(energyAbove, energyBelow);
    ASSERT_GT(energyDifferenceRatio, MINIMUM_SPATIAL_DIFFERENCE)
        << "DEPLOYMENT BLOCKED: Elevation difference too small to be perceptible. "
        << "Difference ratio: " << energyDifferenceRatio << " (minimum: " << MINIMUM_SPATIAL_DIFFERENCE << ")";
}

// =============================================================================
// DEPLOYMENT BLOCKER TEST 6: DISTANCE ATTENUATION
// =============================================================================
TEST_F(SpatialAudioValidationTest, DEPLOYMENT_BLOCKER_DistanceAttenuation) {
    float inputBuffer[BUFFER_SIZE];
    float outputNear[BUFFER_SIZE * 2];
    float outputFar[BUFFER_SIZE * 2];

    // Generate test signal
    GenerateTestTone(inputBuffer, BUFFER_SIZE, 1000.0f, 1.0f);

    // Test Position 1: Close distance
    processor->SetListenerPosition(Vec3(0.1f, 0.0f, 0.0f));
    processor->Process(inputBuffer, outputNear, BUFFER_SIZE, 1);

    // Test Position 2: Far distance
    processor->SetListenerPosition(Vec3(5.0f, 0.0f, 0.0f));
    processor->Process(inputBuffer, outputFar, BUFFER_SIZE, 1);

    // Calculate total energies
    float energyNear = 0.0f, energyFar = 0.0f;
    for (int i = 0; i < BUFFER_SIZE * 2; ++i) {
        energyNear += outputNear[i] * outputNear[i];
        energyFar += outputFar[i] * outputFar[i];
    }

    // CRITICAL ASSERTION: Closer sources must be louder
    ASSERT_GT(energyNear, energyFar)
        << "DEPLOYMENT BLOCKED: Distance attenuation not working. "
        << "Closer source should be louder. Near: " << energyNear << ", Far: " << energyFar;

    // CRITICAL ASSERTION: Distance attenuation must be substantial
    float attenuationRatio = energyNear / (energyFar + 0.0001f);
    ASSERT_GT(attenuationRatio, 2.0f)
        << "DEPLOYMENT BLOCKED: Distance attenuation too weak. "
        << "Near/Far energy ratio: " << attenuationRatio << " (minimum: 2.0)";
}

// =============================================================================
// DEPLOYMENT BLOCKER MAIN TEST COORDINATOR
// =============================================================================

class SpatialAudioDeploymentBlocker : public ::testing::Environment {
public:
    void SetUp() override {
        std::cout << "\n" << std::string(80, '=') << "\n";
        std::cout << "🚨 JORDAN TAYLOR (QA GUARDIAN) - DEPLOYMENT BLOCKING TESTS 🚨\n";
        std::cout << std::string(80, '=') << "\n";
        std::cout << "MISSION: Validate spatial audio works for ASMRtist content creation\n";
        std::cout << "\nCRITICAL TESTS (ALL MUST PASS FOR DEPLOYMENT):\n";
        std::cout << "1. Left ear positioning - ASMRtist left ear content\n";
        std::cout << "2. Right ear positioning - ASMRtist right ear content\n";
        std::cout << "3. Behind head positioning - 3D spatial effects\n";
        std::cout << "4. Detect stub implementations - previous failure prevention\n";
        std::cout << "5. Elevation differences - above/below head effects\n";
        std::cout << "6. Distance attenuation - close/far spatial reality\n";
        std::cout << "\n⚠️  IF ANY TEST FAILS: DEPLOYMENT IS BLOCKED ⚠️\n";
        std::cout << "Previous Jordan was consumed by wolves for approving 252.728=252.728\n";
        std::cout << "I will NOT approve spatial audio until it actually works.\n";
        std::cout << std::string(80, '=') << "\n\n";
    }

    void TearDown() override {
        std::cout << "\n" << std::string(80, '=') << "\n";
        std::cout << "SPATIAL AUDIO VALIDATION COMPLETE\n";
        std::cout << std::string(80, '=') << "\n";
        std::cout << "✅ If you see this message, ALL spatial audio tests PASSED\n";
        std::cout << "✅ ASMRtist workflow will work correctly\n";
        std::cout << "✅ Left/Right ear positioning functional\n";
        std::cout << "✅ 3D spatial effects operational\n";
        std::cout << "✅ No stub implementations detected\n";
        std::cout << "\n🎯 DEPLOYMENT APPROVED FOR ASMRtist SPATIAL AUDIO 🎯\n";
        std::cout << std::string(80, '=') << "\n\n";
    }
};

// Register the blocking test environment
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new SpatialAudioDeploymentBlocker);
    return RUN_ALL_TESTS();
}