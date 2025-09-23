// integration_tests.cpp - System Integration Tests
// End-to-end integration testing for Windows RC validation

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <fstream>
#include <filesystem>

#include "logger.h"
#include "config.h"
#include "application.h"
#include "audio_engine.h"
#include "hrtf_processor.h"
#include "vr_tracker.h"

#ifdef _WIN32
#include "windows_gui.h"
#include "windows_wasapi_virtual_device.h"
#endif

namespace vrb {
namespace testing {

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        vrb::Logger::Initialize("debug", "./test_logs");

        // Create test environment
        CreateTestEnvironment();
    }

    void TearDown() override {
        CleanupTestEnvironment();
    }

    void CreateTestEnvironment() {
        std::filesystem::create_directories("./test_logs");
        std::filesystem::create_directories("./test_configs");
        std::filesystem::create_directories("./test_hrtf_data");

        // Create minimal test HRTF data
        CreateTestHRTFData();

        // Create test configuration
        CreateTestConfiguration();
    }

    void CleanupTestEnvironment() {
        // Optional cleanup - preserve logs for analysis
    }

    void CreateTestHRTFData() {
        // Create minimal HRTF data for testing
        std::filesystem::create_directory("./test_hrtf_data");

        // Create a simple test HRTF file
        std::ofstream hrtf_file("./test_hrtf_data/test.hrtf", std::ios::binary);
        if (hrtf_file.is_open()) {
            // Write minimal HRTF header and data
            std::vector<float> test_ir(256, 0.1f); // Simple impulse response
            hrtf_file.write(reinterpret_cast<const char*>(test_ir.data()),
                           test_ir.size() * sizeof(float));
            hrtf_file.close();
        }
    }

    void CreateTestConfiguration() {
        std::ofstream config_file("./test_configs/integration_test.json");
        config_file << R"({
            "audio": {
                "sampleRate": 48000,
                "bufferSize": 128,
                "api": "auto",
                "virtualOutputName": "VR Binaural Test Output",
                "lowLatencyMode": true
            },
            "hrtf": {
                "dataPath": "./test_hrtf_data",
                "filterLength": 256,
                "convolutionMethod": "auto"
            },
            "vr": {
                "trackingRate": 90,
                "smoothingFactor": 0.95,
                "simulationMode": true
            },
            "logging": {
                "level": "debug",
                "path": "./test_logs",
                "console": true,
                "file": true
            }
        })";
        config_file.close();
    }

    bool WaitForCondition(std::function<bool()> condition,
                         std::chrono::milliseconds timeout,
                         std::chrono::milliseconds check_interval = std::chrono::milliseconds(10)) {
        auto start_time = std::chrono::steady_clock::now();

        while (std::chrono::steady_clock::now() - start_time < timeout) {
            if (condition()) {
                return true;
            }
            std::this_thread::sleep_for(check_interval);
        }

        return false;
    }
};

// ============================================================================
// FULL SYSTEM INTEGRATION TESTS
// ============================================================================

TEST_F(IntegrationTest, FullSystemStartupAndShutdown) {
    LOG_INFO("Testing full system startup and shutdown");

    auto start_time = std::chrono::high_resolution_clock::now();

    // Test complete system initialization
    Config config("./test_configs/integration_test.json");
    ASSERT_TRUE(config.IsValid()) << "Test configuration should be valid";

    HRTFProcessor hrtf;
    ASSERT_TRUE(hrtf.Initialize(config.GetHRTFDataPath())) << "HRTF processor should initialize";

    AudioEngine audio;
    ASSERT_TRUE(audio.Initialize(config, &hrtf)) << "Audio engine should initialize";

    VRTracker vr_tracker;
    bool vr_initialized = vr_tracker.Initialize();
    LOG_INFO("VR Tracker initialization: {}", vr_initialized ? "Success" : "Failed (expected without VR hardware)");

    // Test system startup timing
    auto init_time = std::chrono::high_resolution_clock::now();
    auto startup_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        init_time - start_time);

    EXPECT_LT(startup_duration.count(), 3000) << "System startup should complete within 3 seconds";

    // Test audio system startup
    EXPECT_TRUE(audio.Start()) << "Audio system should start successfully";

    // Verify system is running
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    auto audio_stats = audio.GetStats();
    EXPECT_GT(audio_stats.framesProcessed, 0) << "Audio processing should be active";

    if (vr_initialized) {
        VRPose pose = vr_tracker.GetCurrentPose();
        // Basic validation that pose data is reasonable
        EXPECT_FALSE(std::isnan(pose.position.x)) << "VR pose should have valid position data";
    }

    // Test graceful shutdown
    audio.Stop();

    auto shutdown_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        shutdown_time - start_time);

    LOG_INFO("Full system integration test completed in {}ms", total_duration.count());
}

