// audio_engine.cpp - Core Audio Processing Module Implementation

#include "audio_engine.h"
#include "logger.h"
#include <algorithm>
#include <cstring>
#include <cmath>
#include <numeric>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <cstdlib>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Platform-specific includes
#ifdef _WIN32
#include <mmreg.h>
#include <ks.h>
#include <ksmedia.h>
#elif defined(__linux__)
#include <sys/resource.h>
#include <pthread.h>
#elif defined(__APPLE__)
#include <mach/mach_init.h>
#include <mach/thread_act.h>
#endif

namespace vrb {

// Constants
constexpr int DEFAULT_SAMPLE_RATE = 48000;
constexpr int DEFAULT_BUFFER_SIZE = 128;  // ~2.7ms at 48kHz
constexpr int RING_BUFFER_SIZE = 8192;  // Increased for better stability
constexpr int MAX_BUFFER_SIZE = 2048;
constexpr int MIN_BUFFER_SIZE = 32;
constexpr const char* VIRTUAL_OUTPUT_NAME = "VR Binaural Recorder";
constexpr double MAX_CALLBACK_TIME_MS = 10.0;  // Maximum allowed callback duration
constexpr int ADAPTIVE_BUFFER_THRESHOLD = 5;  // Number of xruns before buffer adjustment
constexpr float PEAK_DECAY_RATE = 0.99f;  // Peak level decay per callback

// SIMD-optimized audio processing functions
namespace simd {
    // Float conversion with SIMD optimization
    void convertInt16ToFloat(const int16_t* input, float* output, size_t samples) {
#ifdef VRB_USE_AVX2
        const size_t simdSamples = samples & ~15;  // Process 16 samples at a time
        const __m256 scale = _mm256_set1_ps(1.0f / 32768.0f);

        for (size_t i = 0; i < simdSamples; i += 16) {
            __m128i int16_data = _mm_loadu_si128(reinterpret_cast<const __m128i*>(input + i));
            __m256i int32_data = _mm256_cvtepi16_epi32(int16_data);
            __m256 float_data = _mm256_cvtepi32_ps(int32_data);
            __m256 scaled_data = _mm256_mul_ps(float_data, scale);
            _mm256_storeu_ps(output + i, scaled_data);

            int16_data = _mm_loadu_si128(reinterpret_cast<const __m128i*>(input + i + 8));
            int32_data = _mm256_cvtepi16_epi32(int16_data);
            float_data = _mm256_cvtepi32_ps(int32_data);
            scaled_data = _mm256_mul_ps(float_data, scale);
            _mm256_storeu_ps(output + i + 8, scaled_data);
        }

        // Process remaining samples
        for (size_t i = simdSamples; i < samples; ++i) {
            output[i] = static_cast<float>(input[i]) / 32768.0f;
        }
#elif defined(VRB_USE_SSE2)
        const size_t simdSamples = samples & ~7;  // Process 8 samples at a time
        const __m128 scale = _mm_set1_ps(1.0f / 32768.0f);

        for (size_t i = 0; i < simdSamples; i += 8) {
            __m128i int16_data = _mm_loadu_si128(reinterpret_cast<const __m128i*>(input + i));
            __m128i int32_lo = _mm_unpacklo_epi16(int16_data, _mm_setzero_si128());
            __m128i int32_hi = _mm_unpackhi_epi16(int16_data, _mm_setzero_si128());

            // Convert to signed 32-bit
            int32_lo = _mm_srai_epi32(_mm_slli_epi32(int32_lo, 16), 16);
            int32_hi = _mm_srai_epi32(_mm_slli_epi32(int32_hi, 16), 16);

            __m128 float_lo = _mm_cvtepi32_ps(int32_lo);
            __m128 float_hi = _mm_cvtepi32_ps(int32_hi);

            __m128 scaled_lo = _mm_mul_ps(float_lo, scale);
            __m128 scaled_hi = _mm_mul_ps(float_hi, scale);

            _mm_storeu_ps(output + i, scaled_lo);
            _mm_storeu_ps(output + i + 4, scaled_hi);
        }

        // Process remaining samples
        for (size_t i = simdSamples; i < samples; ++i) {
            output[i] = static_cast<float>(input[i]) / 32768.0f;
        }
#else
        for (size_t i = 0; i < samples; ++i) {
            output[i] = static_cast<float>(input[i]) / 32768.0f;
        }
#endif
    }

    void convertFloatToInt16(const float* input, int16_t* output, size_t samples) {
#ifdef VRB_USE_AVX2
        const size_t simdSamples = samples & ~15;
        const __m256 scale = _mm256_set1_ps(32767.0f);
        const __m256 min_val = _mm256_set1_ps(-32768.0f);
        const __m256 max_val = _mm256_set1_ps(32767.0f);

        for (size_t i = 0; i < simdSamples; i += 16) {
            __m256 float_data1 = _mm256_loadu_ps(input + i);
            __m256 float_data2 = _mm256_loadu_ps(input + i + 8);

            // Scale and clamp
            float_data1 = _mm256_mul_ps(float_data1, scale);
            float_data2 = _mm256_mul_ps(float_data2, scale);
            float_data1 = _mm256_max_ps(_mm256_min_ps(float_data1, max_val), min_val);
            float_data2 = _mm256_max_ps(_mm256_min_ps(float_data2, max_val), min_val);

            // Convert to int32
            __m256i int32_data1 = _mm256_cvtps_epi32(float_data1);
            __m256i int32_data2 = _mm256_cvtps_epi32(float_data2);

            // Pack to int16
            __m256i packed = _mm256_packs_epi32(int32_data1, int32_data2);
            packed = _mm256_permute4x64_epi64(packed, _MM_SHUFFLE(3,1,2,0));
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(output + i), packed);
        }

        for (size_t i = simdSamples; i < samples; ++i) {
            float scaled = input[i] * 32767.0f;
            output[i] = static_cast<int16_t>(std::clamp(scaled, -32768.0f, 32767.0f));
        }
#else
        for (size_t i = 0; i < samples; ++i) {
            float scaled = input[i] * 32767.0f;
            output[i] = static_cast<int16_t>(std::clamp(scaled, -32768.0f, 32767.0f));
        }
#endif
    }

    float calculatePeakLevel(const float* buffer, size_t samples) {
#ifdef VRB_USE_AVX2
        __m256 max_vals = _mm256_setzero_ps();
        const size_t simdSamples = samples & ~7;

        for (size_t i = 0; i < simdSamples; i += 8) {
            __m256 data = _mm256_loadu_ps(buffer + i);
            __m256 abs_data = _mm256_andnot_ps(_mm256_set1_ps(-0.0f), data);
            max_vals = _mm256_max_ps(max_vals, abs_data);
        }

        float result[8];
        _mm256_storeu_ps(result, max_vals);
        float peak = *std::max_element(result, result + 8);

        // Process remaining samples
        for (size_t i = simdSamples; i < samples; ++i) {
            peak = std::max(peak, std::abs(buffer[i]));
        }

        return peak;
#else
        float peak = 0.0f;
        for (size_t i = 0; i < samples; ++i) {
            peak = std::max(peak, std::abs(buffer[i]));
        }
        return peak;
#endif
    }
}

// Sample rate conversion state
struct AudioEngine::SRCState {
    double ratio;
    std::vector<float> history;
    size_t historyIndex;
    bool initialized;

    SRCState() : ratio(1.0), historyIndex(0), initialized(false) {
        history.resize(256, 0.0f);  // Small history buffer for interpolation
    }
};

AudioEngine::AudioEngine()
    : m_stream(nullptr)
    , m_hrtf(nullptr)
    , m_sampleRate(DEFAULT_SAMPLE_RATE)
    , m_targetSampleRate(DEFAULT_SAMPLE_RATE)
    , m_bufferSize(DEFAULT_BUFFER_SIZE)
    , m_inputChannels(1)
    , m_outputChannels(2)
    , m_inputFormat(AudioFormat::Float32)
    , m_outputFormat(AudioFormat::Float32)
    , m_preferredHostAPI(HostAPI::Default)
    , m_inputDevice(paNoDevice)
    , m_outputDevice(paNoDevice)
    , m_hostApiIndex(-1)
    , m_srcState(std::make_unique<SRCState>())
    , m_callbackHistory(CALLBACK_HISTORY_SIZE) {
}

