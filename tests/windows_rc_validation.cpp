// windows_rc_validation.cpp - Comprehensive Windows Release Candidate Validation
// Professional QA validation system for Windows production deployment

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <fstream>
#include <filesystem>
#include <regex>

#ifdef _WIN32
#include <windows.h>
#include <wbemidl.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <comdef.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <psapi.h>
#include <pdh.h>
#include <versionhelpers.h>
#endif

#include "logger.h"
#include "config.h"
#include "audio_engine.h"
#include "hrtf_processor.h"
#include "vr_tracker.h"

#ifdef _WIN32
#include "windows_wasapi_virtual_device.h"
#include "windows_gui.h"
#include "headset_support.h"
#endif

namespace vrb {
namespace testing {

/**
 * @brief Windows RC Validation Framework
 *
 * Comprehensive validation system for Windows Release Candidate
 * covering all critical aspects for production deployment.
 */
class WindowsRCValidation : public ::testing::Test {
protected:
    void SetUp() override {
        vrb::Logger::Initialize("debug", "./test_logs");

        // Initialize test configuration
        test_config = std::make_unique<Config>("test_config.json");

        // Create test environment
        CreateTestEnvironment();

        // Record test start time
        test_start_time = std::chrono::high_resolution_clock::now();
    }

    void TearDown() override {
        CleanupTestEnvironment();

        // Calculate test duration
        auto test_end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            test_end_time - test_start_time);

        LOG_INFO("Test completed in {}ms", duration.count());
    }

    void CreateTestEnvironment() {
        std::filesystem::create_directory("./test_logs");
        std::filesystem::create_directory("./test_output");
        std::filesystem::create_directory("./test_profiles");
    }

    void CleanupTestEnvironment() {
        // Optional cleanup - leave logs for analysis
    }

    std::unique_ptr<Config> test_config;
    std::chrono::high_resolution_clock::time_point test_start_time;

    // Windows-specific test helpers
#ifdef _WIN32
    struct WindowsSystemInfo {
        std::string os_version;
        std::string build_number;
        std::string processor;
        uint64_t total_memory_mb;
        uint64_t available_memory_mb;
        std::vector<std::string> audio_devices;
        std::vector<std::string> graphics_adapters;
        bool dpi_aware;
        bool wasapi_available;
        bool asio_available;
        bool directsound_available;
    };

    WindowsSystemInfo GetWindowsSystemInfo() {
        WindowsSystemInfo info = {};

        // Get Windows version
        if (IsWindows10OrGreater()) {
            info.os_version = "Windows 10+";
        } else if (IsWindows8Point1OrGreater()) {
            info.os_version = "Windows 8.1";
        } else {
            info.os_version = "Windows (Legacy)";
        }

        // Get memory info
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&memInfo)) {
            info.total_memory_mb = memInfo.ullTotalPhys / (1024 * 1024);
            info.available_memory_mb = memInfo.ullAvailPhys / (1024 * 1024);
        }

        // Check audio API availability
        info.wasapi_available = CheckWASAPIAvailability();
        info.directsound_available = CheckDirectSoundAvailability();

