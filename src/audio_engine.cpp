// audio_engine.h - Core Audio Processing Module
// Handles audio I/O and routing through HRTF processor

#pragma once

#include <atomic>
#include <vector>
#include <string>
#include <memory>
#include <portaudio.h>

// Platform-specific includes for low-latency audio
#ifdef _WIN32
#include <pa_win_wasapi.h>
#include <pa_asio.h>
#endif

#include "config.h"
#include "hrtf_processor.h"
#include "ring_buffer.h"

namespace vrb {

/**
 * @brief Core audio engine handling input/output and spatial processing
 * 
 * This class manages the audio pipeline:
 * 1. Captures audio from selected microphone
 * 2. Processes through HRTF for spatial positioning
 * 3. Outputs binaural audio to virtual audio device
 */
class AudioEngine {
public:
    struct DeviceInfo {
        int index;
        std::string name;
        int maxInputChannels;
        int maxOutputChannels;
        double defaultSampleRate;
    };

    AudioEngine();
    ~AudioEngine();

    /**
     * @brief Initialize audio engine with configuration
     * @param config Application configuration
     * @param hrtf Pointer to HRTF processor
     * @return true if initialization successful
     */
    bool Initialize(const Config& config, HRTFProcessor* hrtf);

    /**
     * @brief Start audio processing
     * @return true if started successfully
     */
    bool Start();

    /**
     * @brief Stop audio processing
     */
    void Stop();

    /**
     * @brief Get list of available input devices
     * @return Vector of available input devices
     */
    std::vector<DeviceInfo> GetInputDevices() const;

    /**
     * @brief Select input device by index
     * @param deviceIndex PortAudio device index
     * @return true if device selected successfully
     */
    bool SelectInputDevice(int deviceIndex);

    /**
     * @brief Get current input device name
     * @return Name of current input device
     */
    std::string GetCurrentInputDevice() const;

    /**
     * @brief Update configuration (hot reload)
     * @param config New configuration
     */
    void UpdateConfiguration(const Config& config);

    /**
     * @brief Get current audio statistics
     */
    struct AudioStats {
        float cpuLoad;
        int64_t framesProcessed;
        int underruns;
        int overruns;
    };
    AudioStats GetStats() const;

private:
    /**
     * @brief PortAudio callback for audio processing
     */
    static int AudioCallback(const void* inputBuffer,
                            void* outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void* userData);

    /**
     * @brief Process audio buffer
     */
    int ProcessAudio(const float* input, float* output, unsigned long frames,
                     PaStreamCallbackFlags statusFlags);

    /**
     * @brief Open audio stream with selected devices
     */
    bool OpenStream();

    /**
     * @brief Close current audio stream
     */
    void CloseStream();

    /**
     * @brief Initialize virtual audio output device
     */
    bool InitializeVirtualOutput();

    // State
    std::atomic<bool> m_initialized{false};
    std::atomic<bool> m_running{false};
    
    // Audio configuration
    int m_sampleRate;
    int m_bufferSize;
    int m_inputChannels;
    int m_outputChannels;
    
    // Devices
    int m_inputDevice;
    int m_outputDevice;
    std::string m_inputDeviceName;
    std::string m_virtualOutputName;
    
    // PortAudio
    PaStream* m_stream;
    
    // Processing
    HRTFProcessor* m_hrtf;
    std::unique_ptr<RingBuffer<float>> m_inputBuffer;
    std::unique_ptr<RingBuffer<float>> m_outputBuffer;
    
    // Statistics
    mutable std::atomic<float> m_cpuLoad{0.0f};
    mutable std::atomic<int64_t> m_framesProcessed{0};
    mutable std::atomic<int> m_underruns{0};
    mutable std::atomic<int> m_overruns{0};
};

// audio_engine.cpp - Implementation

#include "audio_engine.h"
#include "logger.h"
#include <algorithm>
#include <cstring>

namespace vrb {

// Constants
constexpr int DEFAULT_SAMPLE_RATE = 48000;
constexpr int DEFAULT_BUFFER_SIZE = 128;  // ~2.7ms at 48kHz
constexpr int RING_BUFFER_SIZE = 4096;
constexpr const char* VIRTUAL_OUTPUT_NAME = "VR Binaural Recorder";

AudioEngine::AudioEngine() 
    : m_stream(nullptr)
    , m_hrtf(nullptr)
    , m_sampleRate(DEFAULT_SAMPLE_RATE)
    , m_bufferSize(DEFAULT_BUFFER_SIZE)
    , m_inputChannels(1)
    , m_outputChannels(2)
    , m_inputDevice(paNoDevice)
    , m_outputDevice(paNoDevice) {
}

AudioEngine::~AudioEngine() {
    Stop();
    if (m_initialized) {
        Pa_Terminate();
        LOG_INFO("PortAudio terminated");
    }
}

bool AudioEngine::Initialize(const Config& config, HRTFProcessor* hrtf) {
    if (m_initialized) {
        LOG_WARN("Audio engine already initialized");
        return true;
    }

    m_hrtf = hrtf;
    if (!m_hrtf) {
        LOG_ERROR("HRTF processor is null");
        return false;
    }

    // Initialize PortAudio
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        LOG_ERROR("Failed to initialize PortAudio: {}", Pa_GetErrorText(err));
        return false;
    }

