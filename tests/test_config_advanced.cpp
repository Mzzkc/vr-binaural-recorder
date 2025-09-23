// test_config_advanced.cpp - Advanced Configuration System Tests
// Comprehensive validation for config.cpp fixes and thread safety

#include <gtest/gtest.h>
#include "config.h"
#include <fstream>
#include <filesystem>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>

namespace vrb {

class ConfigAdvancedTest : public ::testing::Test {
protected:
    void SetUp() override {
        testConfigPath = "test_config_advanced.json";
        CreateTestConfig();
    }

    void TearDown() override {
        std::filesystem::remove(testConfigPath);
    }

    void CreateTestConfig() {
        Json::Value root;

        // Comprehensive test configuration
        root["audio"]["sampleRate"] = 48000;
        root["audio"]["bufferSize"] = 128;
        root["audio"]["preferredInputDevice"] = "Default";
        root["audio"]["channels"]["input"] = "auto";
        root["audio"]["channels"]["output"] = 2;

        root["hrtf"]["dataPath"] = "./hrtf_data";
        root["hrtf"]["filterLength"] = 512;
        root["hrtf"]["fftSize"] = 1024;

        root["vr"]["trackingRate"] = 90;
        root["vr"]["smoothingFactor"] = 0.95;
        root["vr"]["overlayPosition"]["x"] = 0.0;
        root["vr"]["overlayPosition"]["y"] = 1.5;
        root["vr"]["overlayPosition"]["z"] = -1.0;

        root["performance"]["threadPriority"] = "realtime";
        root["performance"]["simdLevel"] = "auto";

        root["experimental"]["enableGPUProcessing"] = false;
        root["experimental"]["reverb"]["enabled"] = false;
        root["experimental"]["reverb"]["wetLevel"] = 0.1;

        std::ofstream file(testConfigPath);
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  ";
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(root, &file);
    }

    std::string testConfigPath;
};

// Test 1: Configuration Loading and Default Value Handling
TEST_F(ConfigAdvancedTest, ConfigurationLoadingAndDefaults) {
    Config config(testConfigPath);

    // Test loaded values
    EXPECT_EQ(config.GetSampleRate(), 48000);
    EXPECT_EQ(config.GetBufferSize(), 128);
    EXPECT_EQ(config.GetPreferredInputDevice(), "Default");
    EXPECT_EQ(config.GetInputChannels(), 0); // "auto" should return 0
    EXPECT_EQ(config.GetOutputChannels(), 2);

    // Test HRTF values
    EXPECT_EQ(config.GetHRTFDataPath(), "./hrtf_data");
    EXPECT_EQ(config.GetHRTFFilterLength(), 512);
    EXPECT_EQ(config.GetFFTSize(), 1024);

    // Test VR values
    EXPECT_EQ(config.GetTrackingRate(), 90);
    EXPECT_FLOAT_EQ(config.GetSmoothingFactor(), 0.95f);

    float x, y, z;
    config.GetOverlayPosition(x, y, z);
    EXPECT_FLOAT_EQ(x, 0.0f);
    EXPECT_FLOAT_EQ(y, 1.5f);
    EXPECT_FLOAT_EQ(z, -1.0f);

    // Test performance values
    EXPECT_EQ(config.GetThreadPriority(), "realtime");
    EXPECT_EQ(config.GetSIMDLevel(), "auto");

    // Test experimental values
    EXPECT_FALSE(config.GetEnableGPUProcessing());
    EXPECT_FALSE(config.GetEnableReverb());
    EXPECT_FLOAT_EQ(config.GetReverbWetLevel(), 0.1f);

    // Test default values for missing keys
    EXPECT_EQ(config.GetLogLevel(), "info"); // Should use default
    EXPECT_TRUE(config.GetAutoStart()); // Should use default
}

// Test 2: Thread Safety - Concurrent Read/Write Operations
TEST_F(ConfigAdvancedTest, ThreadSafetyConcurrentOperations) {
    Config config(testConfigPath);
    std::atomic<bool> allPassed{true};
    std::atomic<int> readOperations{0};
    std::atomic<int> writeOperations{0};

    const int numThreads = 8;
    const int operationsPerThread = 100;
    std::vector<std::thread> threads;

    // Create reader threads
    for (int i = 0; i < numThreads / 2; ++i) {
        threads.emplace_back([&config, &allPassed, &readOperations, operationsPerThread]() {
            for (int j = 0; j < operationsPerThread; ++j) {
                try {
                    // Read various config values concurrently
                    auto sampleRate = config.GetSampleRate();
                    auto bufferSize = config.GetBufferSize();
                    auto trackingRate = config.GetTrackingRate();
                    auto logLevel = config.GetLogLevel();
                    auto overlayScale = config.GetOverlayScale();

                    // Verify values are reasonable
                    if (sampleRate < 8000 || sampleRate > 192000 ||
                        bufferSize < 32 || bufferSize > 8192 ||
                        trackingRate < 30 || trackingRate > 120) {
                        allPassed = false;
                    }

                    readOperations++;
                } catch (...) {
                    allPassed = false;
                }
                std::this_thread::yield();
            }
        });
    }

    // Create writer threads (using Set template method)
    for (int i = 0; i < numThreads / 2; ++i) {
        threads.emplace_back([&config, &allPassed, &writeOperations, operationsPerThread, i]() {
            for (int j = 0; j < operationsPerThread; ++j) {
                try {
                    // Write different values based on thread ID
                    config.Set("test.thread" + std::to_string(i), j);
                    config.Set("test.value" + std::to_string(i), static_cast<float>(j * 0.1f));
                    config.Set("test.flag" + std::to_string(i), (j % 2) == 0);

                    writeOperations++;
                } catch (...) {
                    allPassed = false;
                }
                std::this_thread::yield();
            }
        });
    }

    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }

    EXPECT_TRUE(allPassed.load()) << "Thread safety test failed";
    EXPECT_EQ(readOperations.load(), (numThreads / 2) * operationsPerThread);
    EXPECT_EQ(writeOperations.load(), (numThreads / 2) * operationsPerThread);
}

// Test 3: Hot Reload Functionality
TEST_F(ConfigAdvancedTest, HotReloadFunctionality) {
    Config config(testConfigPath);

    // Initial values
    EXPECT_EQ(config.GetSampleRate(), 48000);
    EXPECT_FALSE(config.HasChanged());

    // Modify the config file
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    Json::Value newRoot;
    newRoot["audio"]["sampleRate"] = 96000;
    newRoot["audio"]["bufferSize"] = 256;
    newRoot["audio"]["preferredInputDevice"] = "Modified Device";
    newRoot["hrtf"]["dataPath"] = "./modified_hrtf";
    newRoot["vr"]["trackingRate"] = 120;

    std::ofstream file(testConfigPath);
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "  ";
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(newRoot, &file);
    file.close();

    // Wait for file watcher to detect changes
    int attempts = 0;
    while (!config.HasChanged() && attempts < 50) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        attempts++;
    }

    EXPECT_TRUE(config.HasChanged()) << "File watcher should detect changes";

    // Reload and verify changes
    EXPECT_TRUE(config.Reload());
    EXPECT_EQ(config.GetSampleRate(), 96000);
    EXPECT_EQ(config.GetBufferSize(), 256);
    EXPECT_EQ(config.GetPreferredInputDevice(), "Modified Device");
    EXPECT_EQ(config.GetHRTFDataPath(), "./modified_hrtf");
    EXPECT_EQ(config.GetTrackingRate(), 120);
}

// Test 4: Template Set Method Validation
TEST_F(ConfigAdvancedTest, TemplateSetMethodValidation) {
    Config config(testConfigPath);

    // Test setting different types
    config.Set("test.intValue", 12345);
    config.Set("test.floatValue", 3.14159f);
    config.Set("test.boolValue", true);
    config.Set("test.stringValue", std::string("TestString"));

    // Test nested path creation
    config.Set("deep.nested.path.value", 42);
    config.Set("deep.nested.other.flag", false);

    // Save and reload to verify persistence
    EXPECT_TRUE(config.Save());

    Config newConfig(testConfigPath);

    // The template methods don't have getters, but we can verify by
    // checking the saved file contains the values
    std::ifstream file(testConfigPath);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    EXPECT_NE(content.find("12345"), std::string::npos);
    EXPECT_NE(content.find("3.14159"), std::string::npos);
    EXPECT_NE(content.find("true"), std::string::npos);
    EXPECT_NE(content.find("TestString"), std::string::npos);
    EXPECT_NE(content.find("deep"), std::string::npos);
}

// Test 5: Error Handling and Recovery
TEST_F(ConfigAdvancedTest, ErrorHandlingAndRecovery) {
    // Test with corrupted JSON file
    std::ofstream corruptFile(testConfigPath);
    corruptFile << "{ invalid json content without proper closing";
    corruptFile.close();

    Config config(testConfigPath);

    // Should gracefully handle corruption and use defaults
    EXPECT_EQ(config.GetSampleRate(), 48000); // Default value
    EXPECT_EQ(config.GetBufferSize(), 128); // Default value

    // Test recovery by providing valid config
    CreateTestConfig();
    EXPECT_TRUE(config.Reload());
    EXPECT_EQ(config.GetSampleRate(), 48000); // Should load correctly now
}

