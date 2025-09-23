// audio_performance_tests.cpp - Windows Audio Performance Validation
// Comprehensive audio system performance testing for Windows RC

#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <vector>
#include <cmath>
#include <atomic>

#include "logger.h"
#include "config.h"
#include "audio_engine.h"
#include "hrtf_processor.h"

#ifdef _WIN32
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>
#include "windows_wasapi_virtual_device.h"
#endif

namespace vrb {
namespace testing {

class AudioPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        vrb::Logger::Initialize("debug", "./test_logs");

        test_config = std::make_unique<Config>("test_config.json");
        hrtf_processor = std::make_unique<HRTFProcessor>();
        audio_engine = std::make_unique<AudioEngine>();

        ASSERT_TRUE(hrtf_processor->Initialize("./test_hrtf_data"));
        ASSERT_TRUE(audio_engine->Initialize(*test_config, hrtf_processor.get()));
    }

    void TearDown() override {
        if (audio_engine) {
            audio_engine->Stop();
        }
    }

    std::unique_ptr<Config> test_config;
    std::unique_ptr<HRTFProcessor> hrtf_processor;
    std::unique_ptr<AudioEngine> audio_engine;

    struct LatencyMeasurement {
        double input_latency_ms;
        double output_latency_ms;
        double roundtrip_latency_ms;
        std::chrono::high_resolution_clock::time_point timestamp;
    };