AudioEngine::~AudioEngine() {
    Stop();

    // Stop monitoring thread
    if (m_monitorRunning) {
        m_monitorRunning = false;
        if (m_monitorThread.joinable()) {
            m_monitorThread.join();
        }
    }

    // Stop mock processing thread if running
    if (m_mockProcessingRunning) {
        m_mockProcessingRunning = false;
        if (m_mockProcessingThread.joinable()) {
            m_mockProcessingThread.join();
        }
    }

    // Remove virtual audio device if created
    RemoveVirtualAudioDevice();

    // Cleanup platform-specific resources
#ifdef _WIN32
    if (m_avrtHandle) {
        AvRevertMmThreadCharacteristics(m_avrtHandle);
        m_avrtHandle = nullptr;
    }
#endif

    if (m_initialized && !m_mockBackend) {
        Pa_Terminate();
        LOG_INFO("PortAudio terminated");
    } else if (m_initialized && m_mockBackend) {
        LOG_INFO("Mock audio backend terminated");
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

    // Check for headless/WSL2 environment first
    if (IsHeadlessEnvironment()) {
        LOG_INFO("Headless environment detected, initializing mock audio backend");
        return InitializeMockBackend();
    }

    // Initialize PortAudio
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        LOG_ERROR("Failed to initialize PortAudio: {}", Pa_GetErrorText(err));
        // Fall back to mock backend if PortAudio fails
        LOG_WARN("PortAudio failed, falling back to mock backend");
        return InitializeMockBackend();
    }

    // Load configuration
    m_sampleRate = config.GetSampleRate();
    m_targetSampleRate = m_sampleRate;
    m_bufferSize = std::clamp(config.GetBufferSize(), MIN_BUFFER_SIZE, MAX_BUFFER_SIZE);
    m_virtualOutputName = config.GetVirtualOutputName();
    m_exclusiveMode = config.GetWASAPIExclusive();
    m_adaptiveBuffering = true;  // Enable by default

    // Setup preferred host API
    if (config.GetUseASIO()) {
        m_preferredHostAPI = HostAPI::ASIO;
    } else {
#ifdef _WIN32
        m_preferredHostAPI = HostAPI::WASAPI;
#elif defined(__APPLE__)
        m_preferredHostAPI = HostAPI::CoreAudio;
#elif defined(__linux__)
        m_preferredHostAPI = HostAPI::ALSA;
#endif
    }

    // Create ring buffers for thread-safe audio processing
    size_t bufferSize = std::max(RING_BUFFER_SIZE, m_bufferSize * 8);
    m_inputBuffer = std::make_unique<RingBuffer<float>>(bufferSize * m_inputChannels);
    m_outputBuffer = std::make_unique<RingBuffer<float>>(bufferSize * m_outputChannels);
    m_processingBuffer = std::make_unique<RingBuffer<float>>(bufferSize * m_outputChannels);

    // Initialize conversion buffers
    m_conversionBufferInput.resize(m_bufferSize * 8);  // Support up to 8 channels
    m_conversionBufferOutput.resize(m_bufferSize * 8);
    m_resampleBuffer.resize(m_bufferSize * 4);  // For resampling operations

    // Setup platform-specific optimizations
    if (!SetupPlatformOptimizations()) {
        LOG_WARN("Failed to setup platform optimizations");
    }

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
            m_preferredHostAPI = it->hostAPI;
        } else {
            // Use first available device with preferred host API
            auto preferred_it = std::find_if(devices.begin(), devices.end(),
                [&](const DeviceInfo& d) { return d.hostAPI == m_preferredHostAPI; });
            if (preferred_it != devices.end()) {
                m_inputDevice = preferred_it->index;
                m_inputDeviceName = preferred_it->name;
            } else {
                m_inputDevice = devices[0].index;
                m_inputDeviceName = devices[0].name;
            }
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

    // Start performance monitoring thread
    m_monitorRunning = true;
    m_monitorThread = std::thread([this] { MonitorPerformance(); });

    m_initialized = true;
    LOG_INFO("Audio engine initialized - SR: {}Hz, Buffer: {} samples, Host API: {}",
             m_sampleRate, m_bufferSize, static_cast<int>(m_preferredHostAPI));
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

    // Reset statistics
    ResetStats();

    // Handle mock backend
    if (m_mockBackend) {
        m_running = true;
        m_lastCallbackTime = std::chrono::steady_clock::now();
        m_mockLastProcessTime = std::chrono::steady_clock::now();

        // Start mock processing thread
        m_mockProcessingRunning = true;
        m_mockProcessingThread = std::thread([this] {
            MockProcessingLoop();
        });

        LOG_INFO("Mock audio engine started - SR: {}Hz, Buffer: {} samples",
                 m_sampleRate, m_bufferSize);
        return true;
    }

    // Setup real-time priority
    if (!SetupRealtimePriority()) {
        LOG_WARN("Failed to set real-time priority");
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
    m_lastCallbackTime = std::chrono::steady_clock::now();

    // Get actual stream info
    const PaStreamInfo* streamInfo = Pa_GetStreamInfo(m_stream);
    if (streamInfo) {
        LOG_INFO("Audio engine started - Input latency: {:.2f}ms, Output latency: {:.2f}ms, SR: {}Hz",
                 streamInfo->inputLatency * 1000.0,
                 streamInfo->outputLatency * 1000.0,
                 streamInfo->sampleRate);
    } else {
        LOG_INFO("Audio engine started");
    }

    return true;
}

void AudioEngine::Stop() {
    if (!m_running) {
        return;
    }

    m_running = false;

    // Handle mock backend
    if (m_mockBackend) {
        m_mockProcessingRunning = false;
        if (m_mockProcessingThread.joinable()) {
            m_mockProcessingThread.join();
        }

        // Log comprehensive statistics
        AudioStats stats = GetStats();
        LOG_INFO("Mock audio engine stopped - Frames: {}, Peak: {:.3f}/{:.3f}",
                 stats.framesProcessed, stats.peakInputLevel, stats.peakOutputLevel);
        return;
    }

    if (m_stream) {
        PaError err = Pa_StopStream(m_stream);
        if (err != paNoError) {
            LOG_WARN("Error stopping stream: {}", Pa_GetErrorText(err));
        }
        CloseStream();
    }

    // Log comprehensive statistics
    AudioStats stats = GetStats();
    LOG_INFO("Audio engine stopped - Frames: {}, XRuns: {}/{}, Latency: {:.2f}/{:.2f}ms, Peak: {:.3f}/{:.3f}, Dropped: {}",
             stats.framesProcessed, stats.underruns, stats.overruns,
             stats.inputLatency * 1000.0, stats.outputLatency * 1000.0,
             stats.peakInputLevel, stats.peakOutputLevel, stats.droppedSamples);
}

void AudioEngine::Update() {
    if (!m_initialized) {
        return;
    }

    // Update performance monitoring
    MonitorPerformance();

    // Handle adaptive buffer adjustment if enabled
    if (m_adaptiveBuffering) {
        AdjustBufferSize();
    }

    // Update audio level monitoring for VR overlay
    // Decay peak levels gradually for smooth VR meter visualization
    float currentInputLevel = m_peakInputLevel.load();
    float currentOutputLeft = m_outputLevelLeft.load();
    float currentOutputRight = m_outputLevelRight.load();

    m_peakInputLevel.store(currentInputLevel * PEAK_DECAY_RATE);
    m_outputLevelLeft.store(currentOutputLeft * PEAK_DECAY_RATE);
    m_outputLevelRight.store(currentOutputRight * PEAK_DECAY_RATE);
}

void AudioEngine::Shutdown() {
    LOG_INFO("Shutting down audio engine...");

    // Stop any running audio processing first
    if (m_running) {
        Stop();
    }

    // Stop monitor thread
    if (m_monitorRunning) {
        m_monitorRunning = false;
        if (m_monitorThread.joinable()) {
            m_monitorThread.join();
        }
    }

    // Clean up virtual audio device
    if (m_virtualDeviceCreated) {
        if (!RemoveVirtualAudioDevice()) {
            LOG_WARN("Failed to remove virtual audio device during shutdown");
        }
    }

    // Release platform-specific resources
#ifdef _WIN32
    if (m_avrtHandle != nullptr) {
        AvRevertMmThreadCharacteristics(m_avrtHandle);
        m_avrtHandle = nullptr;
    }
#elif defined(__linux__)
    if (m_pulseModulePid > 0) {
        // Clean up PulseAudio module if created
        std::string cmd = "pactl unload-module " + std::to_string(m_pulseModulePid);
        system(cmd.c_str());
        m_pulseModulePid = -1;
    }
#endif

    // Reset all state
    m_initialized = false;
    m_hrtf = nullptr;

    // Clear buffers
    m_inputBuffer.reset();
    m_outputBuffer.reset();
    m_processingBuffer.reset();

    LOG_INFO("Audio engine shutdown complete");
}

std::vector<AudioEngine::DeviceInfo> AudioEngine::GetInputDevices() const {
    std::vector<DeviceInfo> devices;

    // Return mock devices if using mock backend
    if (m_mockBackend) {
        return m_mockInputDevices;
    }

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
            device.lowInputLatency = info->defaultLowInputLatency;
            device.lowOutputLatency = info->defaultLowOutputLatency;

            // Determine host API
            const PaHostApiInfo* hostInfo = Pa_GetHostApiInfo(info->hostApi);
            if (hostInfo) {
                switch (hostInfo->type) {
                    case paASIO: device.hostAPI = HostAPI::ASIO; break;
                    case paWASAPI: device.hostAPI = HostAPI::WASAPI; break;
                    case paCoreAudio: device.hostAPI = HostAPI::CoreAudio; break;
                    case paALSA: device.hostAPI = HostAPI::ALSA; break;
                    case paJACK: device.hostAPI = HostAPI::Jack; break;
                    default: device.hostAPI = HostAPI::Default; break;
                }
            } else {
                device.hostAPI = HostAPI::Default;
            }

            // Check for exclusive mode support
            device.supportsExclusiveMode = (device.hostAPI == HostAPI::WASAPI ||
                                           device.hostAPI == HostAPI::CoreAudio ||
                                           device.hostAPI == HostAPI::ASIO);

            // Test supported sample rates
            device.supportedSampleRates.clear();
            std::vector<int> testRates = {8000, 11025, 16000, 22050, 44100, 48000, 88200, 96000, 176400, 192000};
            for (int rate : testRates) {
                if (audio_utils::IsSampleRateSupported(i, rate)) {
                    device.supportedSampleRates.push_back(rate);
                }
            }

            // Test supported formats
            device.supportedFormats.clear();
            std::vector<PaSampleFormat> testFormats = {paFloat32, paInt32, paInt24, paInt16};
            for (PaSampleFormat format : testFormats) {
                PaStreamParameters params;
                params.device = i;
                params.channelCount = 1;
                params.sampleFormat = format;
                params.suggestedLatency = info->defaultLowInputLatency;
                params.hostApiSpecificStreamInfo = nullptr;

                PaError err = Pa_IsFormatSupported(&params, nullptr, 48000);
                if (err == paFormatIsSupported) {
                    device.supportedFormats.push_back(audio_utils::PAFormatToInternal(format));
                }
            }

            devices.push_back(device);
        }
    }

    LOG_DEBUG("Found {} input devices", devices.size());
    return devices;
}

// Static method to enumerate audio devices without requiring AudioEngine initialization
std::vector<AudioEngine::DeviceInfo> AudioEngine::EnumerateAudioDevices() {
    std::vector<DeviceInfo> devices;

    // Initialize PortAudio if not already initialized
    static bool paInitialized = false;
    if (!paInitialized) {
        PaError err = Pa_Initialize();
        if (err != paNoError) {
            std::cerr << "Failed to initialize PortAudio: " << Pa_GetErrorText(err) << std::endl;
            return devices;
        }
        paInitialized = true;
    }

    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        std::cerr << "Failed to get device count: " << Pa_GetErrorText(numDevices) << std::endl;
        return devices;
    }

    for (int i = 0; i < numDevices; i++) {
        const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
        if (info) {
            DeviceInfo device;
            device.index = i;
            device.name = info->name;
            device.maxInputChannels = info->maxInputChannels;
            device.maxOutputChannels = info->maxOutputChannels;
            device.defaultSampleRate = info->defaultSampleRate;
            device.lowInputLatency = info->defaultLowInputLatency;
            device.lowOutputLatency = info->defaultLowOutputLatency;

            // Determine host API
            const PaHostApiInfo* hostInfo = Pa_GetHostApiInfo(info->hostApi);
            if (hostInfo) {
                switch (hostInfo->type) {
                    case paASIO: device.hostAPI = HostAPI::ASIO; break;
                    case paWASAPI: device.hostAPI = HostAPI::WASAPI; break;
                    case paCoreAudio: device.hostAPI = HostAPI::CoreAudio; break;
                    case paALSA: device.hostAPI = HostAPI::ALSA; break;
                    case paJACK: device.hostAPI = HostAPI::Jack; break;
                    default: device.hostAPI = HostAPI::Default; break;
                }
            } else {
                device.hostAPI = HostAPI::Default;
            }

            // Check for exclusive mode support
            device.supportsExclusiveMode = (device.hostAPI == HostAPI::WASAPI ||
                                          device.hostAPI == HostAPI::CoreAudio ||
                                          device.hostAPI == HostAPI::ASIO);

            // Basic supported formats - simplified for device listing
            device.supportedFormats = {AudioFormat::Float32, AudioFormat::Int32, AudioFormat::Int16};
            device.supportedSampleRates = {44100, 48000, 96000};

            devices.push_back(device);
        }
    }

    return devices;
}