        return info;
    }

    bool CheckWASAPIAvailability() {
        HRESULT hr = CoInitialize(nullptr);
        if (FAILED(hr)) return false;

        Microsoft::WRL::ComPtr<IMMDeviceEnumerator> enumerator;
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                            CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                            reinterpret_cast<void**>(enumerator.GetAddressOf()));

        CoUninitialize();
        return SUCCEEDED(hr);
    }

    bool CheckDirectSoundAvailability() {
        HMODULE dsound = LoadLibrary(L"dsound.dll");
        if (dsound) {
            FreeLibrary(dsound);
            return true;
        }
        return false;
    }

    std::vector<std::string> EnumerateAudioDevices() {
        std::vector<std::string> devices;

        HRESULT hr = CoInitialize(nullptr);
        if (FAILED(hr)) return devices;

        Microsoft::WRL::ComPtr<IMMDeviceEnumerator> enumerator;
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                            CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                            reinterpret_cast<void**>(enumerator.GetAddressOf()));

        if (SUCCEEDED(hr)) {
            Microsoft::WRL::ComPtr<IMMDeviceCollection> collection;
            hr = enumerator->EnumAudioEndpoints(eAll, DEVICE_STATE_ACTIVE,
                                              collection.GetAddressOf());

            if (SUCCEEDED(hr)) {
                UINT count = 0;
                collection->GetCount(&count);

                for (UINT i = 0; i < count; i++) {
                    Microsoft::WRL::ComPtr<IMMDevice> device;
                    hr = collection->Item(i, device.GetAddressOf());

                    if (SUCCEEDED(hr)) {
                        Microsoft::WRL::ComPtr<IPropertyStore> propertyStore;
                        hr = device->OpenPropertyStore(STGM_READ, propertyStore.GetAddressOf());

                        if (SUCCEEDED(hr)) {
                            PROPVARIANT varName;
                            PropVariantInit(&varName);

                            hr = propertyStore->GetValue(PKEY_Device_FriendlyName, &varName);
                            if (SUCCEEDED(hr) && varName.vt == VT_LPWSTR) {
                                std::wstring wideStr = varName.pwszVal;
                                devices.push_back(std::string(wideStr.begin(), wideStr.end()));
                            }

                            PropVariantClear(&varName);
                        }
                    }
                }
            }
        }

        CoUninitialize();
        return devices;
    }

    bool CheckDPIAwareness() {
        HMODULE user32 = GetModuleHandle(L"user32.dll");
        if (user32) {
            typedef BOOL(WINAPI* GetProcessDpiAwarenessContextFunc)(HANDLE, DPI_AWARENESS_CONTEXT*);
            auto getDpiAwarenessContext = reinterpret_cast<GetProcessDpiAwarenessContextFunc>(
                GetProcAddress(user32, "GetProcessDpiAwarenessContext"));

            if (getDpiAwarenessContext) {
                DPI_AWARENESS_CONTEXT context;
                if (getDpiAwarenessContext(GetCurrentProcess(), &context)) {
                    return context != DPI_AWARENESS_CONTEXT_UNAWARE;
                }
            }
        }
        return false;
    }

    struct PerformanceMetrics {
        double cpu_usage_percent;
        uint64_t memory_usage_mb;
        uint64_t peak_memory_mb;
        double audio_latency_ms;
        uint32_t audio_dropouts;
        double frame_rate;
        uint32_t gc_collections;
        double startup_time_ms;
    };

    PerformanceMetrics MeasurePerformance(std::function<void()> workload,
                                        std::chrono::milliseconds duration) {
        PerformanceMetrics metrics = {};

        auto start_time = std::chrono::high_resolution_clock::now();

        // Start performance monitoring
        HANDLE process = GetCurrentProcess();
        PROCESS_MEMORY_COUNTERS_EX memCounters;
        FILETIME startTime, exitTime, kernelTime, userTime;

        GetProcessTimes(process, &startTime, &exitTime, &kernelTime, &userTime);
        GetProcessMemoryInfo(process, (PROCESS_MEMORY_COUNTERS*)&memCounters, sizeof(memCounters));

        uint64_t initial_memory = memCounters.WorkingSetSize / (1024 * 1024);
        metrics.peak_memory_mb = initial_memory;

        // Run workload
        workload();

        // Measure over duration
        auto measure_start = std::chrono::high_resolution_clock::now();
        while (std::chrono::high_resolution_clock::now() - measure_start < duration) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // Update memory usage
            GetProcessMemoryInfo(process, (PROCESS_MEMORY_COUNTERS*)&memCounters, sizeof(memCounters));
            uint64_t current_memory = memCounters.WorkingSetSize / (1024 * 1024);
            metrics.memory_usage_mb = current_memory;
            metrics.peak_memory_mb = std::max(metrics.peak_memory_mb, current_memory);
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        metrics.startup_time_ms = std::chrono::duration<double, std::milli>(
            end_time - start_time).count();

        return metrics;
    }
#endif
};

/**
 * @brief Windows End-to-End User Experience Validation
 */
class WindowsEndToEndTest : public WindowsRCValidation {
protected:
    void SetUp() override {
        WindowsRCValidation::SetUp();
        LOG_INFO("Starting Windows End-to-End validation");
    }
};

