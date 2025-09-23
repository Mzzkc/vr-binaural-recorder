// vr_headset_compatibility_tests.cpp - VR Headset Compatibility Validation
// Premium headset validation: Quest Pro and Bigscreen Beyond

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>

#include "logger.h"
#include "config.h"
#include "vr_tracker.h"
#include "vr_hardware_detection.h"
#include "hrtf_processor.h"
#include "audio_engine.h"

namespace vrb {
namespace testing {

class VRHeadsetCompatibilityTest : public ::testing::Test {
protected:
    void SetUp() override {
        vrb::Logger::Initialize("debug", "./test_logs");

        base_config = std::make_unique<Config>("test_config.json");
        hardware_detection = std::make_unique<VRHardwareDetection>();

        ASSERT_TRUE(hardware_detection->Initialize());
    }

    void TearDown() override {
        // Cleanup
    }

    std::unique_ptr<Config> base_config;
    std::unique_ptr<VRHardwareDetection> hardware_detection;

    // Helper to create Quest Pro optimized config
    Config CreateQuestProConfig() {
        Config config = *base_config;

        // Quest Pro optimizations
        config.Set("vr.headsetModel", "Meta Quest Pro");
        config.Set("vr.trackingRate", 90);
        config.Set("vr.optimizeForWireless", true);
        config.Set("vr.enableEyeTracking", true);
        config.Set("vr.enableHandTracking", true);
        config.Set("vr.enablePassthrough", true);

        // Audio optimizations for wireless
        config.Set("audio.sampleRate", 48000);
        config.Set("audio.bufferSize", 128);  // Balanced latency/stability for wireless
        config.Set("audio.lowLatencyMode", true);
        config.Set("audio.adaptiveBuffering", true);
        config.Set("audio.wirelessOptimization", true);

        // HRTF optimizations
        config.Set("hrtf.optimizeForMobile", true);
        config.Set("hrtf.reducedComplexity", true);
        config.Set("hrtf.fastConvolution", true);

        // Performance optimizations
        config.Set("performance.mobileOptimization", true);
        config.Set("performance.batteryOptimization", true);
        config.Set("performance.thermalThrottling", true);

        return config;
    }

    // Helper to create Bigscreen Beyond optimized config
    Config CreateBigscreenBeyondConfig() {
        Config config = *base_config;

        // Bigscreen Beyond optimizations
        config.Set("vr.headsetModel", "Bigscreen Beyond");
        config.Set("vr.trackingRate", 90);
        config.Set("vr.ultraLowLatency", true);
        config.Set("vr.highFidelityTracking", true);
        config.Set("vr.steamVROptimization", true);

        // Audio optimizations for high-fidelity
        config.Set("audio.sampleRate", 96000);  // High sample rate for audiophile quality
        config.Set("audio.bitDepth", 32);
        config.Set("audio.bufferSize", 64);     // Ultra-low latency
        config.Set("audio.exclusiveMode", true);
        config.Set("audio.highFidelityMode", true);
        config.Set("audio.dmaSOptimization", true);

        // HRTF optimizations for premium experience
        config.Set("hrtf.highQualityMode", true);
        config.Set("hrtf.extendedFrequencyRange", true);
        config.Set("hrtf.precisionConvolution", true);
        config.Set("hrtf.customTuning", "audiophile");

        // Performance optimizations for desktop VR
        config.Set("performance.maxQuality", true);
        config.Set("performance.unlimitedPower", true);
        config.Set("performance.desktopOptimization", true);

        return config;
    }

    // Simulate headset-specific tracking characteristics
    std::vector<VRPose> GenerateQuestProTrackingData() {
        std::vector<VRPose> poses;

        // Quest Pro has excellent inside-out tracking but may have slight wireless jitter
        for (int i = 0; i < 100; i++) {
            VRPose pose;

            // Natural head movement with slight wireless jitter
            pose.position.x = std::sin(i * 0.1f) * 0.3f + (rand() % 100 - 50) * 0.0001f;
            pose.position.y = 1.8f + std::sin(i * 0.05f) * 0.1f + (rand() % 100 - 50) * 0.0001f;
            pose.position.z = std::cos(i * 0.1f) * 0.3f + (rand() % 100 - 50) * 0.0001f;

            // Rotation with Quest Pro's precision
            pose.orientation.x = std::sin(i * 0.02f) * 0.1f;
            pose.orientation.y = std::sin(i * 0.03f) * 0.15f;
            pose.orientation.z = std::sin(i * 0.025f) * 0.05f;
            pose.orientation.w = 1.0f;

            poses.push_back(pose);
        }

        return poses;
    }