    // Load configuration
    m_sampleRate = config.GetSampleRate();
    m_bufferSize = config.GetBufferSize();
    m_virtualOutputName = config.GetVirtualOutputName();

    // Create ring buffers for thread-safe audio processing
    m_inputBuffer = std::make_unique<RingBuffer<float>>(RING_BUFFER_SIZE * m_inputChannels);
    m_outputBuffer = std::make_unique<RingBuffer<float>>(RING_BUFFER_SIZE * m_outputChannels);

    // Find and set default input device
    auto devices = GetInputDevices();
    if (!devices.empty()) {
        // Try to find preferred device from config
        std::string preferredDevice = config.GetPreferredInputDevice();
        auto it = std::find_if(devices.begin(), devices.end(),
            [&](const DeviceInfo& d) { return d.name == preferredDevice; });
        
        if (it != devices.end()) {
            m_inputDevice = it->index;
            m_inputDeviceName = it->name;
        } else {
            // Use first available device
            m_inputDevice = devices[0].index;
            m_inputDeviceName = devices[0].name;
        }
        LOG_INFO("Selected input device: {} (index: {})", m_inputDeviceName, m_inputDevice);
    } else {
        LOG_ERROR("No input devices found");
        return false;
    }

    // Initialize virtual output
    if (!InitializeVirtualOutput()) {
        LOG_ERROR("Failed to initialize virtual audio output");
        Pa_Terminate();
        return false;
    }

    m_initialized = true;
    LOG_INFO("Audio engine initialized - SR: {}Hz, Buffer: {} samples", m_sampleRate, m_bufferSize);
    return true;
}

bool AudioEngine::Start() {
    if (!m_initialized) {
        LOG_ERROR("Audio engine not initialized");
        return false;
    }

    if (m_running) {
        LOG_WARN("Audio engine already running");
        return true;
    }

    if (!OpenStream()) {
        LOG_ERROR("Failed to open audio stream");
        return false;
    }

    PaError err = Pa_StartStream(m_stream);
    if (err != paNoError) {
        LOG_ERROR("Failed to start stream: {}", Pa_GetErrorText(err));
        CloseStream();
        return false;
    }

    m_running = true;
    LOG_INFO("Audio engine started");
    return true;
}

void AudioEngine::Stop() {
    if (!m_running) {
        return;
    }

    m_running = false;

    if (m_stream) {
        Pa_StopStream(m_stream);
        CloseStream();
    }

    LOG_INFO("Audio engine stopped - Frames processed: {}, Underruns: {}, Overruns: {}",
             m_framesProcessed.load(), m_underruns.load(), m_overruns.load());
}

std::vector<AudioEngine::DeviceInfo> AudioEngine::GetInputDevices() const {
    std::vector<DeviceInfo> devices;
    
    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        LOG_ERROR("Failed to get device count: {}", Pa_GetErrorText(numDevices));
        return devices;
    }

    for (int i = 0; i < numDevices; i++) {
        const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
        if (info && info->maxInputChannels > 0) {
            DeviceInfo device;
            device.index = i;
            device.name = info->name;
            device.maxInputChannels = info->maxInputChannels;
            device.maxOutputChannels = info->maxOutputChannels;
            device.defaultSampleRate = info->defaultSampleRate;
            devices.push_back(device);
        }
    }

    LOG_DEBUG("Found {} input devices", devices.size());
    return devices;
}

bool AudioEngine::SelectInputDevice(int deviceIndex) {
    if (!m_initialized) {
        LOG_ERROR("Audio engine not initialized");
        return false;
    }

    const PaDeviceInfo* info = Pa_GetDeviceInfo(deviceIndex);
    if (!info || info->maxInputChannels <= 0) {
        LOG_ERROR("Invalid input device index: {}", deviceIndex);
        return false;
    }

    bool wasRunning = m_running;
    if (wasRunning) {
        Stop();
    }

    m_inputDevice = deviceIndex;
    m_inputDeviceName = info->name;
    m_inputChannels = std::min(2, info->maxInputChannels);  // Support mono or stereo

    LOG_INFO("Selected input device: {} (channels: {})", m_inputDeviceName, m_inputChannels);

    if (wasRunning) {
        return Start();
    }

    return true;
}

std::string AudioEngine::GetCurrentInputDevice() const {
    return m_inputDeviceName;
}