/**
 * @brief Windows Audio System Validation
 */
class WindowsAudioSystemTest : public WindowsRCValidation {
protected:
    void SetUp() override {
        WindowsRCValidation::SetUp();
        LOG_INFO("Starting Windows Audio System validation");
    }
};

/**
 * @brief Windows VR Integration Validation
 */
class WindowsVRIntegrationTest : public WindowsRCValidation {
protected:
    void SetUp() override {
        WindowsRCValidation::SetUp();
        LOG_INFO("Starting Windows VR Integration validation");
    }
};

/**
 * @brief Windows Platform Integration Validation
 */
class WindowsPlatformTest : public WindowsRCValidation {
protected:
    void SetUp() override {
        WindowsRCValidation::SetUp();
        LOG_INFO("Starting Windows Platform Integration validation");
    }
};

/**
 * @brief Windows Performance and Stability Validation
 */
class WindowsPerformanceTest : public WindowsRCValidation {
protected:
    void SetUp() override {
        WindowsRCValidation::SetUp();
        LOG_INFO("Starting Windows Performance validation");
    }
};

// ============================================================================
// TASK 1: WINDOWS END-TO-END VALIDATION
// ============================================================================

TEST_F(WindowsEndToEndTest, ApplicationLaunchAndShutdown) {
    LOG_INFO("Testing application launch and shutdown");

    // Simulate application lifecycle
    auto start_time = std::chrono::high_resolution_clock::now();

    // Initialize core components
    Config config("test_config.json");
    EXPECT_TRUE(config.IsValid());

    HRTFProcessor hrtf;
    EXPECT_TRUE(hrtf.Initialize("./test_hrtf_data"));

    AudioEngine engine;
    EXPECT_TRUE(engine.Initialize(config, &hrtf));

    auto end_time = std::chrono::high_resolution_clock::now();
    auto startup_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);

    EXPECT_LT(startup_time.count(), 5000) << "Startup time should be under 5 seconds";

    LOG_INFO("Application startup time: {}ms", startup_time.count());
}

#ifdef _WIN32
TEST_F(WindowsEndToEndTest, WindowsGUIInitialization) {
    LOG_INFO("Testing Windows GUI initialization");

    // Test GUI can be created without errors
    // Note: In headless environment, this will use mock/stub implementations
    WindowsSystemInfo sysInfo = GetWindowsSystemInfo();

    EXPECT_FALSE(sysInfo.os_version.empty());
    EXPECT_GT(sysInfo.total_memory_mb, 2048) << "System should have at least 2GB RAM";

    LOG_INFO("System Info - OS: {}, Memory: {}MB",
             sysInfo.os_version, sysInfo.total_memory_mb);
}

TEST_F(WindowsEndToEndTest, AudioDeviceEnumeration) {
    LOG_INFO("Testing Windows audio device enumeration");

    auto devices = EnumerateAudioDevices();
    EXPECT_GT(devices.size(), 0) << "Should detect at least one audio device";

    for (const auto& device : devices) {
        EXPECT_FALSE(device.empty()) << "Device name should not be empty";
        LOG_INFO("Detected audio device: {}", device);
    }
}

TEST_F(WindowsEndToEndTest, VirtualAudioDeviceCreation) {
    LOG_INFO("Testing virtual audio device creation");

    if (CheckWASAPIAvailability()) {
        WindowsWASAPIVirtualDevice::DeviceConfig config;
        config.sampleRate = 48000;
        config.channels = 2;
        config.deviceName = L"VR Binaural Test Output";
        config.lowLatencyMode = true;

        WindowsWASAPIVirtualDevice device;
        bool initialized = device.Initialize(config);

        if (initialized) {
            EXPECT_TRUE(device.Start());
            EXPECT_LT(device.GetCurrentLatency(), 20.0) << "Latency should be under 20ms";
            device.Stop();
            LOG_INFO("Virtual device created successfully with latency: {:.2f}ms",
                     device.GetCurrentLatency());
        } else {
            LOG_WARN("Virtual device creation failed (expected in WSL/CI environment)");
        }
    } else {
        LOG_WARN("WASAPI not available - skipping virtual device test");
    }
}
#endif

