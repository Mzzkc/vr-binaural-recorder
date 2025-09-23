// test_main.cpp - Test suite main entry point
#include <gtest/gtest.h>
#include "logger.h"

int main(int argc, char** argv) {
    // Initialize logger for tests
    vrb::Logger::Initialize("debug", "./test_logs");
    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// test_audio_engine.cpp - Audio Engine Tests

#include <gtest/gtest.h>
#include "audio_engine.h"
#include "config.h"
#include "hrtf_processor.h"
#include <thread>
#include <chrono>

namespace vrb {

class AudioEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        config = std::make_unique<Config>("test_config.json");
        hrtf = std::make_unique<HRTFProcessor>();
        hrtf->Initialize("./test_hrtf_data");
        engine = std::make_unique<AudioEngine>();
    }

    void TearDown() override {
        engine.reset();
        hrtf.reset();
        config.reset();
    }

    std::unique_ptr<Config> config;
    std::unique_ptr<HRTFProcessor> hrtf;
    std::unique_ptr<AudioEngine> engine;
};

TEST_F(AudioEngineTest, Initialization) {
    EXPECT_TRUE(engine->Initialize(*config, hrtf.get()));
}

TEST_F(AudioEngineTest, DeviceEnumeration) {
    ASSERT_TRUE(engine->Initialize(*config, hrtf.get()));
    
    auto devices = engine->GetInputDevices();
    EXPECT_GT(devices.size(), 0) << "No input devices found";
    
    // Verify device properties
    for (const auto& device : devices) {
        EXPECT_GE(device.index, 0);
        EXPECT_FALSE(device.name.empty());
        EXPECT_GT(device.maxInputChannels, 0);
        EXPECT_GT(device.defaultSampleRate, 0);
    }
}

TEST_F(AudioEngineTest, DeviceSelection) {
    ASSERT_TRUE(engine->Initialize(*config, hrtf.get()));
    
    auto devices = engine->GetInputDevices();
    if (!devices.empty()) {
        EXPECT_TRUE(engine->SelectInputDevice(devices[0].index));
        EXPECT_EQ(engine->GetCurrentInputDevice(), devices[0].name);
    }
}

TEST_F(AudioEngineTest, StartStop) {
    ASSERT_TRUE(engine->Initialize(*config, hrtf.get()));
    
    EXPECT_TRUE(engine->Start());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto stats = engine->GetStats();
    EXPECT_GE(stats.framesProcessed, 0);
    
    engine->Stop();
}

TEST_F(AudioEngineTest, InvalidDeviceHandling) {
    ASSERT_TRUE(engine->Initialize(*config, hrtf.get()));
    
    // Try to select invalid device
    EXPECT_FALSE(engine->SelectInputDevice(-1));
    EXPECT_FALSE(engine->SelectInputDevice(9999));
}

TEST_F(AudioEngineTest, ConfigurationUpdate) {
    ASSERT_TRUE(engine->Initialize(*config, hrtf.get()));
    
    // Modify config and update
    engine->UpdateConfiguration(*config);
    
    // Verify engine continues to function
    EXPECT_TRUE(engine->Start());
    engine->Stop();
}

} // namespace vrb

// test_hrtf_processor.cpp - HRTF Processor Tests

#include <gtest/gtest.h>
#include "hrtf_processor.h"
#include <vector>
#include <cmath>

namespace vrb {

class HRTFProcessorTest : public ::testing::Test {
protected:
    void SetUp() override {
        processor = std::make_unique<HRTFProcessor>();
    }

