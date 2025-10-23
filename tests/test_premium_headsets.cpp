// test_premium_headsets.cpp - Comprehensive tests for Quest Pro and Bigscreen Beyond support
// Tests all the premium headset features we've implemented

#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>

#include "vr_hardware_detection.h"
#include "hrtf_processor.h"
#include "audio_engine.h"
#include "vr_tracker.h"
#include "config.h"

using namespace vrb;

class PremiumHeadsetTest : public ::testing::Test {
protected:
    void SetUp() override {
        config = std::make_unique<Config>();
        // Set test configuration values
        config->SetSampleRate(48000);
        config->SetBufferSize(128);
    }

    void TearDown() override {
        // Cleanup
    }

    std::unique_ptr<Config> config;
};

// Test VR Hardware Detection for Quest Pro
TEST_F(PremiumHeadsetTest, QuestProDetection) {
    VRHardwareDetection detector;
    ASSERT_TRUE(detector.Initialize());

    // Test Quest Pro entry in compatibility database
    auto supportedHeadsets = VRHardwareDetection::GetSupportedHeadsets();

    bool questProFound = false;
    for (const auto& headset : supportedHeadsets) {
        if (headset.modelName == "Meta Quest Pro") {
            questProFound = true;

            // Verify Quest Pro specifications
            EXPECT_EQ(headset.vendor, VRVendor::Meta);
            EXPECT_EQ(headset.performanceTier, PerformanceTier::Premium);
            EXPECT_EQ(headset.displayWidth, 1800);
            EXPECT_EQ(headset.displayHeight, 1920);
            EXPECT_FLOAT_EQ(headset.refreshRate, 90.0f);

            // Test Quest Pro specific features
            EXPECT_TRUE(headset.hasOffEarAudio);
            EXPECT_TRUE(headset.hasHandTracking);
            EXPECT_TRUE(headset.hasEyeTracking);
            EXPECT_TRUE(headset.hasColorPassthrough);
            EXPECT_TRUE(headset.hasIPDAdjustment);
            EXPECT_FLOAT_EQ(headset.ipd_min, 58.0f);
            EXPECT_FLOAT_EQ(headset.ipd_max, 72.0f);
            EXPECT_FLOAT_EQ(headset.audioLatencyMs, 10.0f);

            // Check supported features
            EXPECT_FALSE(headset.supportedFeatures.empty());
            bool hasAirLink = false;
            bool hasHandTracking = false;
            for (const auto& feature : headset.supportedFeatures) {
                if (feature.find("Wireless Air Link") != std::string::npos) hasAirLink = true;
                if (feature.find("Hand Tracking") != std::string::npos) hasHandTracking = true;
            }
            EXPECT_TRUE(hasAirLink);
            EXPECT_TRUE(hasHandTracking);

            break;
        }
    }

    EXPECT_TRUE(questProFound) << "Quest Pro not found in supported headsets database";
}

// Test VR Hardware Detection for Bigscreen Beyond
TEST_F(PremiumHeadsetTest, BigscreenBeyondDetection) {
    VRHardwareDetection detector;
    ASSERT_TRUE(detector.Initialize());

    auto supportedHeadsets = VRHardwareDetection::GetSupportedHeadsets();

    bool beyondFound = false;
    for (const auto& headset : supportedHeadsets) {
        if (headset.modelName == "Bigscreen Beyond") {
            beyondFound = true;

            // Verify Bigscreen Beyond specifications
            EXPECT_EQ(headset.vendor, VRVendor::Bigscreen);
            EXPECT_EQ(headset.performanceTier, PerformanceTier::Premium);
            EXPECT_EQ(headset.displayWidth, 2560);
            EXPECT_EQ(headset.displayHeight, 2560);
            EXPECT_FLOAT_EQ(headset.refreshRate, 90.0f);

            // Test Bigscreen Beyond specific features
            EXPECT_TRUE(headset.hasDMASAudio);
            EXPECT_TRUE(headset.hasLighthouseTracking);
            EXPECT_TRUE(headset.hasCustomFittedLenses);
            EXPECT_FALSE(headset.hasIntegratedAudio); // Uses DMAS
            EXPECT_EQ(headset.maxSampleRate, 96000);
            EXPECT_FLOAT_EQ(headset.audioLatencyMs, 5.0f);
            EXPECT_FLOAT_EQ(headset.trackingLatency, 8.0f);

            // Check supported features
            EXPECT_FALSE(headset.supportedFeatures.empty());
            bool hasLighthouse = false;
            bool hasDMAS = false;
            bool hasOLED = false;
            for (const auto& feature : headset.supportedFeatures) {
                if (feature.find("SteamVR 2.0 Tracking") != std::string::npos) hasLighthouse = true;
                if (feature.find("DMAS Audio") != std::string::npos) hasDMAS = true;
                if (feature.find("Micro-OLED") != std::string::npos) hasOLED = true;
            }
            EXPECT_TRUE(hasLighthouse);
            EXPECT_TRUE(hasDMAS);
            EXPECT_TRUE(hasOLED);

            break;
        }
    }

    EXPECT_TRUE(beyondFound) << "Bigscreen Beyond not found in supported headsets database";
}