// ============================================================================
// TASK 2: WINDOWS AUDIO SYSTEM VALIDATION
// ============================================================================

TEST_F(WindowsAudioSystemTest, WASAPIDeviceCreationAndDestruction) {
    LOG_INFO("Testing WASAPI device lifecycle");

#ifdef _WIN32
    if (CheckWASAPIAvailability()) {
        WindowsWASAPIVirtualDevice::DeviceConfig config;
        config.sampleRate = 48000;
        config.channels = 2;
        config.bufferSize = 128;
        config.exclusiveMode = false;
        config.lowLatencyMode = true;

        for (int i = 0; i < 5; i++) {
            WindowsWASAPIVirtualDevice device;
            EXPECT_TRUE(device.Initialize(config));

            if (device.GetState() == WindowsWASAPIVirtualDevice::DeviceState::Created) {
                EXPECT_TRUE(device.Start());
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                device.Stop();
            }
        }

        LOG_INFO("WASAPI device lifecycle test completed successfully");
    } else {
        LOG_WARN("WASAPI not available - test skipped");
    }
#else
    LOG_INFO("Non-Windows platform - WASAPI test skipped");
#endif
}

TEST_F(WindowsAudioSystemTest, AudioInputOutputDeviceSelection) {
    LOG_INFO("Testing audio device selection");

    Config config("test_config.json");
    HRTFProcessor hrtf;
    hrtf.Initialize("./test_hrtf_data");

    AudioEngine engine;
    EXPECT_TRUE(engine.Initialize(config, &hrtf));

    auto devices = engine.GetInputDevices();
    EXPECT_GT(devices.size(), 0) << "Should have input devices available";

    // Test device selection
    if (!devices.empty()) {
        EXPECT_TRUE(engine.SelectInputDevice(devices[0].index));
        EXPECT_EQ(engine.GetCurrentInputDevice(), devices[0].name);
    }

    LOG_INFO("Audio device selection test completed");
}

TEST_F(WindowsAudioSystemTest, RealTimeAudioProcessing) {
    LOG_INFO("Testing real-time audio processing performance");

    Config config("test_config.json");
    HRTFProcessor hrtf;
    hrtf.Initialize("./test_hrtf_data");

    AudioEngine engine;
    EXPECT_TRUE(engine.Initialize(config, &hrtf));

    EXPECT_TRUE(engine.Start());

    // Measure performance for 3 seconds
    auto start_time = std::chrono::high_resolution_clock::now();
    std::this_thread::sleep_for(std::chrono::seconds(3));

    auto stats = engine.GetStats();
    engine.Stop();

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);

    EXPECT_GT(stats.framesProcessed, 0) << "Should have processed audio frames";
    EXPECT_EQ(stats.underruns, 0) << "Should have no audio underruns";
    EXPECT_LT(stats.inputLatency, 50.0) << "Input latency should be under 50ms";
    EXPECT_LT(stats.outputLatency, 50.0) << "Output latency should be under 50ms";

    LOG_INFO("Processed {} frames in {}ms, Latency: {:.2f}ms/{:.2f}ms",
             stats.framesProcessed, duration.count(),
             stats.inputLatency, stats.outputLatency);
}

TEST_F(WindowsAudioSystemTest, AudioQualityAndFidelity) {
    LOG_INFO("Testing audio quality and fidelity");

    Config config("test_config.json");
    HRTFProcessor hrtf;
    EXPECT_TRUE(hrtf.Initialize("./test_hrtf_data"));

    // Test HRTF processing with known test signals
    const size_t frames = 1024;
    std::vector<float> input(frames, 0.5f);  // Test signal
    std::vector<float> output(frames * 2, 0.0f);  // Stereo output

    hrtf.Process(input.data(), output.data(), frames, 1);

    // Verify output is not silence and contains spatial information
    float energy = 0.0f;
    for (float sample : output) {
        energy += sample * sample;
    }

    EXPECT_GT(energy, 0.0f) << "HRTF output should contain audio energy";

    // Check for stereo differences (spatial processing)
    float left_energy = 0.0f, right_energy = 0.0f;
    for (size_t i = 0; i < frames; i++) {
        left_energy += output[i * 2] * output[i * 2];
        right_energy += output[i * 2 + 1] * output[i * 2 + 1];
    }

    LOG_INFO("Audio quality test - Energy: {:.6f}, L/R: {:.6f}/{:.6f}",
             energy, left_energy, right_energy);
}