    std::vector<LatencyMeasurement> MeasureLatencyOverTime(
        std::chrono::milliseconds duration,
        std::chrono::milliseconds interval) {

        std::vector<LatencyMeasurement> measurements;
        auto start_time = std::chrono::high_resolution_clock::now();
        auto next_measurement = start_time;

        EXPECT_TRUE(audio_engine->Start());

        while (std::chrono::high_resolution_clock::now() - start_time < duration) {
            if (std::chrono::high_resolution_clock::now() >= next_measurement) {
                auto stats = audio_engine->GetStats();

                LatencyMeasurement measurement;
                measurement.input_latency_ms = stats.inputLatency;
                measurement.output_latency_ms = stats.outputLatency;
                measurement.roundtrip_latency_ms = stats.inputLatency + stats.outputLatency;
                measurement.timestamp = std::chrono::high_resolution_clock::now();

                measurements.push_back(measurement);
                next_measurement += interval;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        audio_engine->Stop();
        return measurements;
    }

    double CalculateAverageLatency(const std::vector<LatencyMeasurement>& measurements) {
        if (measurements.empty()) return 0.0;

        double total = 0.0;
        for (const auto& measurement : measurements) {
            total += measurement.roundtrip_latency_ms;
        }
        return total / measurements.size();
    }

    double CalculateLatencyStandardDeviation(const std::vector<LatencyMeasurement>& measurements) {
        if (measurements.size() < 2) return 0.0;

        double mean = CalculateAverageLatency(measurements);
        double variance = 0.0;

        for (const auto& measurement : measurements) {
            double diff = measurement.roundtrip_latency_ms - mean;
            variance += diff * diff;
        }

        variance /= (measurements.size() - 1);
        return std::sqrt(variance);
    }
};

// ============================================================================
// LATENCY PERFORMANCE TESTS
// ============================================================================

TEST_F(AudioPerformanceTest, LowLatencyPerformance) {
    LOG_INFO("Testing low-latency audio performance");

    // Configure for ultra-low latency
    test_config->Set("audio.bufferSize", 64);
    test_config->Set("audio.lowLatencyMode", true);
    test_config->Set("performance.threadPriority", "realtime");

    // Reinitialize with low-latency settings
    audio_engine = std::make_unique<AudioEngine>();
    ASSERT_TRUE(audio_engine->Initialize(*test_config, hrtf_processor.get()));

    auto measurements = MeasureLatencyOverTime(
        std::chrono::seconds(5),
        std::chrono::milliseconds(100)
    );

    ASSERT_GT(measurements.size(), 0);

    double avg_latency = CalculateAverageLatency(measurements);
    double latency_stddev = CalculateLatencyStandardDeviation(measurements);

    // For VR applications, total roundtrip latency should be under 20ms
    EXPECT_LT(avg_latency, 20.0) << "Average roundtrip latency too high for VR";
    EXPECT_LT(latency_stddev, 5.0) << "Latency variation too high (jitter)";

    LOG_INFO("Low-latency performance - Avg: {:.2f}ms, StdDev: {:.2f}ms",
             avg_latency, latency_stddev);
}

TEST_F(AudioPerformanceTest, LatencyConsistency) {
    LOG_INFO("Testing latency consistency over extended period");

    auto measurements = MeasureLatencyOverTime(
        std::chrono::seconds(30),
        std::chrono::milliseconds(500)
    );

    ASSERT_GT(measurements.size(), 10);

    // Check for latency spikes
    double avg_latency = CalculateAverageLatency(measurements);
    int spike_count = 0;
    const double spike_threshold = avg_latency * 2.0; // 2x average is a spike

    for (const auto& measurement : measurements) {
        if (measurement.roundtrip_latency_ms > spike_threshold) {
            spike_count++;
        }
    }

    double spike_percentage = (static_cast<double>(spike_count) / measurements.size()) * 100.0;

    EXPECT_LT(spike_percentage, 5.0) << "Too many latency spikes detected";

    LOG_INFO("Latency consistency - Spikes: {:.1f}% ({}/{})",
             spike_percentage, spike_count, measurements.size());
}

#ifdef _WIN32
TEST_F(AudioPerformanceTest, WASAPIExclusiveModeLatency) {
    LOG_INFO("Testing WASAPI exclusive mode latency");

    test_config->Set("audio.api", "wasapi");
    test_config->Set("audio.wasapiExclusive", true);
    test_config->Set("audio.bufferSize", 64);

    // Test WASAPI virtual device directly
    WindowsWASAPIVirtualDevice::DeviceConfig device_config;
    device_config.sampleRate = 48000;
    device_config.channels = 2;
    device_config.bufferSize = 64;
    device_config.exclusiveMode = true;
    device_config.lowLatencyMode = true;

    WindowsWASAPIVirtualDevice virtual_device;

    if (virtual_device.Initialize(device_config)) {
        EXPECT_TRUE(virtual_device.Start());

        // Measure latency directly from WASAPI device
        double wasapi_latency = virtual_device.GetCurrentLatency();
        EXPECT_LT(wasapi_latency, 15.0) << "WASAPI exclusive mode latency too high";

        virtual_device.Stop();

        LOG_INFO("WASAPI exclusive mode latency: {:.2f}ms", wasapi_latency);
    } else {
        LOG_WARN("WASAPI exclusive mode test skipped (not available in current environment)");
    }
}

TEST_F(AudioPerformanceTest, WASAPISharedModeLatency) {
    LOG_INFO("Testing WASAPI shared mode latency");

    test_config->Set("audio.api", "wasapi");
    test_config->Set("audio.wasapiExclusive", false);
    test_config->Set("audio.bufferSize", 128);

    WindowsWASAPIVirtualDevice::DeviceConfig device_config;
    device_config.sampleRate = 48000;
    device_config.channels = 2;
    device_config.bufferSize = 128;
    device_config.exclusiveMode = false;
    device_config.lowLatencyMode = false;

    WindowsWASAPIVirtualDevice virtual_device;

    if (virtual_device.Initialize(device_config)) {
        EXPECT_TRUE(virtual_device.Start());

        double wasapi_latency = virtual_device.GetCurrentLatency();
        EXPECT_LT(wasapi_latency, 30.0) << "WASAPI shared mode latency too high";

        virtual_device.Stop();

        LOG_INFO("WASAPI shared mode latency: {:.2f}ms", wasapi_latency);
    } else {
        LOG_WARN("WASAPI shared mode test skipped (not available in current environment)");
    }
}
#endif

// ============================================================================
// THROUGHPUT AND PROCESSING TESTS
// ============================================================================

TEST_F(AudioPerformanceTest, AudioProcessingThroughput) {
    LOG_INFO("Testing audio processing throughput");

    EXPECT_TRUE(audio_engine->Start());

    auto start_time = std::chrono::high_resolution_clock::now();
    size_t initial_frames = audio_engine->GetStats().framesProcessed;

    // Run for 5 seconds
    std::this_thread::sleep_for(std::chrono::seconds(5));

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time).count();

    size_t final_frames = audio_engine->GetStats().framesProcessed;
    size_t frames_processed = final_frames - initial_frames;

    audio_engine->Stop();

    // Calculate throughput
    double frames_per_second = (static_cast<double>(frames_processed) * 1000.0) / duration_ms;
    double expected_fps = test_config->GetSampleRate();
    double throughput_ratio = frames_per_second / expected_fps;

    EXPECT_GT(throughput_ratio, 0.95) << "Audio throughput too low";
    EXPECT_LT(throughput_ratio, 1.05) << "Audio throughput inconsistent";

    LOG_INFO("Audio throughput - {:.0f} fps (expected: {:.0f}, ratio: {:.3f})",
             frames_per_second, expected_fps, throughput_ratio);
}

TEST_F(AudioPerformanceTest, HRTFProcessingPerformance) {
    LOG_INFO("Testing HRTF processing performance");

    const size_t test_frames = 1024;
    const int iterations = 1000;

    std::vector<float> input_mono(test_frames, 0.5f);
    std::vector<float> input_stereo(test_frames * 2, 0.5f);
    std::vector<float> output(test_frames * 2, 0.0f);

    // Warm up
    for (int i = 0; i < 10; i++) {
        hrtf_processor->Process(input_mono.data(), output.data(), test_frames, 1);
    }

    // Measure mono processing
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; i++) {
        hrtf_processor->Process(input_mono.data(), output.data(), test_frames, 1);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto mono_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time);