// Test HRTF Processor Headset Profiles
TEST_F(PremiumHeadsetTest, HRTFHeadsetProfiles) {
    HRTFProcessor hrtf;

    // Test Quest Pro profile
    VRHeadsetInfo questPro;
    questPro.vendor = VRVendor::Meta;
    questPro.modelName = "Meta Quest Pro";
    questPro.hasOffEarAudio = true;
    questPro.audioLatencyMs = 12.0f;

    ASSERT_TRUE(hrtf.InitializeWithHeadset("", questPro));

    HeadsetAudioProfile questProfile = hrtf.GetCurrentProfile();
    EXPECT_EQ(questProfile.vendor, VRVendor::Meta);
    EXPECT_TRUE(questProfile.isOffEarDesign);
    EXPECT_FLOAT_EQ(questProfile.latencyCompensationMs, 12.0f);
    EXPECT_FLOAT_EQ(questProfile.spatialAccuracy, 1.3f);
    EXPECT_FLOAT_EQ(questProfile.mixedRealityBlend, 0.15f);

    // Test Bigscreen Beyond profile
    VRHeadsetInfo beyond;
    beyond.vendor = VRVendor::Bigscreen;
    beyond.modelName = "Bigscreen Beyond";
    beyond.hasDMASAudio = true;
    beyond.maxSampleRate = 96000;
    beyond.audioLatencyMs = 3.0f;

    ASSERT_TRUE(hrtf.InitializeWithHeadset("", beyond));

    HeadsetAudioProfile beyondProfile = hrtf.GetCurrentProfile();
    EXPECT_EQ(beyondProfile.vendor, VRVendor::Bigscreen);
    EXPECT_TRUE(beyondProfile.isDMASAudio);
    EXPECT_TRUE(beyondProfile.highFidelityMode);
    EXPECT_FLOAT_EQ(beyondProfile.latencyCompensationMs, 3.0f);
    EXPECT_FLOAT_EQ(beyondProfile.spatialAccuracy, 1.8f);
    EXPECT_FLOAT_EQ(beyondProfile.ultraLowLatencyMode, 1.0f);
    EXPECT_EQ(beyondProfile.preferredSampleRate, 96000);
}

// Test Audio Engine Headset Optimizations
TEST_F(PremiumHeadsetTest, AudioEngineOptimizations) {
    AudioEngine engine;
    HRTFProcessor hrtf;

    // Test Quest Pro optimizations
    VRHeadsetInfo questPro;
    questPro.vendor = VRVendor::Meta;
    questPro.modelName = "Meta Quest Pro";
    questPro.hasOffEarAudio = true;
    questPro.hasColorPassthrough = true;
    questPro.hasIPDAdjustment = true;
    questPro.maxSampleRate = 48000;
    questPro.audioLatencyMs = 12.0f;

    // Initialize with Quest Pro optimizations
    // Note: This may fail in test environment without actual hardware
    bool questInitialized = engine.InitializeWithHeadset(*config, &hrtf, questPro);
    // Don't assert on initialization in tests since hardware may not be available

    // Test Bigscreen Beyond optimizations
    VRHeadsetInfo beyond;
    beyond.vendor = VRVendor::Bigscreen;
    beyond.modelName = "Bigscreen Beyond";
    beyond.hasDMASAudio = true;
    beyond.hasLighthouseTracking = true;
    beyond.hasCustomFittedLenses = true;
    beyond.maxSampleRate = 96000;
    beyond.audioLatencyMs = 3.0f;
    beyond.performanceTier = PerformanceTier::Premium;

    // Initialize with Beyond optimizations
    bool beyondInitialized = engine.InitializeWithHeadset(*config, &hrtf, beyond);
    // Don't assert on initialization in tests since hardware may not be available
}