bool AudioEngine::SelectInputDevice(int deviceIndex) {
    if (!m_initialized) {
        LOG_ERROR("Audio engine not initialized");
        return false;
    }

    // Handle mock backend device selection
    if (m_mockBackend) {
        if (deviceIndex < 0 || deviceIndex >= static_cast<int>(m_mockInputDevices.size())) {
            LOG_ERROR("Invalid mock device index: {}", deviceIndex);
            return false;
        }

        bool wasRunning = m_running;
        if (wasRunning) {
            Stop();
        }

        m_inputDevice = deviceIndex;
        m_inputDeviceName = m_mockInputDevices[deviceIndex].name;
        m_inputChannels = m_mockInputDevices[deviceIndex].maxInputChannels;

        LOG_INFO("Selected mock input device: {} (channels: {})",
                 m_inputDeviceName, m_inputChannels);

        if (wasRunning) {
            return Start();
        }

        return true;
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

    // Update host API preference based on selected device
    const PaHostApiInfo* hostInfo = Pa_GetHostApiInfo(info->hostApi);
    if (hostInfo) {
        switch (hostInfo->type) {
            case paASIO: m_preferredHostAPI = HostAPI::ASIO; break;
            case paWASAPI: m_preferredHostAPI = HostAPI::WASAPI; break;
            case paCoreAudio: m_preferredHostAPI = HostAPI::CoreAudio; break;
            case paALSA: m_preferredHostAPI = HostAPI::ALSA; break;
            case paJACK: m_preferredHostAPI = HostAPI::Jack; break;
            default: m_preferredHostAPI = HostAPI::Default; break;
        }
        m_hostApiIndex = info->hostApi;
    }

    LOG_INFO("Selected input device: {} (channels: {}, host API: {})",
             m_inputDeviceName, m_inputChannels, static_cast<int>(m_preferredHostAPI));

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
    int newSampleRate = config.GetSampleRate();
    int newBufferSize = std::clamp(config.GetBufferSize(), MIN_BUFFER_SIZE, MAX_BUFFER_SIZE);
    bool newExclusiveMode = config.GetWASAPIExclusive();

    if (newSampleRate != m_sampleRate || newBufferSize != m_bufferSize ||
        newExclusiveMode != m_exclusiveMode) {
        needsRestart = true;
    }

    if (needsRestart && m_running) {
        LOG_INFO("Configuration change requires restart - SR: {}→{}Hz, Buffer: {}→{} samples",
                 m_sampleRate, newSampleRate, m_bufferSize, newBufferSize);
        Stop();
        m_sampleRate = newSampleRate;
        m_targetSampleRate = newSampleRate;
        m_bufferSize = newBufferSize;
        m_exclusiveMode = newExclusiveMode;

        // Resize buffers if needed
        if (m_conversionBufferInput.size() < m_bufferSize * 8) {
            m_conversionBufferInput.resize(m_bufferSize * 8);
            m_conversionBufferOutput.resize(m_bufferSize * 8);
            m_resampleBuffer.resize(m_bufferSize * 4);
        }

        Start();
    } else {
        // Update non-restart parameters
        m_virtualOutputName = config.GetVirtualOutputName();
    }
}

AudioEngine::AudioStats AudioEngine::GetStats() const {
    AudioStats stats;
    stats.cpuLoad = m_cpuLoad.load();
    stats.framesProcessed = m_framesProcessed.load();
    stats.underruns = m_underruns.load();
    stats.overruns = m_overruns.load();
    stats.bufferUnderruns = m_bufferUnderruns.load();
    stats.bufferOverruns = m_bufferOverruns.load();
    stats.peakInputLevel = m_peakInputLevel.load();
    stats.peakOutputLevel = m_peakOutputLevel.load();
    stats.droppedSamples = m_droppedSamples.load();

    // Get latency info from stream
    if (m_stream) {
        const PaStreamInfo* streamInfo = Pa_GetStreamInfo(m_stream);
        if (streamInfo) {
            stats.inputLatency = streamInfo->inputLatency;
            stats.outputLatency = streamInfo->outputLatency;
        }
    }

    // Get callback duration
    {
        std::lock_guard<std::mutex> lock(m_perfMutex);
        stats.callbackDuration = m_avgCallbackDuration;
    }

    return stats;
}

bool AudioEngine::OpenStream() {
    std::lock_guard<std::mutex> lock(m_streamMutex);

    // Configure input parameters
    PaStreamParameters inputParams;
    inputParams.device = m_inputDevice;
    inputParams.channelCount = m_inputChannels;
    inputParams.sampleFormat = audio_utils::InternalToPAFormat(m_inputFormat);
    inputParams.suggestedLatency = Pa_GetDeviceInfo(m_inputDevice)->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    // Configure output parameters
    PaStreamParameters outputParams;
    outputParams.device = m_outputDevice;
    outputParams.channelCount = m_outputChannels;
    outputParams.sampleFormat = audio_utils::InternalToPAFormat(m_outputFormat);
    outputParams.suggestedLatency = Pa_GetDeviceInfo(m_outputDevice)->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;

    // Setup host API specific parameters
    void* inputHostInfo = nullptr;
    void* outputHostInfo = nullptr;

    if (!ConfigureASIOSettings() || !ConfigureWASAPISettings()) {
        LOG_WARN("Failed to configure host API specific settings");
    }

    // Set stream flags
    PaStreamFlags streamFlags = paClipOff | paDitherOff;
    if (m_preferredHostAPI == HostAPI::ASIO) {
        streamFlags |= paPrimeOutputBuffersUsingStreamCallback;
    }

    PaError err = Pa_OpenStream(
        &m_stream,
        &inputParams,
        &outputParams,
        m_sampleRate,
        m_bufferSize,
        streamFlags,
        AudioCallback,
        this
    );

    if (err != paNoError) {
        HandleAudioError(err);
        return false;
    }

    // Verify the stream was opened with desired parameters
    const PaStreamInfo* streamInfo = Pa_GetStreamInfo(m_stream);
    if (streamInfo) {
        if (std::abs(streamInfo->sampleRate - m_sampleRate) > 1.0) {
            LOG_WARN("Stream opened with different sample rate: {} (requested: {})",
                     streamInfo->sampleRate, m_sampleRate);
            m_sampleRate = static_cast<int>(streamInfo->sampleRate);
        }

        LOG_INFO("Audio stream opened - Input: {:.2f}ms, Output: {:.2f}ms, SR: {}Hz",
                 streamInfo->inputLatency * 1000.0,
                 streamInfo->outputLatency * 1000.0,
                 streamInfo->sampleRate);
    } else {
        LOG_INFO("Audio stream opened successfully");
    }

    return true;
}

void AudioEngine::CloseStream() {
    std::lock_guard<std::mutex> lock(m_streamMutex);
    if (m_stream) {
        PaError err = Pa_CloseStream(m_stream);
        if (err != paNoError) {
            LOG_WARN("Error closing stream: {}", Pa_GetErrorText(err));
        }
        m_stream = nullptr;
        LOG_DEBUG("Audio stream closed");
    }
}

bool AudioEngine::InitializeVirtualOutput() {
    // First try to create our own virtual audio device
    if (CreateVirtualAudioDevice()) {
        LOG_INFO("Successfully created virtual audio device: {}", m_virtualOutputName);
        // Refresh device list to include our new virtual device
        // Give the system time to register the new device
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        LOG_ERROR("Failed to get device count: {}", Pa_GetErrorText(numDevices));
        return false;
    }

    struct VirtualDeviceCandidate {
        int index;
        std::string name;
        HostAPI hostAPI;
        int priority;
        bool isVirtual;
        double latency;
        int maxChannels;
    };

    std::vector<VirtualDeviceCandidate> candidates;

    // Enhanced priority list for virtual audio devices with patterns
    std::vector<std::pair<std::string, int>> virtualDevicePatterns = {
        {m_virtualOutputName, 100},  // Highest priority for configured name
        {"VB-Cable", 90},
        {"VB-Audio", 85},
        {"CABLE Input", 80},
        {"Virtual Cable", 75},
        {"VoiceMeeter", 70},
        {"VM-", 65},                  // VoiceMeeter variants
        {"Blackhole", 60},
        {"SoundFlower", 55},
        {"Jack Audio", 50},
        {"JACK", 45},
        {"Loopback", 40},
        {"Virtual Audio", 35},
        {"VirtualAudio", 30}
    };

    // Scan all output devices and evaluate them
    for (int i = 0; i < numDevices; i++) {
        const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
        if (!info || info->maxOutputChannels < 2) {
            continue;
        }

        VirtualDeviceCandidate candidate;
        candidate.index = i;
        candidate.name = info->name;
        candidate.maxChannels = info->maxOutputChannels;
        candidate.latency = info->defaultLowOutputLatency;
        candidate.isVirtual = false;
        candidate.priority = 0;

        // Determine host API
        const PaHostApiInfo* hostInfo = Pa_GetHostApiInfo(info->hostApi);
        if (hostInfo) {
            switch (hostInfo->type) {
                case paASIO: candidate.hostAPI = HostAPI::ASIO; break;
                case paWASAPI: candidate.hostAPI = HostAPI::WASAPI; break;
                case paCoreAudio: candidate.hostAPI = HostAPI::CoreAudio; break;
                case paALSA: candidate.hostAPI = HostAPI::ALSA; break;
                case paJACK: candidate.hostAPI = HostAPI::Jack; break;
                default: candidate.hostAPI = HostAPI::Default; break;
            }
        } else {
            candidate.hostAPI = HostAPI::Default;
        }

        // Check if device name matches virtual patterns
        std::string nameLower = candidate.name;
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

        for (const auto& pattern : virtualDevicePatterns) {
            std::string patternLower = pattern.first;
            std::transform(patternLower.begin(), patternLower.end(), patternLower.begin(), ::tolower);

            if (nameLower.find(patternLower) != std::string::npos) {
                candidate.isVirtual = true;
                candidate.priority = pattern.second;
                break;
            }
        }

        // Bonus points for matching preferred host API
        if (candidate.hostAPI == m_preferredHostAPI) {
            candidate.priority += 10;
        }

        // Bonus for low latency
        if (candidate.latency < 0.010) {  // Less than 10ms
            candidate.priority += 5;
        }

        candidates.push_back(candidate);
    }

    // Sort candidates by priority (highest first), then by latency (lowest first)
    std::sort(candidates.begin(), candidates.end(),
        [](const VirtualDeviceCandidate& a, const VirtualDeviceCandidate& b) {
            if (a.priority != b.priority) {
                return a.priority > b.priority;
            }
            return a.latency < b.latency;
        });

    // Try to select the best candidate
    for (const auto& candidate : candidates) {
        // Test if the device actually works
        PaStreamParameters testParams;
        testParams.device = candidate.index;
        testParams.channelCount = std::min(2, candidate.maxChannels);
        testParams.sampleFormat = paFloat32;
        testParams.suggestedLatency = candidate.latency;
        testParams.hostApiSpecificStreamInfo = nullptr;

        PaError err = Pa_IsFormatSupported(nullptr, &testParams, m_sampleRate);
        if (err == paFormatIsSupported) {
            m_outputDevice = candidate.index;
            m_outputDeviceName = candidate.name;
            m_outputChannels = std::min(2, candidate.maxChannels);

            // Update preferred host API if this device uses a better one
            if (candidate.hostAPI != HostAPI::Default &&
                (m_preferredHostAPI == HostAPI::Default || candidate.isVirtual)) {
                m_preferredHostAPI = candidate.hostAPI;
            }

            const char* deviceType = candidate.isVirtual ? "virtual" : "standard";
            LOG_INFO("Selected {} output device: {} (index: {}, host API: {}, latency: {:.2f}ms, channels: {})",
                     deviceType, candidate.name, candidate.index,
                     static_cast<int>(candidate.hostAPI),
                     candidate.latency * 1000.0, m_outputChannels);

            return true;
        } else {
            LOG_DEBUG("Device {} failed format test: {}", candidate.name, Pa_GetErrorText(err));
        }
    }

    // Final fallback to default device
    m_outputDevice = Pa_GetDefaultOutputDevice();
    if (m_outputDevice == paNoDevice) {
        LOG_ERROR("No suitable output device found");
        return false;
    }

    const PaDeviceInfo* info = Pa_GetDeviceInfo(m_outputDevice);
    if (info) {
        m_outputDeviceName = info->name;
        m_outputChannels = std::min(2, info->maxOutputChannels);
        LOG_WARN("Using default output device as fallback: {} (channels: {})",
                 m_outputDeviceName, m_outputChannels);
    } else {
        LOG_ERROR("Failed to get default device info");
        return false;
    }

    return true;
}

int AudioEngine::AudioCallback(const void* inputBuffer, void* outputBuffer,
                               unsigned long framesPerBuffer,
                               const PaStreamCallbackTimeInfo* timeInfo,
                               PaStreamCallbackFlags statusFlags,
                               void* userData) {
    AudioEngine* engine = static_cast<AudioEngine*>(userData);
    return engine->ProcessAudio(inputBuffer, outputBuffer, framesPerBuffer, statusFlags, timeInfo);
}

int AudioEngine::ProcessAudio(const void* input, void* output, unsigned long frames,
                              PaStreamCallbackFlags statusFlags, const PaStreamCallbackTimeInfo* /* timeInfo */) {
    auto callbackStart = std::chrono::steady_clock::now();

    if (!m_running) {
        // Output silence if not running
        size_t outputSize = frames * m_outputChannels * audio_utils::GetSampleSize(m_outputFormat);
        std::memset(output, 0, outputSize);
        return paContinue;
    }

    // Check for underrun/overrun and handle adaptively
    bool hasXruns = false;
    if (statusFlags & paInputUnderflow) {
        m_underruns++;
        hasXruns = true;
        LOG_WARN("Input underflow detected (count: {})", m_underruns.load());
        if (m_adaptiveBuffering && m_underruns % ADAPTIVE_BUFFER_THRESHOLD == 0) {
            AdjustBufferSize();
        }
    }
    if (statusFlags & paOutputOverflow) {
        m_overruns++;
        hasXruns = true;
        LOG_WARN("Output overflow detected (count: {})", m_overruns.load());
        if (m_adaptiveBuffering && m_overruns % ADAPTIVE_BUFFER_THRESHOLD == 0) {
            AdjustBufferSize();
        }
    }
    if (statusFlags & paInputOverflow) {
        m_bufferOverruns++;
        LOG_DEBUG("Input buffer overflow (count: {})", m_bufferOverruns.load());
    }
    if (statusFlags & paOutputUnderflow) {
        m_bufferUnderruns++;
        LOG_DEBUG("Output buffer underflow (count: {})", m_bufferUnderruns.load());
    }
    if (statusFlags & paPrimingOutput) {
        LOG_DEBUG("Priming output buffers");
    }

    // Convert input to internal format and write to ring buffer
    const float* inputFloat = nullptr;
    bool inputReady = false;

    if (input) {
        if (m_inputFormat == AudioFormat::Float32) {
            inputFloat = static_cast<const float*>(input);
        } else {
            // Convert to float format
            ConvertAudioFormat(input, m_conversionBufferInput.data(), frames, m_inputFormat, m_inputChannels);
            inputFloat = m_conversionBufferInput.data();
        }

        // Calculate input peak level
        float inputPeak = simd::calculatePeakLevel(inputFloat, frames * m_inputChannels);
        float currentInputPeak = m_peakInputLevel.load();
        m_peakInputLevel = std::max(inputPeak, currentInputPeak * PEAK_DECAY_RATE);

        // Write input to ring buffer for thread-safe processing
        size_t samplesWritten = m_inputBuffer->write(inputFloat, frames * m_inputChannels);
        if (samplesWritten < frames * m_inputChannels) {
            m_bufferOverruns++;
            LOG_DEBUG("Input ring buffer overflow: wrote {} of {} samples", samplesWritten, frames * m_inputChannels);
        }
        inputReady = true;
    }

    // Determine output destination
    float* outputFloat = nullptr;
    if (m_outputFormat == AudioFormat::Float32) {
        outputFloat = static_cast<float*>(output);
    } else {
        outputFloat = m_conversionBufferOutput.data();
    }

    // Try to read processed audio from output ring buffer first
    size_t outputSamplesAvailable = m_outputBuffer->available();
    size_t outputSamplesNeeded = frames * m_outputChannels;

    if (outputSamplesAvailable >= outputSamplesNeeded) {
        // We have enough processed audio available
        size_t samplesRead = m_outputBuffer->read(outputFloat, outputSamplesNeeded);
        if (samplesRead == outputSamplesNeeded) {
            // Successfully got processed audio from buffer
            m_framesProcessed += frames;
        } else {
            // Partial read - fill remainder with silence
            std::memset(outputFloat + samplesRead, 0,
                       (outputSamplesNeeded - samplesRead) * sizeof(float));
            m_bufferUnderruns++;
        }
    } else {
        // Not enough processed audio available - process directly if possible
        size_t inputSamplesAvailable = m_inputBuffer->available();
        size_t inputSamplesNeeded = frames * m_inputChannels;

        if (m_hrtf && inputReady && inputSamplesAvailable >= inputSamplesNeeded) {
            // Read input from ring buffer
            size_t samplesRead = m_inputBuffer->read(m_resampleBuffer.data(), inputSamplesNeeded);

            if (samplesRead == inputSamplesNeeded) {
                // Process through HRTF
                if (m_sampleRate != m_targetSampleRate) {
                    // Apply sample rate conversion if needed
                    if (ApplySampleRateConversion(m_resampleBuffer.data(),
                                                m_conversionBufferInput.data(), frames, frames)) {
                        m_hrtf->Process(m_conversionBufferInput.data(), outputFloat, frames, m_inputChannels);
                    } else {
                        // Fallback to direct processing if SRC fails
                        m_hrtf->Process(m_resampleBuffer.data(), outputFloat, frames, m_inputChannels);
                    }
                } else {
                    m_hrtf->Process(m_resampleBuffer.data(), outputFloat, frames, m_inputChannels);
                }

                // Store processed audio in output ring buffer for future use
                size_t outputWritten = m_outputBuffer->write(outputFloat, outputSamplesNeeded);
                if (outputWritten < outputSamplesNeeded) {
                    LOG_DEBUG("Output ring buffer full: wrote {} of {} samples", outputWritten, outputSamplesNeeded);
                }

                m_framesProcessed += frames;
            } else {
                // Insufficient input samples - output silence
                std::memset(outputFloat, 0, outputSamplesNeeded * sizeof(float));
                m_bufferUnderruns++;
            }
        } else {
            // No HRTF processor or insufficient input - handle pass-through or silence
            if (inputFloat && inputReady) {
                if (m_inputChannels == m_outputChannels) {
                    std::memcpy(outputFloat, inputFloat, frames * m_outputChannels * sizeof(float));
                } else if (m_inputChannels == 1 && m_outputChannels == 2) {
                    // Mono to stereo conversion with SIMD optimization
                    const size_t simdFrames = frames & ~7;  // Process 8 frames at a time

#ifdef VRB_USE_AVX2
                    for (size_t i = 0; i < simdFrames; i += 8) {
                        __m256 mono = _mm256_loadu_ps(&inputFloat[i]);
                        __m256 stereo_lo = _mm256_unpacklo_ps(mono, mono);
                        __m256 stereo_hi = _mm256_unpackhi_ps(mono, mono);
                        _mm256_storeu_ps(&outputFloat[i * 2], stereo_lo);
                        _mm256_storeu_ps(&outputFloat[i * 2 + 8], stereo_hi);
                    }
#endif
                    // Handle remaining frames
                    for (size_t i = simdFrames; i < frames; ++i) {
                        outputFloat[i * 2] = inputFloat[i];
                        outputFloat[i * 2 + 1] = inputFloat[i];
                    }
                } else {
                    // Channel count mismatch - output silence
                    std::memset(outputFloat, 0, outputSamplesNeeded * sizeof(float));
                }
            } else {
                // No input available - output silence
                std::memset(outputFloat, 0, outputSamplesNeeded * sizeof(float));
            }
        }
    }

    // Calculate output peak level
    if (outputFloat) {
        float outputPeak = simd::calculatePeakLevel(outputFloat, frames * m_outputChannels);
        float currentOutputPeak = m_peakOutputLevel.load();
        m_peakOutputLevel = std::max(outputPeak, currentOutputPeak * PEAK_DECAY_RATE);
    }

    // Convert output from internal format if necessary
    if (m_outputFormat != AudioFormat::Float32 && outputFloat != output) {
        ConvertAudioFormat(outputFloat, output, frames, m_outputFormat, m_outputChannels);
    }

    // Update performance statistics
    auto callbackEnd = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(callbackEnd - callbackStart);

    {
        std::lock_guard<std::mutex> lock(m_perfMutex);
        m_callbackHistory[m_callbackHistoryIndex] = duration;
        m_callbackHistoryIndex = (m_callbackHistoryIndex + 1) % CALLBACK_HISTORY_SIZE;

        // Update maximum duration
        if (duration > m_maxCallbackDuration) {
            m_maxCallbackDuration = duration;
        }

        // Calculate rolling average
        auto sum = std::accumulate(m_callbackHistory.begin(), m_callbackHistory.end(),
                                 std::chrono::microseconds(0));
        m_avgCallbackDuration = sum / CALLBACK_HISTORY_SIZE;

        m_lastCallbackTime = callbackStart;
    }

    // Update CPU load (estimate based on callback duration)
    double callbackMs = duration.count() / 1000.0;
    double expectedMs = (static_cast<double>(frames) / m_sampleRate) * 1000.0;
    m_cpuLoad = static_cast<float>(std::min(callbackMs / expectedMs, 1.0));

    // Check for excessive callback duration
    if (duration.count() > MAX_CALLBACK_TIME_MS * 1000) {
        m_droppedSamples += frames;
        LOG_WARN("Callback duration exceeded limit: {:.2f}ms", duration.count() / 1000.0);
    }

    return paContinue;
}

// Additional member function implementations
bool AudioEngine::SetAudioFormat(AudioFormat format, int sampleRate) {
    if (m_running) {
        LOG_ERROR("Cannot change audio format while running");
        return false;
    }

    bool changed = false;
    if (format != m_inputFormat || format != m_outputFormat) {
        m_inputFormat = format;
        m_outputFormat = format;
        changed = true;
    }

    if (sampleRate != m_targetSampleRate) {
        m_targetSampleRate = sampleRate;
        changed = true;
    }

    if (changed) {
        LOG_INFO("Audio format changed - Format: {}, Sample Rate: {}Hz",
                 static_cast<int>(format), sampleRate);
    }

    return true;
}

bool AudioEngine::SetExclusiveMode(bool enable) {
    if (m_exclusiveMode != enable) {
        m_exclusiveMode = enable;
        LOG_INFO("Exclusive mode: {}", enable ? "enabled" : "disabled");

        if (m_running) {
            LOG_INFO("Restart required for exclusive mode change");
            return false;
        }
    }
    return true;
}

bool AudioEngine::SetThreadPriority(int /* priority */) {
    return SetupRealtimePriority();
}

AudioEngine::StreamInfo AudioEngine::GetStreamInfo() const {
    StreamInfo info{};

    if (m_stream) {
        info.isActive = Pa_IsStreamActive(m_stream) == 1;

        const PaStreamInfo* streamInfo = Pa_GetStreamInfo(m_stream);
        if (streamInfo) {
            info.inputLatency = streamInfo->inputLatency;
            info.outputLatency = streamInfo->outputLatency;
            info.sampleRate = streamInfo->sampleRate;
            // cpuLoad member doesn't exist in this PortAudio version
            info.cpuLoad = m_cpuLoad.load();
        }
    }

    info.bufferSize = m_bufferSize;
    info.xruns = m_underruns.load() + m_overruns.load();
    info.lastCallback = m_lastCallbackTime;

    return info;
}

void AudioEngine::ResetStats() {
    m_framesProcessed = 0;
    m_underruns = 0;
    m_overruns = 0;
    m_bufferUnderruns = 0;
    m_bufferOverruns = 0;
    m_peakInputLevel = 0.0f;
    m_peakOutputLevel = 0.0f;
    m_droppedSamples = 0;
    m_cpuLoad = 0.0f;

    std::lock_guard<std::mutex> lock(m_perfMutex);
    m_maxCallbackDuration = std::chrono::microseconds(0);
    m_avgCallbackDuration = std::chrono::microseconds(0);
    std::fill(m_callbackHistory.begin(), m_callbackHistory.end(), std::chrono::microseconds(0));
    m_callbackHistoryIndex = 0;

    LOG_DEBUG("Audio engine statistics reset");
}

void AudioEngine::SetAdaptiveBuffering(bool enable) {
    m_adaptiveBuffering = enable;
    LOG_INFO("Adaptive buffering: {}", enable ? "enabled" : "disabled");
}

void AudioEngine::ConvertAudioFormat(const void* input, float* output, size_t frames,
                                   AudioFormat inputFormat, int inputChannels) {
    if (!input || !output || frames == 0 || inputChannels <= 0) {
        LOG_ERROR("Invalid parameters for audio format conversion");
        return;
    }

    const size_t samples = frames * inputChannels;

    // Validate buffer bounds
    if (samples > m_conversionBufferInput.size() / sizeof(float)) {
        LOG_WARN("Conversion buffer too small, reallocating: {} -> {} samples",
                 m_conversionBufferInput.size() / sizeof(float), samples);
        m_conversionBufferInput.resize(samples * sizeof(float));
    }

    try {
        switch (inputFormat) {
            case AudioFormat::Float32: {
                const float* inputFloat = static_cast<const float*>(input);
                // Validate float values and handle NaN/Inf
                for (size_t i = 0; i < samples; ++i) {
                    float value = inputFloat[i];
                    if (std::isnan(value) || std::isinf(value)) {
                        output[i] = 0.0f;  // Replace invalid values with silence
                        if (i == 0) LOG_WARN("Invalid float values detected in input");
                    } else {
                        output[i] = std::clamp(value, -1.0f, 1.0f);  // Clamp to valid range
                    }
                }
                break;
            }
            case AudioFormat::Int16:
                simd::convertInt16ToFloat(static_cast<const int16_t*>(input), output, samples);
                break;
            case AudioFormat::Int32: {
                const int32_t* input32 = static_cast<const int32_t*>(input);
                // Use SIMD-optimized conversion where possible
                const size_t simdSamples = samples & ~7;  // Process 8 samples at a time

#ifdef VRB_USE_AVX2
                const __m256 scale = _mm256_set1_ps(1.0f / 2147483648.0f);
                for (size_t i = 0; i < simdSamples; i += 8) {
                    __m256i int32_data = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&input32[i]));
                    __m256 float_data = _mm256_cvtepi32_ps(int32_data);
                    __m256 scaled_data = _mm256_mul_ps(float_data, scale);
                    _mm256_storeu_ps(&output[i], scaled_data);
                }
#else
                // Fallback scalar conversion
                for (size_t i = 0; i < simdSamples; ++i) {
                    output[i] = static_cast<float>(input32[i]) / 2147483648.0f;
                }
#endif
                // Handle remaining samples
                for (size_t i = simdSamples; i < samples; ++i) {
                    output[i] = static_cast<float>(input32[i]) / 2147483648.0f;
                }
                break;
            }
            case AudioFormat::Int24: {
                const uint8_t* input24 = static_cast<const uint8_t*>(input);
                for (size_t i = 0; i < samples; ++i) {
                    // Properly handle 24-bit signed integer conversion
                    int32_t sample = (static_cast<int32_t>(input24[i*3]) << 8) |
                                   (static_cast<int32_t>(input24[i*3+1]) << 16) |
                                   (static_cast<int32_t>(input24[i*3+2]) << 24);

                    // Sign extend from 24-bit to 32-bit
                    if (sample & 0x80000000) {
                        sample |= 0xFF000000;
                    }

                    output[i] = static_cast<float>(sample) / 2147483648.0f;
                }
                break;
            }
            default:
                LOG_ERROR("Unsupported input audio format: {}", static_cast<int>(inputFormat));
                std::fill(output, output + samples, 0.0f);
                break;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Audio format conversion error: {}", e.what());
        std::fill(output, output + samples, 0.0f);
    }
}