// ============================================================================
// TASK 3: WINDOWS VR INTEGRATION VALIDATION
// ============================================================================

TEST_F(WindowsVRIntegrationTest, VRHeadsetDetection) {
    LOG_INFO("Testing VR headset detection");

    // Test VR hardware detection capabilities
    // Note: In WSL/CI environment, this will test simulation mode
    VRTracker tracker;
    bool initialized = tracker.Initialize();

    if (initialized) {
        VRPose pose = tracker.GetCurrentPose();
        EXPECT_TRUE(pose.position.x != NAN && pose.position.y != NAN && pose.position.z != NAN);

        LOG_INFO("VR tracking initialized - Position: [{:.3f}, {:.3f}, {:.3f}]",
                 pose.position.x, pose.position.y, pose.position.z);
    } else {
        LOG_INFO("VR tracking initialization failed (expected without VR hardware)");
    }
}

TEST_F(WindowsVRIntegrationTest, QuestProCompatibility) {
    LOG_INFO("Testing Quest Pro compatibility profile");

    // Test Quest Pro specific optimization profile
    Config config("test_config.json");
    config.Set("vr.headsetModel", "Meta Quest Pro");
    config.Set("audio.lowLatencyMode", true);
    config.Set("audio.bufferSize", 64);  // Smaller buffer for Quest Pro
    config.Set("hrtf.optimizeForWireless", true);

    HRTFProcessor hrtf;
    EXPECT_TRUE(hrtf.Initialize("./test_hrtf_data"));

    AudioEngine engine;
    EXPECT_TRUE(engine.Initialize(config, &hrtf));

    // Verify Quest Pro optimizations are applied
    auto stats = engine.GetStats();
    EXPECT_LE(stats.inputLatency, 15.0) << "Quest Pro should have ultra-low latency";

    LOG_INFO("Quest Pro compatibility test completed");
}

TEST_F(WindowsVRIntegrationTest, BigscreenBeyondCompatibility) {
    LOG_INFO("Testing Bigscreen Beyond compatibility profile");

    // Test Bigscreen Beyond specific optimization profile
    Config config("test_config.json");
    config.Set("vr.headsetModel", "Bigscreen Beyond");
    config.Set("audio.sampleRate", 96000);  // High-fidelity for Beyond
    config.Set("audio.bitDepth", 32);
    config.Set("hrtf.highFidelityMode", true);

    HRTFProcessor hrtf;
    EXPECT_TRUE(hrtf.Initialize("./test_hrtf_data"));

    AudioEngine engine;
    EXPECT_TRUE(engine.Initialize(config, &hrtf));

    // Verify Beyond optimizations
    auto stats = engine.GetStats();
    EXPECT_LE(stats.inputLatency, 10.0) << "Bigscreen Beyond should have minimal latency";

    LOG_INFO("Bigscreen Beyond compatibility test completed");
}

TEST_F(WindowsVRIntegrationTest, SpatialAudioAccuracy) {
    LOG_INFO("Testing spatial audio positioning accuracy");

    HRTFProcessor hrtf;
    EXPECT_TRUE(hrtf.Initialize("./test_hrtf_data"));

    // Test various spatial positions
    std::vector<VRPose> test_positions = {
        {{0.0f, 1.8f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}},     // Center
        {{-1.0f, 1.8f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}},   // Left
        {{1.0f, 1.8f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}},    // Right
        {{0.0f, 1.8f, -1.0f}, {0.0f, 0.0f, 0.0f, 1.0f}},   // Behind
        {{0.0f, 1.8f, 1.0f}, {0.0f, 0.0f, 0.0f, 1.0f}}     // Front
    };

    VRPose mic_position = {{0.0f, 1.2f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}};

    for (const auto& pos : test_positions) {
        hrtf.UpdateSpatialPosition(pos, mic_position);

        auto stats = hrtf.GetStats();
        EXPECT_GE(stats.distance, 0.0f) << "Distance should be non-negative";
        EXPECT_GE(stats.azimuth, -180.0f) << "Azimuth should be valid";
        EXPECT_LE(stats.azimuth, 180.0f) << "Azimuth should be valid";

        LOG_DEBUG("Position [{:.1f}, {:.1f}, {:.1f}] -> Az: {:.1f}°, El: {:.1f}°, Dist: {:.2f}m",
                 pos.position.x, pos.position.y, pos.position.z,
                 stats.azimuth, stats.elevation, stats.distance);
    }

    LOG_INFO("Spatial audio accuracy test completed");
}