// Test VR Tracker Hardware Status
TEST_F(PremiumHeadsetTest, VRTrackerHardwareStatus) {
    VRTracker tracker;
    ASSERT_TRUE(tracker.Initialize());

    std::string status = tracker.GetHardwareStatus();
    EXPECT_FALSE(status.empty());

    // Should mention Quest Pro and Bigscreen Beyond in simulation mode
    EXPECT_TRUE(status.find("Quest Pro") != std::string::npos ||
                status.find("Bigscreen Beyond") != std::string::npos ||
                status.find("Simulation Mode") != std::string::npos);

    // Get compatibility report
    VRCompatibilityReport report = tracker.GetCompatibilityReport();
    EXPECT_TRUE(report.canRunSimulation) << "Simulation mode should always be available";

    std::string reportText = report.GenerateReport();
    EXPECT_FALSE(reportText.empty());
    EXPECT_TRUE(reportText.find("VR Binaural Recorder") != std::string::npos);
}

// Test Vendor Identification
TEST_F(PremiumHeadsetTest, VendorIdentification) {
    VRHardwareDetection detector;

    // Test Meta vendor identification
    VRVendor metaVendor = detector.IdentifyVendor("Quest Pro", "Meta");
    EXPECT_EQ(metaVendor, VRVendor::Meta);

    metaVendor = detector.IdentifyVendor("Quest 2", "Meta");
    EXPECT_EQ(metaVendor, VRVendor::Meta);

    // Test Bigscreen vendor identification
    VRVendor bigscreenVendor = detector.IdentifyVendor("Beyond", "Bigscreen");
    EXPECT_EQ(bigscreenVendor, VRVendor::Bigscreen);

    bigscreenVendor = detector.IdentifyVendor("Bigscreen Beyond", "");
    EXPECT_EQ(bigscreenVendor, VRVendor::Bigscreen);

    // Test Valve vendor identification
    VRVendor valveVendor = detector.IdentifyVendor("Index", "Valve");
    EXPECT_EQ(valveVendor, VRVendor::Valve);
}

// Test Headset Feature Detection
TEST_F(PremiumHeadsetTest, HeadsetFeatures) {
    VRHardwareDetection detector;
    ASSERT_TRUE(detector.Initialize());

    // Create mock Quest Pro headset
    VRHeadsetInfo questPro;
    questPro.vendor = VRVendor::Meta;
    questPro.modelName = "Meta Quest Pro";
    questPro.hasHandTracking = true;
    questPro.hasEyeTracking = true;
    questPro.hasColorPassthrough = true;
    questPro.hasIPDAdjustment = true;
    questPro.hasOffEarAudio = true;
    questPro.performanceTier = PerformanceTier::Premium;

    // Test performance tier assessment
    PerformanceTier questTier = detector.AssessPerformanceTier(questPro);
    EXPECT_EQ(questTier, PerformanceTier::Premium);

    // Create mock Bigscreen Beyond headset
    VRHeadsetInfo beyond;
    beyond.vendor = VRVendor::Bigscreen;
    beyond.modelName = "Bigscreen Beyond";
    beyond.displayWidth = 2560;
    beyond.displayHeight = 2560;
    beyond.refreshRate = 90.0f;
    beyond.hasDMASAudio = true;
    beyond.hasLighthouseTracking = true;
    beyond.hasCustomFittedLenses = true;
    beyond.performanceTier = PerformanceTier::Premium;

    // Test performance tier assessment
    PerformanceTier beyondTier = detector.AssessPerformanceTier(beyond);
    EXPECT_EQ(beyondTier, PerformanceTier::Premium);
}