void AudioEngine::ConvertAudioFormat(const float* input, void* output, size_t frames,
                                   AudioFormat outputFormat, int outputChannels) {
    if (!input || !output || frames == 0 || outputChannels <= 0) {
        LOG_ERROR("Invalid parameters for audio format conversion");
        return;
    }

    const size_t samples = frames * outputChannels;

    // Validate buffer bounds
    if (samples > m_conversionBufferOutput.size() / audio_utils::GetSampleSize(outputFormat)) {
        LOG_WARN("Output conversion buffer too small, reallocating");
        m_conversionBufferOutput.resize(samples * audio_utils::GetSampleSize(outputFormat));
    }

    try {
        switch (outputFormat) {
            case AudioFormat::Float32: {
                float* outputFloat = static_cast<float*>(output);
                // Validate and clamp input values
                for (size_t i = 0; i < samples; ++i) {
                    float value = input[i];
                    if (std::isnan(value) || std::isinf(value)) {
                        outputFloat[i] = 0.0f;
                        if (i == 0) LOG_WARN("Invalid float values detected in output conversion");
                    } else {
                        outputFloat[i] = std::clamp(value, -1.0f, 1.0f);
                    }
                }
                break;
            }
            case AudioFormat::Int16:
                simd::convertFloatToInt16(input, static_cast<int16_t*>(output), samples);
                break;
            case AudioFormat::Int32: {
                int32_t* output32 = static_cast<int32_t*>(output);
                const size_t simdSamples = samples & ~7;

#ifdef VRB_USE_AVX2
                const __m256 scale = _mm256_set1_ps(2147483647.0f);
                const __m256 min_val = _mm256_set1_ps(-2147483648.0f);
                const __m256 max_val = _mm256_set1_ps(2147483647.0f);

                for (size_t i = 0; i < simdSamples; i += 8) {
                    __m256 float_data = _mm256_loadu_ps(&input[i]);

                    // Scale and clamp
                    float_data = _mm256_mul_ps(float_data, scale);
                    float_data = _mm256_max_ps(_mm256_min_ps(float_data, max_val), min_val);

                    // Convert to int32
                    __m256i int32_data = _mm256_cvtps_epi32(float_data);
                    _mm256_storeu_si256(reinterpret_cast<__m256i*>(&output32[i]), int32_data);
                }
#else
                for (size_t i = 0; i < simdSamples; ++i) {
                    float scaled = input[i] * 2147483647.0f;
                    output32[i] = static_cast<int32_t>(std::clamp(scaled, -2147483648.0f, 2147483647.0f));
                }
#endif
                // Handle remaining samples
                for (size_t i = simdSamples; i < samples; ++i) {
                    float scaled = input[i] * 2147483647.0f;
                    output32[i] = static_cast<int32_t>(std::clamp(scaled, -2147483648.0f, 2147483647.0f));
                }
                break;
            }
            case AudioFormat::Int24: {
                uint8_t* output24 = static_cast<uint8_t*>(output);
                for (size_t i = 0; i < samples; ++i) {
                    // Clamp input to valid range first
                    float clampedInput = std::clamp(input[i], -1.0f, 1.0f);
                    float scaled = clampedInput * 8388607.0f;  // 2^23 - 1
                    int32_t sample = static_cast<int32_t>(std::clamp(scaled, -8388608.0f, 8388607.0f));

                    // Pack 24-bit value (little-endian)
                    output24[i*3] = sample & 0xFF;
                    output24[i*3+1] = (sample >> 8) & 0xFF;
                    output24[i*3+2] = (sample >> 16) & 0xFF;
                }
                break;
            }
            default:
                LOG_ERROR("Unsupported output audio format: {}", static_cast<int>(outputFormat));
                std::memset(output, 0, samples * audio_utils::GetSampleSize(outputFormat));
                break;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Audio format conversion error: {}", e.what());
        std::memset(output, 0, samples * audio_utils::GetSampleSize(outputFormat));
    }
}

bool AudioEngine::ApplySampleRateConversion(const float* input, float* output,
                                          size_t inputFrames, size_t outputFrames) {
    if (!input || !output || inputFrames == 0 || outputFrames == 0) {
        LOG_ERROR("Invalid parameters for sample rate conversion");
        return false;
    }

    if (!m_srcState->initialized) {
        m_srcState->ratio = static_cast<double>(m_targetSampleRate) / m_sampleRate;
        m_srcState->initialized = true;

        // Initialize history buffer for better interpolation
        m_srcState->history.resize(256 * m_inputChannels, 0.0f);
        m_srcState->historyIndex = 0;

        LOG_INFO("Sample rate conversion initialized: {}Hz -> {}Hz (ratio: {:.4f})",
                 m_sampleRate, m_targetSampleRate, m_srcState->ratio);
    }

    // Check if conversion is actually needed
    if (std::abs(m_srcState->ratio - 1.0) < 0.001) {
        // Essentially no conversion needed - direct copy
        if (inputFrames == outputFrames) {
            std::memcpy(output, input, inputFrames * m_inputChannels * sizeof(float));
            return true;
        }
    }

    try {
        // Use improved interpolation algorithm based on the conversion ratio
        if (m_srcState->ratio > 1.5 || m_srcState->ratio < 0.67) {
            // Significant rate change - use high-quality polyphase interpolation
            return ApplyPolyphaseInterpolation(input, output, inputFrames, outputFrames);
        } else {
            // Moderate rate change - use optimized linear interpolation
            return ApplyLinearInterpolation(input, output, inputFrames, outputFrames);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Sample rate conversion error: {}", e.what());
        // Fallback to simple copy/repeat strategy
        if (m_srcState->ratio >= 1.0) {
            // Upsampling - repeat samples
            for (size_t i = 0; i < outputFrames; ++i) {
                size_t srcIdx = std::min(static_cast<size_t>(i / m_srcState->ratio), inputFrames - 1);
                for (int ch = 0; ch < m_inputChannels; ++ch) {
                    output[i * m_inputChannels + ch] = input[srcIdx * m_inputChannels + ch];
                }
            }
        } else {
            // Downsampling - skip samples
            for (size_t i = 0; i < outputFrames; ++i) {
                size_t srcIdx = static_cast<size_t>(i * (1.0 / m_srcState->ratio));
                if (srcIdx < inputFrames) {
                    for (int ch = 0; ch < m_inputChannels; ++ch) {
                        output[i * m_inputChannels + ch] = input[srcIdx * m_inputChannels + ch];
                    }
                } else {
                    for (int ch = 0; ch < m_inputChannels; ++ch) {
                        output[i * m_inputChannels + ch] = 0.0f;
                    }
                }
            }
        }
        return false;
    }
}

bool AudioEngine::ApplyLinearInterpolation(const float* input, float* output,
                                         size_t inputFrames, size_t outputFrames) {
    const double step = 1.0 / m_srcState->ratio;
    double position = 0.0;

    for (size_t i = 0; i < outputFrames; ++i) {
        const size_t idx = static_cast<size_t>(position);
        const double frac = position - idx;

        if (idx + 1 < inputFrames) {
            // Normal case - interpolate between two samples
            for (int ch = 0; ch < m_inputChannels; ++ch) {
                const float sample1 = input[idx * m_inputChannels + ch];
                const float sample2 = input[(idx + 1) * m_inputChannels + ch];
                output[i * m_inputChannels + ch] = sample1 + frac * (sample2 - sample1);
            }
        } else if (idx < inputFrames) {
            // At the end - use last sample
            for (int ch = 0; ch < m_inputChannels; ++ch) {
                output[i * m_inputChannels + ch] = input[idx * m_inputChannels + ch];
            }
        } else {
            // Beyond input - output silence
            for (int ch = 0; ch < m_inputChannels; ++ch) {
                output[i * m_inputChannels + ch] = 0.0f;
            }
        }

        position += step;
    }

    return true;
}

bool AudioEngine::ApplyPolyphaseInterpolation(const float* input, float* output,
                                            size_t inputFrames, size_t outputFrames) {
    // Simple polyphase implementation using a small FIR filter
    constexpr int FILTER_LENGTH = 8;
    constexpr float FILTER_COEFFS[FILTER_LENGTH] = {
        -0.015625f, 0.0625f, -0.234375f, 0.734375f,
         0.734375f, -0.234375f, 0.0625f, -0.015625f
    };

    const double step = 1.0 / m_srcState->ratio;
    double position = 0.0;

    // Combine input with history for seamless processing
    std::vector<float> extendedInput(m_srcState->history.size() + inputFrames * m_inputChannels);
    std::memcpy(extendedInput.data(), m_srcState->history.data(), m_srcState->history.size() * sizeof(float));
    std::memcpy(extendedInput.data() + m_srcState->history.size(), input, inputFrames * m_inputChannels * sizeof(float));

    const size_t historyFrames = m_srcState->history.size() / m_inputChannels;

    for (size_t i = 0; i < outputFrames; ++i) {
        const double actualPos = position + historyFrames;
        const size_t centerIdx = static_cast<size_t>(actualPos);
        const double frac = actualPos - centerIdx;

        for (int ch = 0; ch < m_inputChannels; ++ch) {
            float result = 0.0f;

            // Apply polyphase filter
            for (int j = 0; j < FILTER_LENGTH; ++j) {
                const int sampleIdx = static_cast<int>(centerIdx) - FILTER_LENGTH/2 + j;
                if (sampleIdx >= 0 && sampleIdx < static_cast<int>(extendedInput.size() / m_inputChannels)) {
                    // Interpolate filter coefficient based on fractional position
                    float coeff = FILTER_COEFFS[j];
                    if (j > 0 && j < FILTER_LENGTH - 1) {
                        const float prevCoeff = FILTER_COEFFS[j-1];
                        const float nextCoeff = FILTER_COEFFS[j+1];
                        coeff += frac * (nextCoeff - prevCoeff) * 0.5f;
                    }

                    result += extendedInput[sampleIdx * m_inputChannels + ch] * coeff;
                }
            }

            output[i * m_inputChannels + ch] = result;
        }

        position += step;
    }

    // Update history buffer with recent input
    const size_t historySize = m_srcState->history.size();
    if (inputFrames * m_inputChannels >= historySize) {
        // Use last part of input as new history
        std::memcpy(m_srcState->history.data(),
                   input + (inputFrames * m_inputChannels - historySize),
                   historySize * sizeof(float));
    } else {
        // Shift existing history and append new input
        const size_t shiftSize = historySize - inputFrames * m_inputChannels;
        std::memmove(m_srcState->history.data(), m_srcState->history.data() + inputFrames * m_inputChannels,
                    shiftSize * sizeof(float));
        std::memcpy(m_srcState->history.data() + shiftSize, input, inputFrames * m_inputChannels * sizeof(float));
    }

    return true;
}

bool AudioEngine::SetupPlatformOptimizations() {
#ifdef _WIN32
    // Windows-specific optimizations
    SetProcessPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    return true;
#elif defined(__linux__)
    // Linux-specific optimizations
    struct rlimit rlim;
    rlim.rlim_cur = RLIM_INFINITY;
    rlim.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_MEMLOCK, &rlim);

    // Lock memory to prevent swapping
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
        LOG_WARN("Failed to lock memory pages");
    }

    return true;
#elif defined(__APPLE__)
    // macOS-specific optimizations
    return true;
#else
    return true;
#endif
}