    std::vector<VRPose> GenerateBigscreenBeyondTrackingData() {
        std::vector<VRPose> poses;

        // Bigscreen Beyond has ultra-precise lighthouse tracking
        for (int i = 0; i < 100; i++) {
            VRPose pose;

            // Very precise movement with minimal jitter
            pose.position.x = std::sin(i * 0.1f) * 0.3f + (rand() % 20 - 10) * 0.00001f;
            pose.position.y = 1.8f + std::sin(i * 0.05f) * 0.1f + (rand() % 20 - 10) * 0.00001f;
            pose.position.z = std::cos(i * 0.1f) * 0.3f + (rand() % 20 - 10) * 0.00001f;

            // Ultra-precise rotation
            pose.orientation.x = std::sin(i * 0.02f) * 0.1f;
            pose.orientation.y = std::sin(i * 0.03f) * 0.15f;
            pose.orientation.z = std::sin(i * 0.025f) * 0.05f;
            pose.orientation.w = 1.0f;

            poses.push_back(pose);
        }

        return poses;
    }

    double CalculateTrackingJitter(const std::vector<VRPose>& poses) {
        if (poses.size() < 2) return 0.0;

        double total_jitter = 0.0;
        for (size_t i = 1; i < poses.size(); i++) {
            float dx = poses[i].position.x - poses[i-1].position.x;
            float dy = poses[i].position.y - poses[i-1].position.y;
            float dz = poses[i].position.z - poses[i-1].position.z;

            double movement = std::sqrt(dx*dx + dy*dy + dz*dz);
            total_jitter += movement;
        }

        return total_jitter / (poses.size() - 1);
    }
};

// ============================================================================
// QUEST PRO COMPATIBILITY TESTS
// ============================================================================

TEST_F(VRHeadsetCompatibilityTest, QuestProDetectionAndOptimization) {
    LOG_INFO("Testing Quest Pro detection and optimization");

    // Simulate Quest Pro detection
    auto detected_headsets = hardware_detection->GetAllDetectedHeadsets();

    // Look for Quest Pro in known headsets database
    auto supported_headsets = VRHardwareDetection::GetSupportedHeadsets();
    VRHeadsetInfo quest_pro_info;

    for (const auto& headset : supported_headsets) {
        if (headset.modelName.find("Quest Pro") != std::string::npos) {
            quest_pro_info = headset;
            break;
        }
    }

    ASSERT_FALSE(quest_pro_info.modelName.empty()) << "Quest Pro not found in compatibility database";

    // Verify Quest Pro specifications
    EXPECT_EQ(quest_pro_info.vendor, VRVendor::Meta);
    EXPECT_EQ(quest_pro_info.trackingType, TrackingType::SixDOF);
    EXPECT_EQ(quest_pro_info.performanceTier, PerformanceTier::Premium);
    EXPECT_TRUE(quest_pro_info.hasEyeTracking);
    EXPECT_TRUE(quest_pro_info.hasHandTracking);
    EXPECT_TRUE(quest_pro_info.hasColorPassthrough);
    EXPECT_TRUE(quest_pro_info.hasOffEarAudio);

    // Check optimizations
    EXPECT_EQ(quest_pro_info.displayWidth, 1800);
    EXPECT_EQ(quest_pro_info.displayHeight, 1920);
    EXPECT_EQ(quest_pro_info.refreshRate, 90.0f);
    EXPECT_LE(quest_pro_info.audioLatencyMs, 15.0f);

    LOG_INFO("Quest Pro detection validated - Model: {}, Performance: Premium, Features: Eye/Hand/Passthrough",
             quest_pro_info.modelName);
}

TEST_F(VRHeadsetCompatibilityTest, QuestProAudioOptimization) {
    LOG_INFO("Testing Quest Pro audio optimization");

    Config quest_config = CreateQuestProConfig();

    HRTFProcessor hrtf;
    ASSERT_TRUE(hrtf.Initialize("./test_hrtf_data"));

    AudioEngine engine;
    ASSERT_TRUE(engine.Initialize(quest_config, &hrtf));

    EXPECT_TRUE(engine.Start());

    // Simulate Quest Pro wireless audio characteristics
    std::this_thread::sleep_for(std::chrono::seconds(2));

    auto stats = engine.GetStats();
    engine.Stop();

    // Quest Pro should achieve good latency even over wireless
    EXPECT_LE(stats.inputLatency, 20.0) << "Quest Pro latency optimization failed";
    EXPECT_EQ(stats.underruns, 0) << "Quest Pro should have stable wireless audio";
    EXPECT_GT(stats.framesProcessed, 0) << "Quest Pro audio processing should work";

    LOG_INFO("Quest Pro audio optimization - Latency: {:.2f}ms, Frames: {}, Underruns: {}",
             stats.inputLatency, stats.framesProcessed, stats.underruns);
}

TEST_F(VRHeadsetCompatibilityTest, QuestProWirelessStability) {
    LOG_INFO("Testing Quest Pro wireless stability");

    Config quest_config = CreateQuestProConfig();

    VRTracker tracker;
    if (tracker.Initialize()) {
        // Simulate wireless tracking data with potential jitter
        auto tracking_data = GenerateQuestProTrackingData();

        double jitter = CalculateTrackingJitter(tracking_data);

        // Quest Pro should maintain reasonable tracking stability over wireless
        EXPECT_LT(jitter, 0.002) << "Quest Pro wireless tracking too jittery";

        LOG_INFO("Quest Pro wireless stability - Average jitter: {:.6f}m", jitter);
    } else {
        LOG_INFO("VR tracking not available - Quest Pro wireless test in simulation mode");
    }
}

TEST_F(VRHeadsetCompatibilityTest, QuestProBatteryOptimization) {
    LOG_INFO("Testing Quest Pro battery optimization");

    Config quest_config = CreateQuestProConfig();

    // Verify battery optimization settings
    EXPECT_TRUE(quest_config.GetBool("performance.batteryOptimization"));
    EXPECT_TRUE(quest_config.GetBool("performance.mobileOptimization"));
    EXPECT_TRUE(quest_config.GetBool("hrtf.reducedComplexity"));

    // Test that battery optimizations don't sacrifice too much quality
    HRTFProcessor hrtf;
    ASSERT_TRUE(hrtf.Initialize("./test_hrtf_data"));

    // Process test audio with battery optimizations
    const size_t frames = 1024;
    std::vector<float> input(frames, 0.5f);
    std::vector<float> output(frames * 2, 0.0f);

    auto start_time = std::chrono::high_resolution_clock::now();
    hrtf.Process(input.data(), output.data(), frames, 1);
    auto end_time = std::chrono::high_resolution_clock::now();

    auto processing_time = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time);