    // Measure stereo processing
    start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; i++) {
        hrtf_processor->Process(input_stereo.data(), output.data(), test_frames, 2);
    }

    end_time = std::chrono::high_resolution_clock::now();
    auto stereo_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time);

    double mono_time_per_frame = static_cast<double>(mono_duration.count()) /
                                (iterations * test_frames);
    double stereo_time_per_frame = static_cast<double>(stereo_duration.count()) /
                                  (iterations * test_frames);

    // HRTF processing should be fast enough for real-time
    // At 48kHz, each frame should process in under 20.8 microseconds
    EXPECT_LT(mono_time_per_frame, 20.0) << "HRTF mono processing too slow";
    EXPECT_LT(stereo_time_per_frame, 25.0) << "HRTF stereo processing too slow";

    LOG_INFO("HRTF performance - Mono: {:.2f}μs/frame, Stereo: {:.2f}μs/frame",
             mono_time_per_frame, stereo_time_per_frame);
}

TEST_F(AudioPerformanceTest, SpatialAudioUpdatePerformance) {
    LOG_INFO("Testing spatial audio update performance");

    const int position_updates = 1000;
    std::vector<VRPose> test_poses;

    // Generate test positions
    for (int i = 0; i < position_updates; i++) {
        VRPose pose;
        pose.position.x = std::sin(i * 0.1f) * 2.0f;
        pose.position.y = 1.8f;
        pose.position.z = std::cos(i * 0.1f) * 2.0f;
        pose.orientation = {0.0f, 0.0f, 0.0f, 1.0f};
        test_poses.push_back(pose);
    }

    VRPose mic_pose = {{0.0f, 1.2f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}};

    // Measure spatial update performance
    auto start_time = std::chrono::high_resolution_clock::now();

    for (const auto& pose : test_poses) {
        hrtf_processor->UpdateSpatialPosition(pose, mic_pose);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time);

    double time_per_update = static_cast<double>(duration.count()) / position_updates;

    // Spatial updates should be very fast (< 100μs each for 90Hz VR)
    EXPECT_LT(time_per_update, 100.0) << "Spatial audio updates too slow for VR";

    LOG_INFO("Spatial update performance - {:.2f}μs per update", time_per_update);
}

// ============================================================================
// MEMORY AND RESOURCE TESTS
// ============================================================================

TEST_F(AudioPerformanceTest, MemoryUsageStability) {
    LOG_INFO("Testing memory usage stability");

#ifdef _WIN32
    HANDLE process = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS_EX initial_mem, peak_mem, final_mem;

    GetProcessMemoryInfo(process, (PROCESS_MEMORY_COUNTERS*)&initial_mem, sizeof(initial_mem));
    peak_mem = initial_mem;

    EXPECT_TRUE(audio_engine->Start());

    // Run for extended period with monitoring
    const int duration_seconds = 10;
    for (int i = 0; i < duration_seconds * 10; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        PROCESS_MEMORY_COUNTERS_EX current_mem;
        GetProcessMemoryInfo(process, (PROCESS_MEMORY_COUNTERS*)&current_mem, sizeof(current_mem));

        if (current_mem.WorkingSetSize > peak_mem.WorkingSetSize) {
            peak_mem = current_mem;
        }
    }

    audio_engine->Stop();

    GetProcessMemoryInfo(process, (PROCESS_MEMORY_COUNTERS*)&final_mem, sizeof(final_mem));

    uint64_t initial_mb = initial_mem.WorkingSetSize / (1024 * 1024);
    uint64_t peak_mb = peak_mem.WorkingSetSize / (1024 * 1024);
    uint64_t final_mb = final_mem.WorkingSetSize / (1024 * 1024);

    uint64_t memory_growth = final_mb - initial_mb;
    uint64_t peak_usage = peak_mb - initial_mb;

    EXPECT_LT(memory_growth, 10) << "Excessive memory growth detected";
    EXPECT_LT(peak_usage, 100) << "Peak memory usage too high";

    LOG_INFO("Memory usage - Initial: {}MB, Peak: {}MB, Final: {}MB, Growth: {}MB",
             initial_mb, peak_mb, final_mb, memory_growth);
#else
    LOG_INFO("Memory usage test not available on non-Windows platforms");
#endif
}