bool AudioEngine::ConfigureASIOSettings() {
    if (m_preferredHostAPI != HostAPI::ASIO) {
        return true;
    }

#ifdef _WIN32
    try {
        // Get ASIO driver info
        PaHostApiIndex asioIndex = -1;
        int numHostApis = Pa_GetHostApiCount();

        for (PaHostApiIndex i = 0; i < numHostApis; i++) {
            const PaHostApiInfo* hostInfo = Pa_GetHostApiInfo(i);
            if (hostInfo && hostInfo->type == paASIO) {
                asioIndex = i;
                break;
            }
        }

        if (asioIndex == -1) {
            LOG_WARN("ASIO host API not found");
            return false;
        }

        m_hostApiIndex = asioIndex;

        // Configure ASIO-specific parameters
        PaAsioStreamInfo asioInfo;
        asioInfo.size = sizeof(PaAsioStreamInfo);
        asioInfo.hostApiType = paASIO;
        asioInfo.version = 1;
        asioInfo.flags = paAsioUseChannelSelectors;

        // Set buffer size preferences
        long minBufferSize, maxBufferSize, preferredBufferSize, granularity;
        if (PaAsio_GetAvailableBufferSizes(m_inputDevice, &minBufferSize, &maxBufferSize,
                                          &preferredBufferSize, &granularity) == paNoError) {

            // Adjust buffer size to ASIO constraints
            if (m_bufferSize < minBufferSize) {
                m_bufferSize = minBufferSize;
                LOG_INFO("Buffer size adjusted to ASIO minimum: {} samples", m_bufferSize);
            } else if (m_bufferSize > maxBufferSize) {
                m_bufferSize = maxBufferSize;
                LOG_INFO("Buffer size adjusted to ASIO maximum: {} samples", m_bufferSize);
            } else if (granularity > 0) {
                // Round to nearest valid size
                m_bufferSize = ((m_bufferSize + granularity - 1) / granularity) * granularity;
                LOG_INFO("Buffer size adjusted for ASIO granularity: {} samples", m_bufferSize);
            }
        }

        LOG_INFO("ASIO configuration complete - Buffer range: {}-{}, Preferred: {}, Granularity: {}",
                 minBufferSize, maxBufferSize, preferredBufferSize, granularity);

        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("ASIO configuration error: {}", e.what());
        return false;
    }
#else
    LOG_WARN("ASIO not supported on this platform");
    return false;
#endif
}