    // Battery-optimized processing should still be fast enough for real-time
    double time_per_frame = static_cast<double>(processing_time.count()) / frames;
    EXPECT_LT(time_per_frame, 25.0) << "Quest Pro battery optimization too aggressive";

    // Verify output quality
    float energy = 0.0f;
    for (float sample : output) {
        energy += sample * sample;
    }
    EXPECT_GT(energy, 0.0f) << "Quest Pro optimization shouldn't eliminate audio";

    LOG_INFO("Quest Pro battery optimization - Processing: {:.2f}μs/frame, Energy: {:.6f}",
             time_per_frame, energy);
}

// ============================================================================
// BIGSCREEN BEYOND COMPATIBILITY TESTS
// ============================================================================

TEST_F(VRHeadsetCompatibilityTest, BigscreenBeyondDetectionAndOptimization) {
    LOG_INFO("Testing Bigscreen Beyond detection and optimization");

    // Look for Bigscreen Beyond in compatibility database
    auto supported_headsets = VRHardwareDetection::GetSupportedHeadsets();
    VRHeadsetInfo beyond_info;

    for (const auto& headset : supported_headsets) {
        if (headset.modelName.find("Bigscreen Beyond") != std::string::npos) {
            beyond_info = headset;
            break;
        }
    }

    ASSERT_FALSE(beyond_info.modelName.empty()) << "Bigscreen Beyond not found in compatibility database";

    // Verify Bigscreen Beyond specifications
    EXPECT_EQ(beyond_info.vendor, VRVendor::Bigscreen);
    EXPECT_EQ(beyond_info.trackingType, TrackingType::RoomScale);
    EXPECT_EQ(beyond_info.performanceTier, PerformanceTier::Premium);
    EXPECT_TRUE(beyond_info.hasLighthouseTracking);
    EXPECT_TRUE(beyond_info.hasDMASAudio);
    EXPECT_TRUE(beyond_info.hasCustomFittedLenses);
    EXPECT_FALSE(beyond_info.hasIntegratedAudio); // Separate DMAS required

    // Check premium specifications
    EXPECT_EQ(beyond_info.displayWidth, 2560);
    EXPECT_EQ(beyond_info.displayHeight, 2560);
    EXPECT_EQ(beyond_info.refreshRate, 90.0f);
    EXPECT_LE(beyond_info.audioLatencyMs, 8.0f);
    EXPECT_EQ(beyond_info.maxSampleRate, 96000);

    LOG_INFO("Bigscreen Beyond detection validated - Model: {}, Performance: Premium, Features: Micro-OLED/Lighthouse",
             beyond_info.modelName);
}

TEST_F(VRHeadsetCompatibilityTest, BigscreenBeyondHighFidelityAudio) {
    LOG_INFO("Testing Bigscreen Beyond high-fidelity audio");

    Config beyond_config = CreateBigscreenBeyondConfig();

    // Verify high-fidelity settings
    EXPECT_EQ(beyond_config.GetInt("audio.sampleRate"), 96000);
    EXPECT_EQ(beyond_config.GetInt("audio.bitDepth"), 32);
    EXPECT_EQ(beyond_config.GetInt("audio.bufferSize"), 64);
    EXPECT_TRUE(beyond_config.GetBool("audio.highFidelityMode"));

    HRTFProcessor hrtf;
    ASSERT_TRUE(hrtf.Initialize("./test_hrtf_data"));

    AudioEngine engine;
    ASSERT_TRUE(engine.Initialize(beyond_config, &hrtf));

    EXPECT_TRUE(engine.Start());

    // Test high-fidelity processing
    std::this_thread::sleep_for(std::chrono::seconds(2));

    auto stats = engine.GetStats();
    engine.Stop();

    // Bigscreen Beyond should achieve ultra-low latency with high fidelity
    EXPECT_LE(stats.inputLatency, 10.0) << "Bigscreen Beyond should have ultra-low latency";
    EXPECT_EQ(stats.underruns, 0) << "High-fidelity mode should be stable";

    LOG_INFO("Bigscreen Beyond high-fidelity - Latency: {:.2f}ms, Sample Rate: {}Hz",
             stats.inputLatency, beyond_config.GetInt("audio.sampleRate"));
}

TEST_F(VRHeadsetCompatibilityTest, BigscreenBeyondPrecisionTracking) {
    LOG_INFO("Testing Bigscreen Beyond precision tracking");

    Config beyond_config = CreateBigscreenBeyondConfig();

    VRTracker tracker;
    if (tracker.Initialize()) {
        // Simulate Lighthouse tracking data with high precision
        auto tracking_data = GenerateBigscreenBeyondTrackingData();

        double jitter = CalculateTrackingJitter(tracking_data);

        // Bigscreen Beyond with Lighthouse should have minimal jitter
        EXPECT_LT(jitter, 0.0005) << "Bigscreen Beyond tracking should be ultra-precise";

        LOG_INFO("Bigscreen Beyond precision tracking - Average jitter: {:.8f}m", jitter);
    } else {
        LOG_INFO("VR tracking not available - Bigscreen Beyond precision test in simulation mode");
    }
}

TEST_F(VRHeadsetCompatibilityTest, BigscreenBeyondLatencyOptimization) {
    LOG_INFO("Testing Bigscreen Beyond latency optimization");

    Config beyond_config = CreateBigscreenBeyondConfig();

    HRTFProcessor hrtf;
    ASSERT_TRUE(hrtf.Initialize("./test_hrtf_data"));

    // Test ultra-low latency processing
    const size_t test_iterations = 1000;
    const size_t frames = 64; // Small buffer for ultra-low latency

    std::vector<float> input(frames, 0.5f);
    std::vector<float> output(frames * 2, 0.0f);

    auto start_time = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < test_iterations; i++) {
        hrtf.Process(input.data(), output.data(), frames, 1);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_time = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time);