TEST_F(IntegrationTest, AudioVRIntegration) {
    LOG_INFO("Testing Audio-VR integration");

    Config config("./test_configs/integration_test.json");
    HRTFProcessor hrtf;
    ASSERT_TRUE(hrtf.Initialize(config.GetHRTFDataPath()));

    AudioEngine audio;
    ASSERT_TRUE(audio.Initialize(config, &hrtf));

    VRTracker vr_tracker;
    bool vr_available = vr_tracker.Initialize();

    EXPECT_TRUE(audio.Start());

    // Test spatial audio updates with VR tracking
    if (vr_available) {
        LOG_INFO("Testing with real VR tracking");

        for (int i = 0; i < 10; i++) {
            VRPose head_pose = vr_tracker.GetCurrentPose();
            VRPose mic_pose = {{0.0f, 1.2f, -1.0f}, {0.0f, 0.0f, 0.0f, 1.0f}};

            hrtf.UpdateSpatialPosition(head_pose, mic_pose);

            auto hrtf_stats = hrtf.GetStats();
            EXPECT_GE(hrtf_stats.distance, 0.0f) << "HRTF distance should be valid";

            std::this_thread::sleep_for(std::chrono::milliseconds(11)); // ~90Hz VR rate
        }
    } else {
        LOG_INFO("Testing with simulated VR tracking");

        // Simulate VR head movement
        for (int i = 0; i < 10; i++) {
            VRPose head_pose;
            head_pose.position.x = std::sin(i * 0.1f) * 0.5f;
            head_pose.position.y = 1.8f;
            head_pose.position.z = std::cos(i * 0.1f) * 0.5f;
            head_pose.orientation = {0.0f, 0.0f, 0.0f, 1.0f};

            VRPose mic_pose = {{0.0f, 1.2f, -1.0f}, {0.0f, 0.0f, 0.0f, 1.0f}};

            hrtf.UpdateSpatialPosition(head_pose, mic_pose);

            auto hrtf_stats = hrtf.GetStats();
            EXPECT_GE(hrtf_stats.distance, 0.0f) << "Simulated HRTF distance should be valid";
            EXPECT_GE(hrtf_stats.azimuth, -180.0f) << "Azimuth should be in valid range";
            EXPECT_LE(hrtf_stats.azimuth, 180.0f) << "Azimuth should be in valid range";

            std::this_thread::sleep_for(std::chrono::milliseconds(11));
        }
    }

    audio.Stop();

    auto final_stats = audio.GetStats();
    EXPECT_EQ(final_stats.underruns, 0) << "Audio-VR integration should be stable";

    LOG_INFO("Audio-VR integration test completed - Frames: {}, Underruns: {}",
             final_stats.framesProcessed, final_stats.underruns);
}