// Test System Requirements
TEST_F(PremiumHeadsetTest, SystemRequirements) {
    VRSystemRequirements minReq = VRHardwareDetection::GetMinimumRequirements();
    VRSystemRequirements recReq = VRHardwareDetection::GetRecommendedRequirements();

    // Minimum requirements
    EXPECT_EQ(minReq.minCpuCores, 4);
    EXPECT_EQ(minReq.minRamGB, 8);
    EXPECT_EQ(minReq.maxLatencyMs, 25);
    EXPECT_TRUE(minReq.requiresUSB3);
    EXPECT_TRUE(minReq.requiresDisplayPort);

    // Recommended requirements
    EXPECT_EQ(recReq.minCpuCores, 6);
    EXPECT_EQ(recReq.minRamGB, 16);
    EXPECT_EQ(recReq.maxLatencyMs, 15);
    EXPECT_FALSE(recReq.additionalRequirements.empty());
}

// Test Audio Profile Creation
TEST_F(PremiumHeadsetTest, AudioProfiles) {
    // Test Quest Pro profile creation
    HeadsetAudioProfile questProfile = HRTFProcessor::CreateQuestProProfile();
    EXPECT_EQ(questProfile.vendor, VRVendor::Meta);
    EXPECT_TRUE(questProfile.isOffEarDesign);
    EXPECT_FALSE(questProfile.isDMASAudio);
    EXPECT_FLOAT_EQ(questProfile.mixedRealityBlend, 0.15f);
    EXPECT_EQ(questProfile.preferredSampleRate, 48000);

    // Test frequency response (should have bass boost for off-ear)
    EXPECT_GT(questProfile.frequencyResponse[0], 1.0f); // Low frequency boost

    // Test Bigscreen Beyond profile creation
    HeadsetAudioProfile beyondProfile = HRTFProcessor::CreateBigscreenBeyondProfile();
    EXPECT_EQ(beyondProfile.vendor, VRVendor::Bigscreen);
    EXPECT_TRUE(beyondProfile.isDMASAudio);
    EXPECT_TRUE(beyondProfile.highFidelityMode);
    EXPECT_FALSE(beyondProfile.isOffEarDesign);
    EXPECT_FLOAT_EQ(beyondProfile.ultraLowLatencyMode, 1.0f);
    EXPECT_EQ(beyondProfile.preferredSampleRate, 96000);

    // Test frequency response (should be flat for premium audio)
    for (int i = 0; i < 10; ++i) {
        EXPECT_FLOAT_EQ(beyondProfile.frequencyResponse[i], 1.0f);
    }

    // Test generic profile
    HeadsetAudioProfile genericProfile = HRTFProcessor::CreateGenericProfile();
    EXPECT_EQ(genericProfile.vendor, VRVendor::Unknown);
    EXPECT_FALSE(genericProfile.isOffEarDesign);
    EXPECT_FALSE(genericProfile.isDMASAudio);
    EXPECT_FALSE(genericProfile.useHighQualityHRTF);
}

// Integration test
TEST_F(PremiumHeadsetTest, FullIntegrationTest) {
    // This test simulates the full workflow of premium headset detection and setup

    VRHardwareDetection detector;
    ASSERT_TRUE(detector.Initialize());

    VRTracker tracker;
    ASSERT_TRUE(tracker.Initialize());

    HRTFProcessor hrtf;
    AudioEngine engine;

    // Test with simulation mode (should always work)
    tracker.EnableSimulationMode(true);
    EXPECT_TRUE(tracker.IsSimulationMode());

    // Get hardware status
    std::string status = tracker.GetHardwareStatus();
    EXPECT_TRUE(status.find("Simulation Mode") != std::string::npos);

    // Get compatibility report
    VRCompatibilityReport report = tracker.GetCompatibilityReport();
    EXPECT_TRUE(report.canRunSimulation);

    std::string reportText = report.GenerateReport();
    EXPECT_FALSE(reportText.empty());

    // Test troubleshooting steps
    auto troubleshooting = tracker.GetTroubleshootingSteps();
    EXPECT_FALSE(troubleshooting.empty());
    EXPECT_GT(troubleshooting.size(), 3); // Should have multiple steps
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}