    std::unique_ptr<HRTFProcessor> processor;
};

TEST_F(HRTFProcessorTest, Initialization) {
    EXPECT_TRUE(processor->Initialize("./test_hrtf_data"));
}

TEST_F(HRTFProcessorTest, ProcessMonoInput) {
    ASSERT_TRUE(processor->Initialize("./test_hrtf_data"));
    
    const size_t frames = 1024;
    std::vector<float> input(frames, 0.5f);  // Constant input signal
    std::vector<float> output(frames * 2, 0.0f);  // Stereo output
    
    processor->Process(input.data(), output.data(), frames, 1);
    
    // Verify output is not silence
    float sum = 0.0f;
    for (float sample : output) {
        sum += std::abs(sample);
    }
    EXPECT_GT(sum, 0.0f);
}

TEST_F(HRTFProcessorTest, ProcessStereoInput) {
    ASSERT_TRUE(processor->Initialize("./test_hrtf_data"));
    
    const size_t frames = 512;
    std::vector<float> input(frames * 2);
    std::vector<float> output(frames * 2, 0.0f);
    
    // Generate stereo test signal
    for (size_t i = 0; i < frames; ++i) {
        input[i * 2] = std::sin(2.0f * M_PI * 440.0f * i / 48000.0f);  // Left
        input[i * 2 + 1] = std::sin(2.0f * M_PI * 880.0f * i / 48000.0f);  // Right
    }
    
    processor->Process(input.data(), output.data(), frames, 2);
    
    // Verify output contains signal
    float energy = 0.0f;
    for (float sample : output) {
        energy += sample * sample;
    }
    EXPECT_GT(energy, 0.0f);
}

TEST_F(HRTFProcessorTest, SpatialPositionUpdate) {
    ASSERT_TRUE(processor->Initialize("./test_hrtf_data"));
    
    VRPose headPose;
    headPose.position = {0.0f, 1.8f, 0.0f};
    headPose.orientation = {0.0f, 0.0f, 0.0f, 1.0f};
    
    VRPose micPose;
    micPose.position = {0.0f, 1.2f, -1.0f};
    micPose.orientation = {0.0f, 0.0f, 0.0f, 1.0f};
    
    processor->UpdateSpatialPosition(headPose, micPose);
    
    auto stats = processor->GetStats();
    EXPECT_NE(stats.distance, 0.0f);
}

TEST_F(HRTFProcessorTest, Reset) {
    ASSERT_TRUE(processor->Initialize("./test_hrtf_data"));
    
    processor->Reset();
    
    auto stats = processor->GetStats();
    EXPECT_EQ(stats.azimuth, 0.0f);
    EXPECT_EQ(stats.elevation, 0.0f);
    EXPECT_EQ(stats.distance, 1.0f);
}

TEST_F(HRTFProcessorTest, DistanceAttenuation) {
    ASSERT_TRUE(processor->Initialize("./test_hrtf_data"));
    
    const size_t frames = 256;
    std::vector<float> input(frames, 1.0f);
    std::vector<float> outputNear(frames * 2);
    std::vector<float> outputFar(frames * 2);
    
    // Process with near position
    VRPose headNear;
    headNear.position = {0.0f, 1.2f, -0.5f};
    VRPose mic;
    mic.position = {0.0f, 1.2f, -1.0f};
    
    processor->UpdateSpatialPosition(headNear, mic);
    processor->Process(input.data(), outputNear.data(), frames, 1);
    
    // Process with far position
    VRPose headFar;
    headFar.position = {0.0f, 1.2f, -5.0f};
    
    processor->UpdateSpatialPosition(headFar, mic);
    processor->Process(input.data(), outputFar.data(), frames, 1);
    
    // Calculate RMS
    float rmsNear = 0.0f, rmsFar = 0.0f;
    for (size_t i = 0; i < frames * 2; ++i) {
        rmsNear += outputNear[i] * outputNear[i];
        rmsFar += outputFar[i] * outputFar[i];
    }
    rmsNear = std::sqrt(rmsNear / (frames * 2));
    rmsFar = std::sqrt(rmsFar / (frames * 2));
    
    // Near should be louder than far
    EXPECT_GT(rmsNear, rmsFar);
}

} // namespace vrb

// test_ring_buffer.cpp - Ring Buffer Tests

#include <gtest/gtest.h>
#include "ring_buffer.h"
#include <thread>
#include <vector>

namespace vrb {

TEST(RingBufferTest, BasicWriteRead) {
    RingBuffer<float> buffer(1024);
    
    std::vector<float> writeData(100, 1.0f);
    std::vector<float> readData(100, 0.0f);
    
    size_t written = buffer.write(writeData.data(), 100);
    EXPECT_EQ(written, 100);
    
    size_t read = buffer.read(readData.data(), 100);
    EXPECT_EQ(read, 100);
    
    for (float value : readData) {
        EXPECT_EQ(value, 1.0f);
    }
}

TEST(RingBufferTest, Overflow) {
    RingBuffer<float> buffer(100);
    size_t actualCapacity = buffer.capacity();  // Will be 128 (next power of 2)

    std::vector<float> data(actualCapacity + 50, 2.0f);

    size_t written = buffer.write(data.data(), actualCapacity + 50);
    EXPECT_LE(written, actualCapacity);  // Should not write more than actual capacity
}

TEST(RingBufferTest, Underflow) {
    RingBuffer<float> buffer(100);
    
    std::vector<float> readData(50, 0.0f);
    
    size_t read = buffer.read(readData.data(), 50);
    EXPECT_EQ(read, 0);  // Nothing to read from empty buffer
}

TEST(RingBufferTest, ConcurrentAccess) {
    RingBuffer<int> buffer(10000);
    const int numSamples = 100000;
    
    std::thread writer([&buffer, numSamples]() {
        for (int i = 0; i < numSamples; ++i) {
            while (buffer.write(&i, 1) == 0) {
                std::this_thread::yield();
            }
        }
    });
    
    std::thread reader([&buffer, numSamples]() {
        int expected = 0;
        int value;
        while (expected < numSamples) {
            if (buffer.read(&value, 1) == 1) {
                EXPECT_EQ(value, expected);
                expected++;
            } else {
                std::this_thread::yield();
            }
        }
    });
    
    writer.join();
    reader.join();
}

TEST(RingBufferTest, Reset) {
    RingBuffer<float> buffer(100);

    std::vector<float> data(50, 3.0f);
    buffer.write(data.data(), 50);

    EXPECT_EQ(buffer.available(), 50);

    buffer.reset();

    EXPECT_EQ(buffer.available(), 0);
    EXPECT_EQ(buffer.free(), buffer.capacity() - 1);  // Correctly expects capacity - 1
}

} // namespace vrb