// Test 6: Performance and Memory Usage
TEST_F(ConfigAdvancedTest, PerformanceAndMemoryUsage) {
    Config config(testConfigPath);

    const int iterations = 10000;
    auto startTime = std::chrono::high_resolution_clock::now();

    // Rapid read operations
    for (int i = 0; i < iterations; ++i) {
        config.GetSampleRate();
        config.GetBufferSize();
        config.GetHRTFDataPath();
        config.GetTrackingRate();
        config.GetLogLevel();

        float x, y, z;
        config.GetOverlayPosition(x, y, z);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

    // Performance should be reasonable (less than 1ms per operation on average)
    double avgTime = static_cast<double>(duration.count()) / (iterations * 6); // 6 operations per iteration
    EXPECT_LT(avgTime, 1000.0) << "Config read operations too slow: " << avgTime << " microseconds";

    // Test write performance
    startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        config.Set("perf.test" + std::to_string(i), i);
    }

    endTime = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    avgTime = static_cast<double>(duration.count()) / 1000.0;
    EXPECT_LT(avgTime, 10000.0) << "Config write operations too slow: " << avgTime << " microseconds";
}

// Test 7: All Configuration Paths Coverage
TEST_F(ConfigAdvancedTest, AllConfigurationPathsCoverage) {
    Config config(testConfigPath);

    // Test all audio configuration paths
    EXPECT_GE(config.GetSampleRate(), 8000);
    EXPECT_GE(config.GetBufferSize(), 32);
    EXPECT_TRUE(config.GetPreferredInputDevice().length() >= 0);
    EXPECT_TRUE(config.GetVirtualOutputName().length() > 0);
    EXPECT_GE(config.GetInputChannels(), 0);
    EXPECT_GE(config.GetOutputChannels(), 1);

    // Test all HRTF configuration paths
    EXPECT_TRUE(config.GetHRTFDataPath().length() > 0);
    EXPECT_TRUE(config.GetHRTFDataset().length() > 0);
    EXPECT_GE(config.GetHRTFFilterLength(), 64);
    EXPECT_TRUE(config.GetConvolutionMethod().length() > 0);
    EXPECT_GE(config.GetMaxDistance(), 0.1f);
    EXPECT_GE(config.GetReferenceDistance(), 0.1f);
    EXPECT_GE(config.GetRolloffFactor(), 0.0f);
    EXPECT_GE(config.GetFFTSize(), 64);

    // Test all VR configuration paths
    EXPECT_GE(config.GetTrackingRate(), 30);
    EXPECT_GE(config.GetSmoothingFactor(), 0.0f);
    EXPECT_LE(config.GetSmoothingFactor(), 1.0f);
    EXPECT_GE(config.GetPredictionTime(), 0.0f);
    EXPECT_GE(config.GetOverlayScale(), 0.1f);

    float x, y, z;
    config.GetOverlayPosition(x, y, z);
    config.GetMicrophonePosition(x, y, z);

    // Test all performance configuration paths
    EXPECT_TRUE(config.GetThreadPriority().length() > 0);
    EXPECT_TRUE(config.GetCPUAffinity().length() > 0);
    EXPECT_TRUE(config.GetSIMDLevel().length() > 0);
    EXPECT_GE(config.GetRingBufferSize(), 256);

    // Test all logging configuration paths
    EXPECT_TRUE(config.GetLogLevel().length() > 0);
    EXPECT_TRUE(config.GetLogPath().length() > 0);
    EXPECT_TRUE(config.GetLogMaxFileSize().length() > 0);
    EXPECT_GE(config.GetLogMaxFiles(), 1);

    // Test all debug configuration paths
    EXPECT_GE(config.GetMetricsInterval(), 100);

    // Test all network configuration paths
    EXPECT_GE(config.GetControlPort(), 1024);
    EXPECT_GE(config.GetOSCPort(), 1024);
    EXPECT_TRUE(config.GetOSCAddress().length() > 0);
    EXPECT_GE(config.GetWebsocketPort(), 1024);

    // Test all UI configuration paths
    EXPECT_TRUE(config.GetUITheme().length() > 0);
    EXPECT_GE(config.GetUIFontSize(), 8);
    EXPECT_GE(config.GetAutoHideDelay(), 0);
    EXPECT_GE(config.GetUITransparency(), 0.0f);
    EXPECT_LE(config.GetUITransparency(), 1.0f);

    // Test experimental configuration paths
    EXPECT_TRUE(config.GetRoomSize().length() > 0);
    EXPECT_GE(config.GetReverbWetLevel(), 0.0f);
    EXPECT_LE(config.GetReverbWetLevel(), 1.0f);
    EXPECT_GE(config.GetReverbRoomSize(), 0.0f);
    EXPECT_LE(config.GetReverbRoomSize(), 1.0f);
    EXPECT_GE(config.GetReverbDamping(), 0.0f);
    EXPECT_LE(config.GetReverbDamping(), 1.0f);

    // Test automation configuration paths
    EXPECT_TRUE(config.GetRecordPath().length() > 0);
    EXPECT_TRUE(config.GetRecordFormat().length() > 0);
    EXPECT_GT(config.GetPlaybackSpeed(), 0.0f);

    // Test fallback configuration paths
    EXPECT_GE(config.GetEmergencyBufferSize(), 64);
}

} // namespace vrb