void AudioEngine::UpdateConfiguration(const Config& config) {
    bool needsRestart = false;

    if (config.GetSampleRate() != m_sampleRate || 
        config.GetBufferSize() != m_bufferSize) {
        needsRestart = true;
    }

    if (needsRestart && m_running) {
        LOG_INFO("Configuration change requires restart");
        Stop();
        m_sampleRate = config.GetSampleRate();
        m_bufferSize = config.GetBufferSize();
        Start();
    }
}

AudioEngine::AudioStats AudioEngine::GetStats() const {
    AudioStats stats;
    stats.cpuLoad = m_cpuLoad.load();
    stats.framesProcessed = m_framesProcessed.load();
    stats.underruns = m_underruns.load();
    stats.overruns = m_overruns.load();
    return stats;
}

bool AudioEngine::OpenStream() {
    PaStreamParameters inputParams;
    inputParams.device = m_inputDevice;
    inputParams.channelCount = m_inputChannels;
    inputParams.sampleFormat = paFloat32;
    inputParams.suggestedLatency = Pa_GetDeviceInfo(m_inputDevice)->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    PaStreamParameters outputParams;
    outputParams.device = m_outputDevice;
    outputParams.channelCount = m_outputChannels;
    outputParams.sampleFormat = paFloat32;
    outputParams.suggestedLatency = Pa_GetDeviceInfo(m_outputDevice)->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;

    PaError err = Pa_OpenStream(
        &m_stream,
        &inputParams,
        &outputParams,
        m_sampleRate,
        m_bufferSize,
        paClipOff,
        AudioCallback,
        this
    );

    if (err != paNoError) {
        LOG_ERROR("Failed to open stream: {}", Pa_GetErrorText(err));
        return false;
    }

    LOG_INFO("Audio stream opened successfully");
    return true;
}

void AudioEngine::CloseStream() {
    if (m_stream) {
        Pa_CloseStream(m_stream);
        m_stream = nullptr;
    }
}

bool AudioEngine::InitializeVirtualOutput() {
    // Find virtual audio cable output device
    int numDevices = Pa_GetDeviceCount();
    
    for (int i = 0; i < numDevices; i++) {
        const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
        if (info && info->maxOutputChannels >= 2) {
            std::string name(info->name);
            // Look for VB-Cable or other virtual audio devices
            if (name.find("VB-Cable") != std::string::npos ||
                name.find("Virtual Cable") != std::string::npos ||
                name.find(m_virtualOutputName) != std::string::npos) {
                m_outputDevice = i;
                LOG_INFO("Found virtual output device: {} (index: {})", name, i);
                return true;
            }
        }
    }

    // If no virtual cable found, use default output (for testing)
    m_outputDevice = Pa_GetDefaultOutputDevice();
    if (m_outputDevice == paNoDevice) {
        LOG_ERROR("No output device available");
        return false;
    }

    const PaDeviceInfo* info = Pa_GetDeviceInfo(m_outputDevice);
    LOG_WARN("Virtual audio cable not found, using default output: {}", info->name);
    return true;
}

int AudioEngine::AudioCallback(const void* inputBuffer, void* outputBuffer,
                               unsigned long framesPerBuffer,
                               const PaStreamCallbackTimeInfo* timeInfo,
                               PaStreamCallbackFlags statusFlags,
                               void* userData) {
    AudioEngine* engine = static_cast<AudioEngine*>(userData);
    return engine->ProcessAudio(static_cast<const float*>(inputBuffer),
                                static_cast<float*>(outputBuffer),
                                framesPerBuffer, statusFlags);
}

int AudioEngine::ProcessAudio(const float* input, float* output, unsigned long frames,
                              PaStreamCallbackFlags statusFlags) {
    if (!m_running) {
        // Output silence if not running
        std::memset(output, 0, frames * m_outputChannels * sizeof(float));
        return paContinue;
    }

    // Check for underrun/overrun
    if (statusFlags & paInputUnderflow) {
        m_underruns++;
        LOG_DEBUG("Input underflow detected");
    }
    if (statusFlags & paOutputOverflow) {
        m_overruns++;
        LOG_DEBUG("Output overflow detected");
    }

    // Process through HRTF
    if (m_hrtf && input && output) {
        m_hrtf->Process(input, output, frames, m_inputChannels);
        m_framesProcessed += frames;
    } else {
        // Pass-through or silence
        if (input && m_inputChannels == m_outputChannels) {
            std::memcpy(output, input, frames * m_outputChannels * sizeof(float));
        } else {
            std::memset(output, 0, frames * m_outputChannels * sizeof(float));
        }
    }

    // Update CPU load
    const PaStreamInfo* streamInfo = Pa_GetStreamInfo(m_stream);
    if (streamInfo) {
        m_cpuLoad = streamInfo->cpuLoad;
    }

    return paContinue;
}

} // namespace vrb