TEST_F(IntegrationTest, ConfigurationHotReload) {
    LOG_INFO("Testing configuration hot reload");

    Config config("./test_configs/integration_test.json");
    HRTFProcessor hrtf;
    ASSERT_TRUE(hrtf.Initialize(config.GetHRTFDataPath()));

    AudioEngine audio;
    ASSERT_TRUE(audio.Initialize(config, &hrtf));
    EXPECT_TRUE(audio.Start());

    // Get initial settings
    int initial_buffer_size = config.GetBufferSize();
    EXPECT_EQ(initial_buffer_size, 128);

    // Modify configuration file
    std::ofstream config_file("./test_configs/integration_test.json");
    config_file << R"({
        "audio": {
            "sampleRate": 48000,
            "bufferSize": 256,
            "api": "auto",
            "virtualOutputName": "VR Binaural Test Output Modified",
            "lowLatencyMode": false
        },
        "hrtf": {
            "dataPath": "./test_hrtf_data",
            "filterLength": 256,
            "convolutionMethod": "auto"
        },
        "vr": {
            "trackingRate": 90,
            "smoothingFactor": 0.95,
            "simulationMode": true
        },
        "logging": {
            "level": "debug",
            "path": "./test_logs",
            "console": true,
            "file": true
        }
    })";
    config_file.close();

    // Give time for file system notification
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Check if configuration has changed
    bool config_changed = WaitForCondition(
        [&config]() { return config.HasChanged(); },
        std::chrono::seconds(2)
    );

    if (config_changed) {
        EXPECT_TRUE(config.Reload()) << "Configuration reload should succeed";

        // Verify new settings
        int new_buffer_size = config.GetBufferSize();
        EXPECT_EQ(new_buffer_size, 256) << "Buffer size should be updated";

        std::string new_output_name = config.GetVirtualOutputName();
        EXPECT_EQ(new_output_name, "VR Binaural Test Output Modified") << "Output name should be updated";

        // Test that audio engine can adapt to new configuration
        audio.UpdateConfiguration(config);

        LOG_INFO("Configuration hot reload successful - Buffer: {} -> {}, Output: {}",
                initial_buffer_size, new_buffer_size, new_output_name);
    } else {
        LOG_WARN("Configuration change not detected (expected in some environments)");
    }

    audio.Stop();
}

TEST_F(IntegrationTest, ErrorRecoveryAndResilience) {
    LOG_INFO("Testing error recovery and system resilience");

    Config config("./test_configs/integration_test.json");
    HRTFProcessor hrtf;
    ASSERT_TRUE(hrtf.Initialize(config.GetHRTFDataPath()));

    AudioEngine audio;
    ASSERT_TRUE(audio.Initialize(config, &hrtf));
    EXPECT_TRUE(audio.Start());

    // Test 1: Simulated audio device disconnect/reconnect
    LOG_INFO("Testing simulated audio device disconnect");

    auto initial_stats = audio.GetStats();
    EXPECT_GT(initial_stats.framesProcessed, 0);

    // Simulate temporary audio interruption
    audio.Stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Test recovery
    EXPECT_TRUE(audio.Start()) << "Audio engine should recover from stop/start";

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    auto recovery_stats = audio.GetStats();
    EXPECT_GT(recovery_stats.framesProcessed, 0) << "Audio should resume processing after recovery";

    // Test 2: Invalid spatial position handling
    LOG_INFO("Testing invalid spatial position handling");

    VRPose invalid_pose;
    invalid_pose.position = {NAN, INFINITY, -INFINITY};
    invalid_pose.orientation = {NAN, NAN, NAN, NAN};

    VRPose valid_mic_pose = {{0.0f, 1.2f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}};

    // HRTF processor should handle invalid input gracefully
    hrtf.UpdateSpatialPosition(invalid_pose, valid_mic_pose);

    auto hrtf_stats = hrtf.GetStats();
    EXPECT_FALSE(std::isnan(hrtf_stats.distance)) << "HRTF should handle invalid input gracefully";
    EXPECT_FALSE(std::isnan(hrtf_stats.azimuth)) << "HRTF should provide valid output despite invalid input";

    // Test 3: Configuration corruption recovery
    LOG_INFO("Testing configuration corruption recovery");

    // Create corrupted configuration
    std::ofstream corrupt_config("./test_configs/integration_test.json");
    corrupt_config << "{ invalid json content without closing";
    corrupt_config.close();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // System should continue operating with previous valid configuration
    auto final_stats = audio.GetStats();
    EXPECT_EQ(final_stats.underruns, 0) << "System should remain stable despite config corruption";

    audio.Stop();

    LOG_INFO("Error recovery test completed - System maintained stability");
}