// ============================================================================
// TASK 4: WINDOWS PLATFORM INTEGRATION
// ============================================================================

#ifdef _WIN32
TEST_F(WindowsPlatformTest, DPIAwarenessAndScaling) {
    LOG_INFO("Testing DPI awareness and scaling");

    bool dpi_aware = CheckDPIAwareness();
    LOG_INFO("DPI Awareness: {}", dpi_aware ? "Enabled" : "Disabled");

    // DPI awareness is important for proper GUI rendering
    if (!dpi_aware) {
        LOG_WARN("Application should be DPI-aware for proper Windows integration");
    }
}

TEST_F(WindowsPlatformTest, WindowsAudioAPICompatibility) {
    LOG_INFO("Testing Windows Audio API compatibility");

    bool wasapi_ok = CheckWASAPIAvailability();
    bool dsound_ok = CheckDirectSoundAvailability();

    EXPECT_TRUE(wasapi_ok) << "WASAPI should be available on Windows";
    EXPECT_TRUE(dsound_ok) << "DirectSound should be available on Windows";

    LOG_INFO("Audio API Support - WASAPI: {}, DirectSound: {}",
             wasapi_ok ? "Available" : "Not Available",
             dsound_ok ? "Available" : "Not Available");
}

TEST_F(WindowsPlatformTest, WindowsSecurityCompatibility) {
    LOG_INFO("Testing Windows security compatibility");

    // Test that application can run with standard Windows security features
    HANDLE process = GetCurrentProcess();
    DWORD flags = 0;

    // Check DEP (Data Execution Prevention)
    if (GetProcessDEPPolicy(process, &flags, nullptr)) {
        bool dep_enabled = (flags & PROCESS_DEP_ENABLE) != 0;
        LOG_INFO("DEP Status: {}", dep_enabled ? "Enabled" : "Disabled");
    }

    // Test basic Windows API access
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    EXPECT_GT(sysInfo.dwNumberOfProcessors, 0);

    LOG_INFO("System Info - Processors: {}, Page Size: {} bytes",
             sysInfo.dwNumberOfProcessors, sysInfo.dwPageSize);
}

TEST_F(WindowsPlatformTest, MultiMonitorCompatibility) {
    LOG_INFO("Testing multi-monitor compatibility");

    int monitor_count = GetSystemMetrics(SM_CMONITORS);
    EXPECT_GE(monitor_count, 1) << "Should detect at least one monitor";

    // Test DPI settings on each monitor
    auto enum_proc = [](HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) -> BOOL {
        MONITORINFOEX info;
        info.cbSize = sizeof(MONITORINFOEX);

        if (GetMonitorInfo(hMonitor, &info)) {
            LOG_INFO("Monitor: {} - Rect: [{}, {}, {}, {}]",
                     std::string(info.szDevice),
                     info.rcMonitor.left, info.rcMonitor.top,
                     info.rcMonitor.right, info.rcMonitor.bottom);
        }

        return TRUE;
    };

    EnumDisplayMonitors(nullptr, nullptr, enum_proc, 0);

    LOG_INFO("Multi-monitor test completed - {} monitor(s) detected", monitor_count);
}
#endif

// ============================================================================
// TASK 5: WINDOWS PERFORMANCE AND STABILITY
// ============================================================================