// test_config.cpp - Configuration Tests

#include <gtest/gtest.h>
#include "config.h"
#include <fstream>
#include <filesystem>

namespace vrb {

class ConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        testConfigPath = "test_config_temp.json";
        
        // Create test config file
        Json::Value root;
        root["audio"]["sampleRate"] = 44100;
        root["audio"]["bufferSize"] = 256;
        root["audio"]["preferredInputDevice"] = "Test Device";
        root["audio"]["virtualOutputName"] = "Test Output";
        root["hrtf"]["dataPath"] = "./test_hrtf";
        root["logging"]["level"] = "debug";
        root["logging"]["path"] = "./test_logs";
        root["vr"]["overlayScale"] = 0.75;
        
        std::ofstream file(testConfigPath);
        Json::StreamWriterBuilder builder;
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(root, &file);
        file.close();
    }

    void TearDown() override {
        // Clean up test file
        std::filesystem::remove(testConfigPath);
    }

    std::string testConfigPath;
};

TEST_F(ConfigTest, LoadConfig) {
    Config config(testConfigPath);
    
    EXPECT_EQ(config.GetSampleRate(), 44100);
    EXPECT_EQ(config.GetBufferSize(), 256);
    EXPECT_EQ(config.GetPreferredInputDevice(), "Test Device");
    EXPECT_EQ(config.GetVirtualOutputName(), "Test Output");
    EXPECT_EQ(config.GetHRTFDataPath(), "./test_hrtf");
    EXPECT_EQ(config.GetLogLevel(), "debug");
    EXPECT_EQ(config.GetLogPath(), "./test_logs");
    EXPECT_FLOAT_EQ(config.GetOverlayScale(), 0.75f);
}

TEST_F(ConfigTest, DefaultConfig) {
    // Test with non-existent file
    Config config("non_existent_config.json");
    
    // Should use defaults
    EXPECT_EQ(config.GetSampleRate(), 48000);
    EXPECT_EQ(config.GetBufferSize(), 128);
    
    // Clean up created default file
    std::filesystem::remove("non_existent_config.json");
}

TEST_F(ConfigTest, ConfigReload) {
    Config config(testConfigPath);

    EXPECT_EQ(config.GetSampleRate(), 44100);

    // Give watcher thread time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Modify file
    Json::Value root;
    root["audio"]["sampleRate"] = 96000;
    root["audio"]["bufferSize"] = 256;
    root["audio"]["preferredInputDevice"] = "";
    root["audio"]["virtualOutputName"] = "Modified Output";
    root["hrtf"]["dataPath"] = "./test_hrtf";
    root["logging"]["level"] = "info";
    root["logging"]["path"] = "./test_logs";
    root["vr"]["overlayScale"] = 0.5;

    std::ofstream file(testConfigPath);
    Json::StreamWriterBuilder builder;
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(root, &file);
    file.flush();
    file.close();

    // Manually trigger file timestamp update to ensure change detection
    auto now = std::chrono::system_clock::now();
    auto file_time = std::chrono::time_point_cast<std::filesystem::file_time_type::duration>(
        now - std::chrono::system_clock::now() + std::filesystem::file_time_type::clock::now());
    std::filesystem::last_write_time(testConfigPath, file_time);

    // Wait for the watcher thread polling cycle (500ms) plus buffer time
    std::this_thread::sleep_for(std::chrono::milliseconds(700));

    EXPECT_TRUE(config.HasChanged());
    EXPECT_TRUE(config.Reload());
    EXPECT_EQ(config.GetSampleRate(), 96000);
    EXPECT_EQ(config.GetVirtualOutputName(), "Modified Output");
}

TEST_F(ConfigTest, InvalidConfig) {
    // Create invalid JSON
    std::ofstream file("invalid_config.json");
    file << "{ invalid json content";
    file.close();
    
    Config config("invalid_config.json");
    
    // Should handle error gracefully and use defaults
    EXPECT_EQ(config.GetSampleRate(), 48000);
    
    std::filesystem::remove("invalid_config.json");
}

} // namespace vrb