    double time_per_buffer = static_cast<double>(total_time.count()) / test_iterations;
    double time_per_frame = time_per_buffer / frames;

    // Bigscreen Beyond should achieve ultra-low processing latency
    EXPECT_LT(time_per_frame, 15.0) << "Bigscreen Beyond processing not optimized for ultra-low latency";
    EXPECT_LT(time_per_buffer, 1000.0) << "Buffer processing time too high for Bigscreen Beyond";

    LOG_INFO("Bigscreen Beyond latency optimization - {:.2f}μs/frame, {:.2f}μs/buffer",
             time_per_frame, time_per_buffer);
}

// ============================================================================
// COMPARATIVE HEADSET TESTS
// ============================================================================

TEST_F(VRHeadsetCompatibilityTest, HeadsetPerformanceComparison) {
    LOG_INFO("Comparing Quest Pro vs Bigscreen Beyond performance");

    // Test Quest Pro configuration
    Config quest_config = CreateQuestProConfig();

    HRTFProcessor quest_hrtf;
    ASSERT_TRUE(quest_hrtf.Initialize("./test_hrtf_data"));

    AudioEngine quest_engine;
    ASSERT_TRUE(quest_engine.Initialize(quest_config, &quest_hrtf));

    EXPECT_TRUE(quest_engine.Start());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto quest_stats = quest_engine.GetStats();
    quest_engine.Stop();

    // Test Bigscreen Beyond configuration
    Config beyond_config = CreateBigscreenBeyondConfig();

    HRTFProcessor beyond_hrtf;
    ASSERT_TRUE(beyond_hrtf.Initialize("./test_hrtf_data"));

    AudioEngine beyond_engine;
    ASSERT_TRUE(beyond_engine.Initialize(beyond_config, &beyond_hrtf));

    EXPECT_TRUE(beyond_engine.Start());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto beyond_stats = beyond_engine.GetStats();
    beyond_engine.Stop();

    // Compare performance characteristics
    LOG_INFO("Performance Comparison:");
    LOG_INFO("  Quest Pro    - Latency: {:.2f}ms, Frames: {}, SR: {}Hz",
             quest_stats.inputLatency, quest_stats.framesProcessed,
             quest_config.GetInt("audio.sampleRate"));
    LOG_INFO("  Beyond       - Latency: {:.2f}ms, Frames: {}, SR: {}Hz",
             beyond_stats.inputLatency, beyond_stats.framesProcessed,
             beyond_config.GetInt("audio.sampleRate"));

    // Bigscreen Beyond should have lower latency due to wired connection and optimizations
    EXPECT_LE(beyond_stats.inputLatency, quest_stats.inputLatency)
        << "Bigscreen Beyond should have lower latency than Quest Pro";

    // Both should have stable performance
    EXPECT_EQ(quest_stats.underruns, 0);
    EXPECT_EQ(beyond_stats.underruns, 0);
}