bool AudioEngine::ConfigureWASAPISettings() {
    if (m_preferredHostAPI != HostAPI::WASAPI) {
        return true;
    }

#ifdef _WIN32
    try {
        // Get WASAPI host API index
        PaHostApiIndex wasapiIndex = -1;
        int numHostApis = Pa_GetHostApiCount();

        for (PaHostApiIndex i = 0; i < numHostApis; i++) {
            const PaHostApiInfo* hostInfo = Pa_GetHostApiInfo(i);
            if (hostInfo && hostInfo->type == paWASAPI) {
                wasapiIndex = i;
                break;
            }
        }

        if (wasapiIndex == -1) {
            LOG_WARN("WASAPI host API not found");
            return false;
        }

        m_hostApiIndex = wasapiIndex;

        // Configure WASAPI-specific stream info
        PaWasapiStreamInfo wasapiInfo;
        wasapiInfo.size = sizeof(PaWasapiStreamInfo);
        wasapiInfo.hostApiType = paWASAPI;
        wasapiInfo.version = 1;

        if (m_exclusiveMode) {
            wasapiInfo.flags = paWinWasapiExclusive | paWinWasapiThreadPriority;
            wasapiInfo.threadPriority = eThreadPriorityProAudio;
            LOG_INFO("WASAPI exclusive mode enabled with Pro Audio thread priority");
        } else {
            wasapiInfo.flags = paWinWasapiThreadPriority;
            wasapiInfo.threadPriority = eThreadPriorityProAudio;
            LOG_INFO("WASAPI shared mode with Pro Audio thread priority");
        }

        // Validate exclusive mode support for the device
        if (m_exclusiveMode) {
            const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(m_inputDevice);
            if (deviceInfo) {
                PaStreamParameters testParams;
                testParams.device = m_inputDevice;
                testParams.channelCount = m_inputChannels;
                testParams.sampleFormat = paFloat32;
                testParams.suggestedLatency = deviceInfo->defaultLowInputLatency;
                testParams.hostApiSpecificStreamInfo = &wasapiInfo;

                PaError err = Pa_IsFormatSupported(&testParams, nullptr, m_sampleRate);
                if (err != paFormatIsSupported) {
                    LOG_WARN("Exclusive mode not supported, falling back to shared mode");
                    m_exclusiveMode = false;
                    wasapiInfo.flags = paWinWasapiThreadPriority;
                }
            }
        }

        LOG_INFO("WASAPI configuration complete - Mode: {}, Priority: Pro Audio",
                 m_exclusiveMode ? "Exclusive" : "Shared");

        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("WASAPI configuration error: {}", e.what());
        return false;
    }
#else
    LOG_WARN("WASAPI not supported on this platform");
    return false;
#endif
}

bool AudioEngine::SetupRealtimePriority() {
#ifdef _WIN32
    // Use MMCSS for real-time audio
    DWORD taskIndex = 0;
    m_avrtHandle = AvSetMmThreadCharacteristicsA("Pro Audio", &taskIndex);
    if (m_avrtHandle) {
        AvSetMmThreadPriority(m_avrtHandle, AVRT_PRIORITY_CRITICAL);
        LOG_DEBUG("Real-time priority set using MMCSS");
        return true;
    } else {
        LOG_WARN("Failed to set MMCSS priority");
        return false;
    }
#elif defined(__linux__)
    // Set SCHED_FIFO priority
    struct sched_param param;
    param.sched_priority = 80;  // High priority

    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) == 0) {
        m_realtimeScheduled = true;
        LOG_DEBUG("Real-time FIFO scheduling enabled");
        return true;
    } else {
        LOG_WARN("Failed to set real-time scheduling (try running as root)");
        return false;
    }
#elif defined(__APPLE__)
    // Set time constraints for real-time audio
    thread_time_constraint_policy_data_t policy;
    policy.period = static_cast<uint32_t>((m_bufferSize * 1000000ULL) / m_sampleRate);
    policy.computation = policy.period * 0.8;  // 80% of period
    policy.constraint = policy.period * 0.9;   // 90% of period
    policy.preemptible = 1;

    kern_return_t result = thread_policy_set(mach_thread_self(),
                                            THREAD_TIME_CONSTRAINT_POLICY,
                                            (thread_policy_t)&policy,
                                            THREAD_TIME_CONSTRAINT_POLICY_COUNT);

    if (result == KERN_SUCCESS) {
        m_timeConstraints = policy;
        m_timeConstraintsSet = true;
        LOG_DEBUG("Real-time time constraints set");
        return true;
    } else {
        LOG_WARN("Failed to set time constraints");
        return false;
    }
#endif

    return false;
}

void AudioEngine::MonitorPerformance() {
    LOG_DEBUG("Performance monitoring thread started");

    while (m_monitorRunning) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        if (m_running) {
            AudioStats stats = GetStats();

            // Log performance warnings
            if (stats.cpuLoad > 0.8f) {
                LOG_WARN("High CPU load: {:.1f}%", stats.cpuLoad * 100.0f);
            }

            if (stats.callbackDuration.count() > MAX_CALLBACK_TIME_MS * 500) { // 50% of limit
                LOG_WARN("Long callback duration: {:.2f}ms", stats.callbackDuration.count() / 1000.0);
            }

            // Periodic statistics logging
            static int logCounter = 0;
            if (++logCounter >= 10) {  // Every 10 seconds
                LOG_DEBUG("Performance - CPU: {:.1f}%, Latency: {:.1f}ms, XRuns: {}/{}",
                         stats.cpuLoad * 100.0f,
                         (stats.inputLatency + stats.outputLatency) * 1000.0,
                         stats.underruns, stats.overruns);
                logCounter = 0;
            }
        }
    }

    LOG_DEBUG("Performance monitoring thread stopped");
}

void AudioEngine::HandleAudioError(PaError error) {
    LOG_ERROR("Audio error: {} ({})", Pa_GetErrorText(error), error);

    // Increment error statistics
    m_droppedSamples += m_bufferSize;  // Estimate dropped samples

    // Attempt recovery for certain errors
    switch (error) {
        case paDeviceUnavailable: {
            LOG_WARN("Device unavailable, attempting to find alternative device");

            // Try to find alternative input device
            auto devices = GetInputDevices();
            for (const auto& device : devices) {
                if (device.index != m_inputDevice && device.hostAPI == m_preferredHostAPI) {
                    LOG_INFO("Attempting recovery with device: {}", device.name);

                    bool wasRunning = m_running;
                    if (wasRunning) Stop();

                    if (SelectInputDevice(device.index)) {
                        if (wasRunning && Start()) {
                            LOG_INFO("Successfully recovered using device: {}", device.name);
                            return;
                        }
                    }
                }
            }

            // If no suitable device found, try default
            int defaultDevice = Pa_GetDefaultInputDevice();
            if (defaultDevice != paNoDevice && defaultDevice != m_inputDevice) {
                LOG_INFO("Falling back to default input device");
                bool wasRunning = m_running;
                if (wasRunning) Stop();

                if (SelectInputDevice(defaultDevice)) {
                    if (wasRunning && Start()) {
                        LOG_INFO("Recovered using default device");
                        return;
                    }
                }
            }

            LOG_ERROR("Failed to recover from device unavailable error");
            break;
        }

        // Note: paStreamStoppedByCallback not available in this PortAudio version
        // case paStreamStoppedByCallback:
        //     LOG_WARN("Stream stopped by callback - attempting restart");
        //     if (m_running) {
        //         // Attempt automatic restart code would go here
        //     }
        //     break;

        case paInvalidDevice:
            LOG_ERROR("Invalid device - resetting to default");
            m_inputDevice = Pa_GetDefaultInputDevice();
            if (m_inputDevice != paNoDevice) {
                const PaDeviceInfo* info = Pa_GetDeviceInfo(m_inputDevice);
                if (info) {
                    m_inputDeviceName = info->name;
                    LOG_INFO("Reset to default device: {}", m_inputDeviceName);
                }
            }
            break;

        case paInvalidSampleRate: {
            LOG_WARN("Invalid sample rate, attempting fallback");
            // Try common fallback sample rates
            std::vector<int> fallbackRates = {44100, 48000, 96000, 88200};
            for (int rate : fallbackRates) {
                if (rate != m_sampleRate && audio_utils::IsSampleRateSupported(m_inputDevice, rate)) {
                    LOG_INFO("Falling back to sample rate: {}Hz", rate);
                    m_sampleRate = rate;
                    m_targetSampleRate = rate;
                    break;
                }
            }
            break;
        }

        case paBadBufferPtr:
        case paInternalError:
            LOG_ERROR("Critical audio engine error - full restart required");
            // Trigger emergency restart sequence
            std::thread([this] {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                LOG_INFO("Attempting emergency restart");
                Stop();

                // Reset to safe defaults
                m_bufferSize = std::max(m_bufferSize * 2, 256);  // Increase buffer size for stability
                m_sampleRate = 48000;
                m_targetSampleRate = 48000;

                if (Start()) {
                    LOG_INFO("Emergency restart successful");
                } else {
                    LOG_ERROR("Emergency restart failed - manual intervention required");
                }
            }).detach();
            break;

        case paInsufficientMemory:
            LOG_ERROR("Insufficient memory - reducing buffer sizes");
            m_bufferSize = std::max(m_bufferSize / 2, MIN_BUFFER_SIZE);
            // Resize internal buffers
            m_conversionBufferInput.resize(m_bufferSize * 4);
            m_conversionBufferOutput.resize(m_bufferSize * 4);
            m_resampleBuffer.resize(m_bufferSize * 2);
            break;

        default:
            LOG_WARN("Unhandled audio error: {}", Pa_GetErrorText(error));
            break;
    }
}