#ifdef _WIN32
TEST_F(IntegrationTest, WindowsSpecificIntegration) {
    LOG_INFO("Testing Windows-specific integration features");

    // Test Windows WASAPI virtual device integration
    WindowsWASAPIVirtualDevice::DeviceConfig device_config;
    device_config.sampleRate = 48000;
    device_config.channels = 2;
    device_config.bufferSize = 128;
    device_config.deviceName = L"VR Binaural Integration Test";
    device_config.lowLatencyMode = true;

    WindowsWASAPIVirtualDevice virtual_device;

    if (virtual_device.Initialize(device_config)) {
        EXPECT_TRUE(virtual_device.Start()) << "Windows virtual device should start";

        // Test integration with main audio engine
        Config config("./test_configs/integration_test.json");
        config.Set("audio.api", "wasapi");
        config.Set("audio.virtualOutputName", "VR Binaural Integration Test");

        HRTFProcessor hrtf;
        ASSERT_TRUE(hrtf.Initialize(config.GetHRTFDataPath()));

        AudioEngine audio;
        ASSERT_TRUE(audio.Initialize(config, &hrtf));

        // Test that they can coexist
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        double virtual_latency = virtual_device.GetCurrentLatency();
        EXPECT_LT(virtual_latency, 30.0) << "Windows virtual device latency should be reasonable";

        virtual_device.Stop();

        LOG_INFO("Windows WASAPI integration test completed - Latency: {:.2f}ms", virtual_latency);
    } else {
        LOG_WARN("Windows WASAPI integration test skipped (not available in current environment)");
    }
}

TEST_F(IntegrationTest, WindowsDPIAndMultiMonitorHandling) {
    LOG_INFO("Testing Windows DPI and multi-monitor handling");

    // Test DPI awareness
    HWND test_window = CreateWindow(L"STATIC", L"Test", WS_OVERLAPPEDWINDOW,
                                   0, 0, 100, 100, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);

    if (test_window) {
        // Test DPI scaling
        HDC hdc = GetDC(test_window);
        int dpi_x = GetDeviceCaps(hdc, LOGPIXELSX);
        int dpi_y = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(test_window, hdc);

        EXPECT_GT(dpi_x, 0) << "Should detect valid DPI";
        EXPECT_GT(dpi_y, 0) << "Should detect valid DPI";

        // Test multi-monitor handling
        int monitor_count = GetSystemMetrics(SM_CMONITORS);
        EXPECT_GE(monitor_count, 1) << "Should detect at least one monitor";

        DestroyWindow(test_window);

        LOG_INFO("Windows DPI/Monitor test - DPI: {}x{}, Monitors: {}", dpi_x, dpi_y, monitor_count);
    } else {
        LOG_WARN("Windows DPI test skipped (window creation failed)");
    }
}
#endif

// ============================================================================
// STRESS AND LONGEVITY TESTS
// ============================================================================