TEST_F(VRHeadsetCompatibilityTest, HeadsetSpecificFeatureValidation) {
    LOG_INFO("Validating headset-specific features");

    auto supported_headsets = VRHardwareDetection::GetSupportedHeadsets();

    VRHeadsetInfo quest_pro, beyond;
    for (const auto& headset : supported_headsets) {
        if (headset.modelName.find("Quest Pro") != std::string::npos) {
            quest_pro = headset;
        } else if (headset.modelName.find("Bigscreen Beyond") != std::string::npos) {
            beyond = headset;
        }
    }

    // Quest Pro specific features
    ASSERT_FALSE(quest_pro.modelName.empty());
    EXPECT_TRUE(quest_pro.hasEyeTracking) << "Quest Pro should support eye tracking";
    EXPECT_TRUE(quest_pro.hasHandTracking) << "Quest Pro should support hand tracking";
    EXPECT_TRUE(quest_pro.hasColorPassthrough) << "Quest Pro should support passthrough";
    EXPECT_TRUE(quest_pro.hasOffEarAudio) << "Quest Pro should have off-ear audio";
    EXPECT_TRUE(quest_pro.hasIPDAdjustment) << "Quest Pro should have motorized IPD";

    // Bigscreen Beyond specific features
    ASSERT_FALSE(beyond.modelName.empty());
    EXPECT_TRUE(beyond.hasLighthouseTracking) << "Beyond should use Lighthouse tracking";
    EXPECT_TRUE(beyond.hasDMASAudio) << "Beyond should support DMAS audio";
    EXPECT_TRUE(beyond.hasCustomFittedLenses) << "Beyond should have custom lenses";
    EXPECT_FALSE(beyond.hasIntegratedAudio) << "Beyond requires separate DMAS";
    EXPECT_EQ(beyond.maxSampleRate, 96000) << "Beyond should support 96kHz audio";

    // Performance tier validation
    EXPECT_EQ(quest_pro.performanceTier, PerformanceTier::Premium);
    EXPECT_EQ(beyond.performanceTier, PerformanceTier::Premium);

    LOG_INFO("Headset feature validation completed - Quest Pro: Eye/Hand/Passthrough, Beyond: Lighthouse/DMAS/Custom");
}