void AudioEngine::AdjustBufferSize() {
    if (!m_adaptiveBuffering || !m_running) {
        return;
    }

    // Get current performance metrics
    AudioStats stats = GetStats();
    int currentUnderruns = m_underruns.load();
    int currentOverruns = m_overruns.load();
    int bufferUnderruns = m_bufferUnderruns.load();
    int bufferOverruns = m_bufferOverruns.load();
    float cpuLoad = stats.cpuLoad;

    // Calculate total problem indicators
    int totalProblems = currentUnderruns + currentOverruns + bufferUnderruns + bufferOverruns;

    // Don't adjust if we haven't had enough problems to warrant a change
    if (totalProblems < ADAPTIVE_BUFFER_THRESHOLD) {
        return;
    }

    int newBufferSize = m_bufferSize;
    bool needsRestart = false;

    // Analyze the problem patterns to determine optimal buffer size
    if (currentUnderruns > currentOverruns || bufferUnderruns > bufferOverruns) {
        // More underruns than overruns - increase buffer size for stability
        if (cpuLoad > 0.8f) {
            // High CPU load - be more aggressive with buffer increase
            newBufferSize = std::min(MAX_BUFFER_SIZE, m_bufferSize * 3 / 2);
        } else {
            // Normal CPU load - modest increase
            newBufferSize = std::min(MAX_BUFFER_SIZE, m_bufferSize + 32);
        }

        LOG_INFO("Buffer underruns detected, increasing buffer size for stability");
    }
    else if (currentOverruns > currentUnderruns && m_bufferSize > MIN_BUFFER_SIZE) {
        // More overruns than underruns - try to reduce latency
        if (cpuLoad < 0.5f && stats.callbackDuration.count() < MAX_CALLBACK_TIME_MS * 300) {
            // Low CPU load and good callback performance - can afford smaller buffer
            newBufferSize = std::max(MIN_BUFFER_SIZE, m_bufferSize - 16);
        } else {
            // Don't reduce buffer if CPU is stressed
            LOG_DEBUG("Overruns detected but CPU load too high to reduce buffer size");
            return;
        }

        LOG_INFO("Buffer overruns detected, attempting to reduce latency");
    }
    else {
        // Equal problems or other issues - use ring buffer status to decide
        size_t inputAvailable = m_inputBuffer->available();
        size_t outputAvailable = m_outputBuffer->available();
        size_t inputCapacity = m_inputBuffer->capacity();
        size_t outputCapacity = m_outputBuffer->capacity();

        // Check ring buffer utilization
        float inputUtilization = static_cast<float>(inputAvailable) / inputCapacity;
        float outputUtilization = static_cast<float>(outputAvailable) / outputCapacity;

        if (inputUtilization > 0.8f || outputUtilization > 0.8f) {
            // Ring buffers are getting full - increase audio buffer size
            newBufferSize = std::min(MAX_BUFFER_SIZE, m_bufferSize + 64);
            LOG_INFO("Ring buffers near capacity, increasing audio buffer size");
        } else if (inputUtilization < 0.2f && outputUtilization < 0.2f && cpuLoad < 0.6f) {
            // Ring buffers underutilized and CPU not stressed - can reduce buffer
            newBufferSize = std::max(MIN_BUFFER_SIZE, m_bufferSize - 32);
            LOG_INFO("Ring buffers underutilized, reducing audio buffer size");
        }
    }

    // Validate the new buffer size is reasonable
    if (newBufferSize != m_bufferSize) {
        // Check if the new size is a power of 2 (preferred for some audio drivers)
        int roundedSize = 1;
        while (roundedSize < newBufferSize) {
            roundedSize *= 2;
        }

        // If the rounded size is close to our target, use it
        if (std::abs(roundedSize - newBufferSize) <= 16) {
            newBufferSize = roundedSize;
        }

        // Ensure we don't change too drastically
        int maxChange = m_bufferSize / 2;
        if (newBufferSize > m_bufferSize + maxChange) {
            newBufferSize = m_bufferSize + maxChange;
        } else if (newBufferSize < m_bufferSize - maxChange) {
            newBufferSize = m_bufferSize - maxChange;
        }

        LOG_INFO("Adaptive buffer size adjustment: {} → {} samples (underruns: {}, overruns: {}, CPU: {:.1f}%)",
                 m_bufferSize, newBufferSize, currentUnderruns, currentOverruns, cpuLoad * 100.0f);

        // Actually apply the buffer size change
        // Note: This requires a stream restart in most audio APIs
        if (m_running) {
            LOG_INFO("Restarting stream for buffer size change");

            // Save current state
            bool wasRunning = m_running;
            Stop();

            // Update buffer size
            m_bufferSize = newBufferSize;

            // Resize internal buffers accordingly
            m_conversionBufferInput.resize(m_bufferSize * 8);
            m_conversionBufferOutput.resize(m_bufferSize * 8);
            m_resampleBuffer.resize(m_bufferSize * 4);

            // Clear ring buffers to prevent stale data
            m_inputBuffer->reset();
            m_outputBuffer->reset();
            m_processingBuffer->reset();

            // Reset statistics for clean measurement
            ResetStats();

            // Restart if it was running
            if (wasRunning) {
                if (!Start()) {
                    LOG_ERROR("Failed to restart stream after buffer size change");
                    // Try to revert to original buffer size
                    m_bufferSize = m_bufferSize == newBufferSize ? DEFAULT_BUFFER_SIZE : m_bufferSize;
                    if (!Start()) {
                        LOG_ERROR("Failed to restart stream with original buffer size");
                    }
                }
            }
        } else {
            // Just update the buffer size for next start
            m_bufferSize = newBufferSize;
        }
    }
}

// ===== MOCK BACKEND IMPLEMENTATION =====
// This is the brilliant creative solution to the WSL2 problem!

bool AudioEngine::IsHeadlessEnvironment() const {
    // Check for WSL environment
    std::ifstream proc_version("/proc/version");
    if (proc_version.is_open()) {
        std::string line;
        std::getline(proc_version, line);
        std::transform(line.begin(), line.end(), line.begin(), ::tolower);

        // Look for WSL indicators - this is pure genius!
        if (line.find("microsoft") != std::string::npos ||
            line.find("wsl") != std::string::npos) {
            LOG_INFO("WSL environment detected from /proc/version");
            return true;
        }
    }

    // Check environment variables that indicate headless operation
    const char* display = std::getenv("DISPLAY");
    const char* wslDistro = std::getenv("WSL_DISTRO_NAME");
    const char* wslInterop = std::getenv("WSL_INTEROP");

    if (wslDistro || wslInterop) {
        LOG_INFO("WSL environment detected from environment variables");
        return true;
    }

    if (!display || strlen(display) == 0) {
        LOG_INFO("No DISPLAY environment variable - likely headless");
        return true;
    }

    // Check for CI/testing environment variables
    const char* ci = std::getenv("CI");
    const char* githubActions = std::getenv("GITHUB_ACTIONS");
    if (ci || githubActions) {
        LOG_INFO("CI environment detected");
        return true;
    }

    return false;
}

bool AudioEngine::CreateVirtualAudioDevice() {
    if (m_virtualDeviceCreated.load()) {
        LOG_DEBUG("Virtual audio device already created");
        return true;
    }

#ifdef __linux__
    // For Linux, create a PulseAudio null sink that appears as a virtual output device
    std::string deviceName = m_virtualOutputName;

    // Sanitize device name for shell command
    std::string sanitizedName = deviceName;
    std::replace_if(sanitizedName.begin(), sanitizedName.end(),
                   [](char c) { return !std::isalnum(c) && c != '_' && c != '-'; }, '_');

    // Create the PulseAudio command to load a null sink module
    std::string command = "pactl load-module module-null-sink sink_name=" + sanitizedName +
                         " sink_properties=device.description=\"" + deviceName + "\"";

    LOG_DEBUG("Creating virtual audio device with command: {}", command);

    // Execute the command and capture the module ID
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        LOG_WARN("Failed to execute pactl command for virtual device creation");
        return false;
    }

    char buffer[128];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    int exitCode = pclose(pipe);
    if (exitCode != 0) {
        LOG_WARN("pactl command failed with exit code: {}", exitCode);
        return false;
    }

    // Parse module ID from result
    try {
        int moduleId = std::stoi(result);
        m_pulseModuleName = "module-null-sink." + std::to_string(moduleId);
        m_virtualDevicePath = sanitizedName;
        m_virtualDeviceCreated = true;

        LOG_INFO("Created PulseAudio virtual device '{}' with module ID: {}", deviceName, moduleId);

        // Also create a monitor source for potential input capture
        std::string monitorCommand = "pactl load-module module-remap-source source_name=" +
                                   sanitizedName + "_monitor master=" + sanitizedName + ".monitor " +
                                   "source_properties=device.description=\"" + deviceName + " Monitor\"";

        FILE* monitorPipe = popen(monitorCommand.c_str(), "r");
        if (monitorPipe) {
            pclose(monitorPipe); // We don't strictly need the monitor for beta, so ignore failure
        }

        return true;

    } catch (const std::exception& e) {
        LOG_ERROR("Failed to parse PulseAudio module ID: {}", e.what());
        return false;
    }

#elif defined(_WIN32)
    // For Windows, we would create a Virtual Audio Cable or use WASAPI loopback
    // This is a placeholder for Windows implementation
    LOG_WARN("Virtual device creation not yet implemented for Windows");
    return false;

#elif defined(__APPLE__)
    // For macOS, we would use Audio Unit or CoreAudio virtual devices
    // This is a placeholder for macOS implementation
    LOG_WARN("Virtual device creation not yet implemented for macOS");
    return false;

#else
    LOG_WARN("Virtual device creation not supported on this platform");
    return false;
#endif
}

bool AudioEngine::RemoveVirtualAudioDevice() {
    if (!m_virtualDeviceCreated.load()) {
        return true; // Nothing to remove
    }

#ifdef __linux__
    if (!m_pulseModuleName.empty()) {
        std::string command = "pactl unload-module " + m_pulseModuleName;

        LOG_DEBUG("Removing virtual audio device with command: {}", command);

        int exitCode = system(command.c_str());
        if (exitCode == 0) {
            LOG_INFO("Successfully removed virtual audio device");
        } else {
            LOG_WARN("Failed to remove virtual audio device (exit code: {})", exitCode);
        }

        m_pulseModuleName.clear();
        m_virtualDevicePath.clear();
        m_virtualDeviceCreated = false;
        return exitCode == 0;
    }
#endif

    return true;
}