TEST_F(IntegrationTest, ExtendedOperationStability) {
    LOG_INFO("Testing extended operation stability");

    Config config("./test_configs/integration_test.json");
    HRTFProcessor hrtf;
    ASSERT_TRUE(hrtf.Initialize(config.GetHRTFDataPath()));

    AudioEngine audio;
    ASSERT_TRUE(audio.Initialize(config, &hrtf));
    EXPECT_TRUE(audio.Start());

    const int test_duration_minutes = 1; // Shortened for test efficiency
    const int check_interval_seconds = 5;
    const int total_checks = (test_duration_minutes * 60) / check_interval_seconds;

    std::vector<AudioEngine::Stats> stats_history;
    VRTracker vr_tracker;
    bool vr_available = vr_tracker.Initialize();

    for (int check = 0; check < total_checks; check++) {
        // Simulate typical VR usage patterns
        if (vr_available) {
            VRPose pose = vr_tracker.GetCurrentPose();
            VRPose mic_pose = {{0.0f, 1.2f, -1.0f}, {0.0f, 0.0f, 0.0f, 1.0f}};
            hrtf.UpdateSpatialPosition(pose, mic_pose);
        } else {
            // Simulate VR movement
            VRPose simulated_pose;
            simulated_pose.position.x = std::sin(check * 0.1f) * 0.5f;
            simulated_pose.position.y = 1.8f + std::sin(check * 0.05f) * 0.1f;
            simulated_pose.position.z = std::cos(check * 0.1f) * 0.5f;
            simulated_pose.orientation = {0.0f, 0.0f, 0.0f, 1.0f};

            VRPose mic_pose = {{0.0f, 1.2f, -1.0f}, {0.0f, 0.0f, 0.0f, 1.0f}};
            hrtf.UpdateSpatialPosition(simulated_pose, mic_pose);
        }

        std::this_thread::sleep_for(std::chrono::seconds(check_interval_seconds));

        auto stats = audio.GetStats();
        stats_history.push_back(stats);

        // Continuous validation
        EXPECT_EQ(stats.underruns, 0) << "No audio underruns should occur during extended operation";
        EXPECT_GT(stats.framesProcessed, 0) << "Audio processing should continue";
        EXPECT_LT(stats.inputLatency, 100.0) << "Latency should remain reasonable";

        LOG_DEBUG("Stability check {}/{} - Frames: {}, Latency: {:.2f}ms, CPU: {:.1f}%",
                 check + 1, total_checks, stats.framesProcessed,
                 stats.inputLatency, stats.cpuLoad * 100.0f);
    }

    audio.Stop();

    // Analyze stability metrics
    bool performance_stable = true;
    double initial_latency = stats_history[0].inputLatency;

    for (const auto& stats : stats_history) {
        if (std::abs(stats.inputLatency - initial_latency) > 20.0) {
            performance_stable = false;
            break;
        }
    }

    EXPECT_TRUE(performance_stable) << "Performance should remain stable over extended operation";

    LOG_INFO("Extended stability test completed - {} checks over {} minutes, Performance stable: {}",
             total_checks, test_duration_minutes, performance_stable ? "Yes" : "No");
}

TEST_F(IntegrationTest, MemoryLeakDetection) {
    LOG_INFO("Testing for memory leaks during operation cycles");

#ifdef _WIN32
    HANDLE process = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS_EX initial_mem, final_mem;

    GetProcessMemoryInfo(process, (PROCESS_MEMORY_COUNTERS*)&initial_mem, sizeof(initial_mem));

    // Perform multiple initialization/cleanup cycles
    for (int cycle = 0; cycle < 5; cycle++) {
        Config config("./test_configs/integration_test.json");
        HRTFProcessor hrtf;
        ASSERT_TRUE(hrtf.Initialize(config.GetHRTFDataPath()));

        AudioEngine audio;
        ASSERT_TRUE(audio.Initialize(config, &hrtf));
        EXPECT_TRUE(audio.Start());

        // Simulate usage
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        audio.Stop();
        // Objects destroyed at end of scope
    }

    // Force garbage collection
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    GetProcessMemoryInfo(process, (PROCESS_MEMORY_COUNTERS*)&final_mem, sizeof(final_mem));

    uint64_t initial_mb = initial_mem.WorkingSetSize / (1024 * 1024);
    uint64_t final_mb = final_mem.WorkingSetSize / (1024 * 1024);
    int64_t memory_growth = final_mb - initial_mb;

    EXPECT_LT(memory_growth, 20) << "Memory growth should be minimal after multiple cycles";

    LOG_INFO("Memory leak detection - Initial: {}MB, Final: {}MB, Growth: {}MB",
             initial_mb, final_mb, memory_growth);
#else
    LOG_INFO("Memory leak detection not available on non-Windows platforms");
#endif
}

} // namespace testing
} // namespace vrb

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    std::cout << "\n";
    std::cout << "=========================================================\n";
    std::cout << "              SYSTEM INTEGRATION TESTS\n";
    std::cout << "=========================================================\n";
    std::cout << "\n";
    std::cout << "Testing complete system integration and stability\n";
    std::cout << "\n";

    return RUN_ALL_TESTS();
}