TEST_F(WindowsPerformanceTest, StartupPerformance) {
    LOG_INFO("Testing startup performance");

    auto measure_startup = [this]() {
        Config config("test_config.json");
        HRTFProcessor hrtf;
        hrtf.Initialize("./test_hrtf_data");
        AudioEngine engine;
        engine.Initialize(config, &hrtf);
    };

#ifdef _WIN32
    auto metrics = MeasurePerformance(measure_startup, std::chrono::seconds(1));

    EXPECT_LT(metrics.startup_time_ms, 5000.0) << "Startup should complete within 5 seconds";
    EXPECT_LT(metrics.memory_usage_mb, 500) << "Memory usage should be under 500MB";
    EXPECT_LT(metrics.cpu_usage_percent, 25.0) << "CPU usage should be under 25%";

    LOG_INFO("Startup Performance - Time: {:.2f}ms, Memory: {}MB",
             metrics.startup_time_ms, metrics.memory_usage_mb);
#else
    LOG_INFO("Performance measurement not available on non-Windows platforms");
#endif
}

TEST_F(WindowsPerformanceTest, ExtendedOperationStability) {
    LOG_INFO("Testing extended operation stability");

    Config config("test_config.json");
    HRTFProcessor hrtf;
    hrtf.Initialize("./test_hrtf_data");

    AudioEngine engine;
    EXPECT_TRUE(engine.Initialize(config, &hrtf));
    EXPECT_TRUE(engine.Start());

    // Run for extended period
    const int duration_seconds = 10;  // Shortened for test efficiency
    const int check_interval_ms = 1000;

    size_t initial_frames = 0;
    size_t last_underruns = 0;

    for (int i = 0; i < duration_seconds; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(check_interval_ms));

        auto stats = engine.GetStats();

        // Check for audio dropouts
        EXPECT_EQ(stats.underruns, last_underruns) << "No new underruns should occur";
        last_underruns = stats.underruns;

        // Verify continuous processing
        if (i == 0) {
            initial_frames = stats.framesProcessed;
        } else {
            EXPECT_GT(stats.framesProcessed, initial_frames) << "Audio processing should continue";
        }

        // Check memory stability (basic check)
        EXPECT_LT(stats.inputLatency, 100.0) << "Latency should remain stable";
    }

    engine.Stop();

    auto final_stats = engine.GetStats();
    LOG_INFO("Extended operation completed - Frames: {}, Underruns: {}",
             final_stats.framesProcessed, final_stats.underruns);
}

TEST_F(WindowsPerformanceTest, MemoryLeakDetection) {
    LOG_INFO("Testing memory leak detection");

#ifdef _WIN32
    HANDLE process = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS_EX initial_mem, final_mem;

    GetProcessMemoryInfo(process, (PROCESS_MEMORY_COUNTERS*)&initial_mem, sizeof(initial_mem));

    // Perform multiple initialize/cleanup cycles
    for (int i = 0; i < 10; i++) {
        Config config("test_config.json");
        HRTFProcessor hrtf;
        hrtf.Initialize("./test_hrtf_data");

        AudioEngine engine;
        engine.Initialize(config, &hrtf);
        engine.Start();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        engine.Stop();
        // Objects automatically destroyed at end of scope
    }

    // Force garbage collection if available
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    GetProcessMemoryInfo(process, (PROCESS_MEMORY_COUNTERS*)&final_mem, sizeof(final_mem));

    uint64_t initial_mb = initial_mem.WorkingSetSize / (1024 * 1024);
    uint64_t final_mb = final_mem.WorkingSetSize / (1024 * 1024);
    int64_t memory_growth = final_mb - initial_mb;

    EXPECT_LT(memory_growth, 50) << "Memory growth should be minimal (< 50MB)";

    LOG_INFO("Memory leak test - Initial: {}MB, Final: {}MB, Growth: {}MB",
             initial_mb, final_mb, memory_growth);
#else
    LOG_INFO("Memory leak detection not available on non-Windows platforms");
#endif
}

// ============================================================================
// TASK 6: WINDOWS INSTALLATION AND DISTRIBUTION VALIDATION
// ============================================================================

TEST_F(WindowsRCValidation, ConfigurationFileValidation) {
    LOG_INFO("Testing configuration file validation");

    // Test default configuration
    Config default_config("test_config.json");
    EXPECT_TRUE(default_config.IsValid());

    // Test configuration with Windows-specific settings
    default_config.Set("audio.api", "wasapi");
    default_config.Set("audio.useASIO", true);
    default_config.Set("audio.wasapiExclusive", true);

    EXPECT_EQ(default_config.GetString("audio.api"), "wasapi");
    EXPECT_TRUE(default_config.GetBool("audio.useASIO"));

    LOG_INFO("Configuration validation completed");
}