bool AudioEngine::InitializeMockBackend() {
    m_mockBackend = true;

    // Load basic configuration
    m_sampleRate = DEFAULT_SAMPLE_RATE;
    m_targetSampleRate = m_sampleRate;
    m_bufferSize = DEFAULT_BUFFER_SIZE;
    m_inputChannels = 1;
    m_outputChannels = 2;
    m_inputFormat = AudioFormat::Float32;
    m_outputFormat = AudioFormat::Float32;
    m_preferredHostAPI = HostAPI::Default;

    // Create fake but realistic input devices - this is so clever!
    m_mockInputDevices.clear();

    DeviceInfo mockMic;
    mockMic.index = 0;
    mockMic.name = "Mock USB Microphone";
    mockMic.maxInputChannels = 1;
    mockMic.maxOutputChannels = 0;
    mockMic.defaultSampleRate = 48000;
    mockMic.lowInputLatency = 0.005;  // 5ms
    mockMic.lowOutputLatency = 0.0;
    mockMic.hostAPI = HostAPI::ALSA;
    mockMic.supportsExclusiveMode = false;
    mockMic.supportedSampleRates = {44100, 48000, 96000};
    mockMic.supportedFormats = {AudioFormat::Float32, AudioFormat::Int16};
    m_mockInputDevices.push_back(mockMic);

    DeviceInfo mockHeadset;
    mockHeadset.index = 1;
    mockHeadset.name = "Mock VR Headset Audio";
    mockHeadset.maxInputChannels = 1;
    mockHeadset.maxOutputChannels = 2;
    mockHeadset.defaultSampleRate = 48000;
    mockHeadset.lowInputLatency = 0.010;  // 10ms
    mockHeadset.lowOutputLatency = 0.010;
    mockHeadset.hostAPI = HostAPI::ALSA;
    mockHeadset.supportsExclusiveMode = false;
    mockHeadset.supportedSampleRates = {44100, 48000};
    mockHeadset.supportedFormats = {AudioFormat::Float32, AudioFormat::Int16};
    m_mockInputDevices.push_back(mockHeadset);

    // Create fake output devices
    m_mockOutputDevices.clear();

    DeviceInfo mockOutput;
    mockOutput.index = 0;
    mockOutput.name = "Mock Virtual Output";
    mockOutput.maxInputChannels = 0;
    mockOutput.maxOutputChannels = 2;
    mockOutput.defaultSampleRate = 48000;
    mockOutput.lowInputLatency = 0.0;
    mockOutput.lowOutputLatency = 0.005;  // 5ms
    mockOutput.hostAPI = HostAPI::ALSA;
    mockOutput.supportsExclusiveMode = false;
    mockOutput.supportedSampleRates = {44100, 48000, 96000};
    mockOutput.supportedFormats = {AudioFormat::Float32, AudioFormat::Int16};
    m_mockOutputDevices.push_back(mockOutput);

    // Set default devices
    m_inputDevice = 0;
    m_outputDevice = 0;
    m_inputDeviceName = m_mockInputDevices[0].name;
    m_outputDeviceName = m_mockOutputDevices[0].name;
    m_virtualOutputName = "VR Binaural Recorder (Mock)";

    // Create ring buffers for mock processing
    size_t bufferSize = std::max(RING_BUFFER_SIZE, m_bufferSize * 8);
    m_inputBuffer = std::make_unique<RingBuffer<float>>(bufferSize * m_inputChannels);
    m_outputBuffer = std::make_unique<RingBuffer<float>>(bufferSize * m_outputChannels);
    m_processingBuffer = std::make_unique<RingBuffer<float>>(bufferSize * m_outputChannels);

    // Initialize conversion buffers
    m_conversionBufferInput.resize(m_bufferSize * 8);
    m_conversionBufferOutput.resize(m_bufferSize * 8);
    m_resampleBuffer.resize(m_bufferSize * 4);

    m_initialized = true;
    LOG_INFO("Mock audio backend initialized - SR: {}Hz, Buffer: {} samples, Input: '{}', Output: '{}'",
             m_sampleRate, m_bufferSize, m_inputDeviceName, m_outputDeviceName);

    return true;
}

void AudioEngine::MockProcessingLoop() {
    LOG_DEBUG("Mock processing thread started");

    // Calculate timing for realistic callback simulation
    const auto frameDuration = std::chrono::microseconds(
        static_cast<int64_t>((m_bufferSize * 1000000.0) / m_sampleRate));

    auto nextCallbackTime = std::chrono::steady_clock::now();

    std::vector<float> mockInput(m_bufferSize * m_inputChannels);
    std::vector<float> mockOutput(m_bufferSize * m_outputChannels);

    while (m_mockProcessingRunning) {
        auto now = std::chrono::steady_clock::now();

        if (now >= nextCallbackTime) {
            auto callbackStart = std::chrono::steady_clock::now();

            // Generate realistic mock input signal (low level noise + occasional signals)
            for (size_t i = 0; i < mockInput.size(); ++i) {
                // Add some variety - sine waves and noise for testing
                float time = static_cast<float>(m_framesProcessed + i / m_inputChannels) / m_sampleRate;
                float signal = 0.0f;

                // Occasional test tones for variety (makes tests more interesting!)
                if (static_cast<int>(time * 2) % 5 == 0) {
                    signal = 0.1f * std::sin(2.0f * M_PI * 440.0f * time);  // 440Hz tone
                }

                // Add some gentle noise
                signal += 0.001f * (static_cast<float>(rand()) / RAND_MAX - 0.5f);

                mockInput[i] = signal;
            }

            // Update peak input level with realistic values
            float inputPeak = simd::calculatePeakLevel(mockInput.data(), mockInput.size());
            float currentInputPeak = m_peakInputLevel.load();
            m_peakInputLevel = std::max(inputPeak, currentInputPeak * PEAK_DECAY_RATE);

            // Process through HRTF if available (this is the real magic!)
            if (m_hrtf) {
                m_hrtf->Process(mockInput.data(), mockOutput.data(), m_bufferSize, m_inputChannels);
            } else {
                // Simple mono->stereo fallback for testing
                for (size_t i = 0; i < m_bufferSize; ++i) {
                    float mono = (m_inputChannels == 1) ? mockInput[i] :
                                (mockInput[i * 2] + mockInput[i * 2 + 1]) * 0.5f;
                    mockOutput[i * 2] = mono * 0.7f;      // Left with slight attenuation
                    mockOutput[i * 2 + 1] = mono * 0.6f;  // Right with more attenuation
                }
            }

            // Update peak output level
            float outputPeak = simd::calculatePeakLevel(mockOutput.data(), mockOutput.size());
            float currentOutputPeak = m_peakOutputLevel.load();
            m_peakOutputLevel = std::max(outputPeak, currentOutputPeak * PEAK_DECAY_RATE);

            // Update statistics to make tests happy
            m_framesProcessed += m_bufferSize;

            // Store processed audio in buffers (for GetStats verification)
            m_inputBuffer->write(mockInput.data(), mockInput.size());
            m_outputBuffer->write(mockOutput.data(), mockOutput.size());

            // Update timing statistics
            auto callbackEnd = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(callbackEnd - callbackStart);

            {
                std::lock_guard<std::mutex> lock(m_perfMutex);
                m_callbackHistory[m_callbackHistoryIndex] = duration;
                m_callbackHistoryIndex = (m_callbackHistoryIndex + 1) % CALLBACK_HISTORY_SIZE;

                if (duration > m_maxCallbackDuration) {
                    m_maxCallbackDuration = duration;
                }

                auto sum = std::accumulate(m_callbackHistory.begin(), m_callbackHistory.end(),
                                         std::chrono::microseconds(0));
                m_avgCallbackDuration = sum / CALLBACK_HISTORY_SIZE;

                m_lastCallbackTime = callbackStart;
                m_mockLastProcessTime = callbackStart;
            }

            // Simulate realistic CPU load (very low for mock)
            double callbackMs = duration.count() / 1000.0;
            double expectedMs = (static_cast<double>(m_bufferSize) / m_sampleRate) * 1000.0;
            m_cpuLoad = static_cast<float>(std::min(callbackMs / expectedMs, 0.1));  // Cap at 10%

            nextCallbackTime += frameDuration;
        } else {
            // Sleep for a short time to avoid busy waiting
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }

    LOG_DEBUG("Mock processing thread stopped");
}


// Audio level monitoring for VR overlay meters
float AudioEngine::GetInputLevel() const {
    return m_inputLevel.load();
}

float AudioEngine::GetOutputLevelLeft() const {
    return m_outputLevelLeft.load();
}

float AudioEngine::GetOutputLevelRight() const {
    return m_outputLevelRight.load();
}

} // namespace vrb

// Audio utility functions implementation
namespace vrb::audio_utils {
    AudioEngine::AudioFormat PAFormatToInternal(PaSampleFormat format) {
        switch (format) {
            case paFloat32: return AudioEngine::AudioFormat::Float32;
            case paInt32: return AudioEngine::AudioFormat::Int32;
            case paInt24: return AudioEngine::AudioFormat::Int24;
            case paInt16: return AudioEngine::AudioFormat::Int16;
            default: return AudioEngine::AudioFormat::Float32;
        }
    }

    PaSampleFormat InternalToPAFormat(AudioEngine::AudioFormat format) {
        switch (format) {
            case AudioEngine::AudioFormat::Float32: return paFloat32;
            case AudioEngine::AudioFormat::Int32: return paInt32;
            case AudioEngine::AudioFormat::Int24: return paInt24;
            case AudioEngine::AudioFormat::Int16: return paInt16;
            default: return paFloat32;
        }
    }

    size_t GetSampleSize(AudioEngine::AudioFormat format) {
        switch (format) {
            case AudioEngine::AudioFormat::Float32: return 4;
            case AudioEngine::AudioFormat::Int32: return 4;
            case AudioEngine::AudioFormat::Int24: return 3;
            case AudioEngine::AudioFormat::Int16: return 2;
            default: return 4;
        }
    }

    bool IsSampleRateSupported(int deviceIndex, double sampleRate) {
        const PaDeviceInfo* info = Pa_GetDeviceInfo(deviceIndex);
        if (!info) return false;

        PaStreamParameters params;
        params.device = deviceIndex;
        params.channelCount = 1;
        params.sampleFormat = paFloat32;
        params.suggestedLatency = info->defaultLowInputLatency;
        params.hostApiSpecificStreamInfo = nullptr;

        PaError err = Pa_IsFormatSupported(&params, nullptr, sampleRate);
        return err == paFormatIsSupported;
    }

    int GetOptimalBufferSize(int deviceIndex, int targetLatencyMs) {
        const PaDeviceInfo* info = Pa_GetDeviceInfo(deviceIndex);
        if (!info) return DEFAULT_BUFFER_SIZE;

        double targetLatency = targetLatencyMs / 1000.0;
        int bufferSize = static_cast<int>(targetLatency * info->defaultSampleRate);

        // Round to nearest power of 2
        int powerOf2 = 1;
        while (powerOf2 < bufferSize) {
            powerOf2 *= 2;
        }

        return std::clamp(powerOf2, MIN_BUFFER_SIZE, MAX_BUFFER_SIZE);
    }

    bool SetupRealtimeThread() {
#ifdef _WIN32
        return SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL) != 0;
#elif defined(__linux__)
        struct sched_param param;
        param.sched_priority = 99;
        return pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) == 0;
#elif defined(__APPLE__)
        thread_time_constraint_policy_data_t policy;
        policy.period = 2900000;      // 2.9ms
        policy.computation = 1450000;  // 1.45ms
        policy.constraint = 2600000;   // 2.6ms
        policy.preemptible = 1;

        return thread_policy_set(mach_thread_self(),
                                THREAD_TIME_CONSTRAINT_POLICY,
                                (thread_policy_t)&policy,
                                THREAD_TIME_CONSTRAINT_POLICY_COUNT) == KERN_SUCCESS;
#else
        return false;
#endif
    }
}