TEST_F(AudioPerformanceTest, CPUUsageUnderLoad) {
    LOG_INFO("Testing CPU usage under load");

    EXPECT_TRUE(audio_engine->Start());

    // Simulate VR workload with frequent spatial updates
    const int updates_per_second = 90; // VR refresh rate
    const int test_duration_seconds = 5;
    const int total_updates = updates_per_second * test_duration_seconds;

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < total_updates; i++) {
        // Simulate head movement
        VRPose head_pose;
        head_pose.position.x = std::sin(i * 0.1f) * 0.5f;
        head_pose.position.y = 1.8f + std::sin(i * 0.05f) * 0.1f;
        head_pose.position.z = std::cos(i * 0.1f) * 0.5f;
        head_pose.orientation = {0.0f, std::sin(i * 0.02f) * 0.1f, 0.0f, 1.0f};

        VRPose mic_pose = {{0.0f, 1.2f, -1.0f}, {0.0f, 0.0f, 0.0f, 1.0f}};

        hrtf_processor->UpdateSpatialPosition(head_pose, mic_pose);

        // Maintain update rate
        auto target_time = start_time + std::chrono::microseconds(
            (i + 1) * 1000000 / updates_per_second);

        std::this_thread::sleep_until(target_time);
    }

    audio_engine->Stop();

    auto end_time = std::chrono::high_resolution_clock::now();
    auto actual_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);

    // Check timing accuracy
    int expected_duration_ms = test_duration_seconds * 1000;
    int timing_error_ms = std::abs(static_cast<int>(actual_duration.count()) - expected_duration_ms);

    EXPECT_LT(timing_error_ms, 100) << "CPU load affected timing accuracy";

    LOG_INFO("CPU load test completed - Duration: {}ms (expected: {}ms, error: {}ms)",
             actual_duration.count(), expected_duration_ms, timing_error_ms);
}

// ============================================================================
// STRESS TESTS
// ============================================================================

TEST_F(AudioPerformanceTest, ExtendedStressTest) {
    LOG_INFO("Running extended stress test");

    EXPECT_TRUE(audio_engine->Start());

    const int stress_duration_minutes = 2; // Shortened for CI
    const int check_interval_seconds = 10;
    const int total_checks = (stress_duration_minutes * 60) / check_interval_seconds;

    std::vector<AudioEngine::Stats> stats_history;

    for (int check = 0; check < total_checks; check++) {
        std::this_thread::sleep_for(std::chrono::seconds(check_interval_seconds));

        auto stats = audio_engine->GetStats();
        stats_history.push_back(stats);

        // Continuous validation
        EXPECT_EQ(stats.underruns, 0) << "Audio underruns detected during stress test";
        EXPECT_GT(stats.framesProcessed, 0) << "Audio processing stopped";

        LOG_DEBUG("Stress test check {}/{} - Frames: {}, Latency: {:.2f}ms",
                 check + 1, total_checks, stats.framesProcessed, stats.inputLatency);
    }

    audio_engine->Stop();

    // Analyze stability over time
    bool latency_stable = true;
    double first_latency = stats_history[0].inputLatency;

    for (const auto& stats : stats_history) {
        if (std::abs(stats.inputLatency - first_latency) > 10.0) {
            latency_stable = false;
            break;
        }
    }

    EXPECT_TRUE(latency_stable) << "Latency became unstable during stress test";

    LOG_INFO("Extended stress test completed successfully - {} checks over {} minutes",
             total_checks, stress_duration_minutes);
}

} // namespace testing
} // namespace vrb

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    std::cout << "\n";
    std::cout << "=======================================================\n";
    std::cout << "      WINDOWS AUDIO PERFORMANCE VALIDATION\n";
    std::cout << "=======================================================\n";
    std::cout << "\n";

    return RUN_ALL_TESTS();
}