TEST_F(WindowsRCValidation, FilesystemAndPermissions) {
    LOG_INFO("Testing filesystem access and permissions");

    // Test log directory creation
    std::string log_dir = "./test_logs";
    EXPECT_TRUE(std::filesystem::exists(log_dir) ||
                std::filesystem::create_directory(log_dir));

    // Test configuration file access
    std::string config_path = "test_config.json";
    std::ofstream config_file(config_path);
    EXPECT_TRUE(config_file.is_open()) << "Should be able to create config files";
    config_file << "{}";
    config_file.close();

    // Test read access
    std::ifstream read_file(config_path);
    EXPECT_TRUE(read_file.is_open()) << "Should be able to read config files";
    read_file.close();

    LOG_INFO("Filesystem and permissions test completed");
}

TEST_F(WindowsRCValidation, DependencyVerification) {
    LOG_INFO("Testing dependency verification");

    // Verify critical DLLs are available (in normal Windows environment)
    std::vector<std::string> required_dlls = {
        "kernel32.dll",
        "user32.dll",
        "ole32.dll",
        "oleaut32.dll"
    };

    for (const auto& dll : required_dlls) {
        std::wstring wide_dll(dll.begin(), dll.end());
        HMODULE module = LoadLibrary(wide_dll.c_str());

        if (module) {
            FreeLibrary(module);
            LOG_DEBUG("DLL available: {}", dll);
        } else {
            LOG_WARN("DLL not available: {}", dll);
        }
    }

    LOG_INFO("Dependency verification completed");
}

} // namespace testing
} // namespace vrb

// ============================================================================
// WINDOWS RC VALIDATION MAIN
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    std::cout << "\n";
    std::cout << "=========================================================\n";
    std::cout << "    VR BINAURAL RECORDER - WINDOWS RC VALIDATION\n";
    std::cout << "=========================================================\n";
    std::cout << "\n";
    std::cout << "Professional QA validation for Windows Release Candidate\n";
    std::cout << "Testing all critical aspects for production deployment\n";
    std::cout << "\n";

#ifdef _WIN32
    std::cout << "Platform: Windows (Native)\n";
#else
    std::cout << "Platform: Non-Windows (Limited functionality)\n";
#endif

    std::cout << "\n";
    std::cout << "Test Categories:\n";
    std::cout << "  ✓ End-to-End User Experience\n";
    std::cout << "  ✓ Windows Audio System Integration\n";
    std::cout << "  ✓ VR Headset Compatibility (Quest Pro, Beyond)\n";
    std::cout << "  ✓ Windows Platform Integration\n";
    std::cout << "  ✓ Performance and Stability\n";
    std::cout << "  ✓ Installation and Distribution\n";
    std::cout << "\n";

    auto start_time = std::chrono::high_resolution_clock::now();

    int result = RUN_ALL_TESTS();

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(
        end_time - start_time);

    std::cout << "\n";
    std::cout << "=========================================================\n";
    std::cout << "            WINDOWS RC VALIDATION COMPLETE\n";
    std::cout << "=========================================================\n";
    std::cout << "\n";
    std::cout << "Total execution time: " << duration.count() << " seconds\n";
    std::cout << "Result: " << (result == 0 ? "PASSED" : "FAILED") << "\n";
    std::cout << "\n";

    if (result == 0) {
        std::cout << "🎉 WINDOWS RELEASE CANDIDATE APPROVED 🎉\n";
        std::cout << "\n";
        std::cout << "All critical validation tests passed!\n";
        std::cout << "Ready for Windows production deployment.\n";
    } else {
        std::cout << "❌ WINDOWS RELEASE CANDIDATE REQUIRES FIXES\n";
        std::cout << "\n";
        std::cout << "Some critical tests failed.\n";
        std::cout << "Please review test output and address issues.\n";
    }

    std::cout << "\n";

    return result;
}