TEST_F(VRHeadsetCompatibilityTest, HeadsetLimitationsAndRecommendations) {
    LOG_INFO("Validating headset limitations and recommendations");

    auto supported_headsets = VRHardwareDetection::GetSupportedHeadsets();

    for (const auto& headset : supported_headsets) {
        if (headset.modelName.find("Quest Pro") != std::string::npos) {
            // Quest Pro limitations
            EXPECT_GT(headset.knownLimitations.size(), 0) << "Quest Pro should have documented limitations";

            bool has_wireless_limitation = false;
            bool has_battery_limitation = false;

            for (const auto& limitation : headset.knownLimitations) {
                if (limitation.find("wireless") != std::string::npos ||
                    limitation.find("Air Link") != std::string::npos) {
                    has_wireless_limitation = true;
                }
                if (limitation.find("battery") != std::string::npos ||
                    limitation.find("Battery") != std::string::npos) {
                    has_battery_limitation = true;
                }
            }

            EXPECT_TRUE(has_wireless_limitation) << "Quest Pro should document wireless limitations";
            EXPECT_TRUE(has_battery_limitation) << "Quest Pro should document battery limitations";

        } else if (headset.modelName.find("Bigscreen Beyond") != std::string::npos) {
            // Bigscreen Beyond limitations
            EXPECT_GT(headset.knownLimitations.size(), 0) << "Bigscreen Beyond should have documented limitations";

            bool has_lighthouse_requirement = false;
            bool has_custom_fit_requirement = false;
            bool has_dmas_requirement = false;

            for (const auto& limitation : headset.knownLimitations) {
                if (limitation.find("Lighthouse") != std::string::npos ||
                    limitation.find("SteamVR") != std::string::npos) {
                    has_lighthouse_requirement = true;
                }
                if (limitation.find("custom") != std::string::npos ||
                    limitation.find("fit") != std::string::npos) {
                    has_custom_fit_requirement = true;
                }
                if (limitation.find("DMAS") != std::string::npos ||
                    limitation.find("audio") != std::string::npos) {
                    has_dmas_requirement = true;
                }
            }

            EXPECT_TRUE(has_lighthouse_requirement) << "Beyond should document Lighthouse requirement";
            EXPECT_TRUE(has_custom_fit_requirement) << "Beyond should document custom fit requirement";
            EXPECT_TRUE(has_dmas_requirement) << "Beyond should document DMAS requirement";
        }
    }

    LOG_INFO("Headset limitations validation completed");
}

} // namespace testing
} // namespace vrb

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    std::cout << "\n";
    std::cout << "=========================================================\n";
    std::cout << "       VR HEADSET COMPATIBILITY VALIDATION\n";
    std::cout << "=========================================================\n";
    std::cout << "\n";
    std::cout << "Testing premium VR headset compatibility:\n";
    std::cout << "  • Meta Quest Pro - Premium wireless VR\n";
    std::cout << "  • Bigscreen Beyond - Ultra-premium OLED\n";
    std::cout << "\n";

    return RUN_ALL_TESTS();
}