// audio_engine.h - Core Audio Processing Module
// Handles audio I/O and routing through HRTF processor

#pragma once

// SIMD support detection
#if defined(__AVX2__) || (defined(_MSC_VER) && defined(__AVX2__))
#define VRB_USE_AVX2 1
#include <immintrin.h>
#elif defined(__SSE2__) || (defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86_FP)))
#define VRB_USE_SSE2 1
#include <emmintrin.h>
#endif

#include <atomic>
#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <portaudio.h>

// Platform-specific includes for low-latency audio
#ifdef _WIN32
#include <pa_win_wasapi.h>
#include <pa_asio.h>
#include <windows.h>
#include <avrt.h>
#pragma comment(lib, "avrt.lib")
#elif defined(__linux__)
#include <pa_linux_alsa.h>
#include <sched.h>
#include <sys/mman.h>
#elif defined(__APPLE__)
#include <pa_mac_core.h>
#include <mach/mach.h>
#include <mach/thread_policy.h>
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
    enum class AudioFormat {
        Float32,
        Int32,
        Int24,
        Int16
    };

    enum class HostAPI {
        Default,
        ASIO,
        WASAPI,
        CoreAudio,
        ALSA,
        Jack
    };

    struct DeviceInfo {
        int index;
        std::string name;
        int maxInputChannels;
        int maxOutputChannels;
        double defaultSampleRate;
        double lowInputLatency;
        double lowOutputLatency;
        HostAPI hostAPI;
        bool supportsExclusiveMode;
        std::vector<int> supportedSampleRates;
        std::vector<AudioFormat> supportedFormats;
    };

    struct StreamInfo {
        bool isActive;
        double inputLatency;
        double outputLatency;
        double sampleRate;
        int bufferSize;
        float cpuLoad;
        int xruns;
        std::chrono::steady_clock::time_point lastCallback;
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
     * @brief Update audio engine state and processing
     * Called each frame for audio system updates
     */
    void Update();

    /**
     * @brief Stop audio processing
     */
    void Stop();

    /**
     * @brief Shutdown audio engine and cleanup resources
     * Stops processing and releases all audio resources
     */
    void Shutdown();

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
     * @brief Check if mock backend is active
     * @return true if using mock backend for testing
     */
    bool IsMockBackend() const { return m_mockBackend; }

    /**
     * @brief Get current audio statistics
     */
    struct AudioStats {
        float cpuLoad;
        int64_t framesProcessed;
        int underruns;
        int overruns;
        double inputLatency;
        double outputLatency;
        int bufferUnderruns;
        int bufferOverruns;
        float peakInputLevel;
        float peakOutputLevel;
        std::chrono::microseconds callbackDuration;
        int droppedSamples;
    };
    AudioStats GetStats() const;


    /**
     * @brief Audio level monitoring for VR overlay meters
     */
    float GetInputLevel() const;
    float GetOutputLevelLeft() const;
    float GetOutputLevelRight() const;

    /**
     * @brief Set audio format and sample rate
     * @param format Audio sample format
     * @param sampleRate Target sample rate
     * @return true if format change successful
     */
    bool SetAudioFormat(AudioFormat format, int sampleRate);

    /**
     * @brief Enable exclusive mode (WASAPI/CoreAudio)
     * @param enable Enable or disable exclusive mode
     * @return true if successful
     */
    bool SetExclusiveMode(bool enable);

    /**
     * @brief Set thread priority for audio processing
     * @param priority Thread priority level (0-99)
     * @return true if successful
     */
    bool SetThreadPriority(int priority);

    /**
     * @brief Get stream information
     * @return Current stream status and timing info
     */
    StreamInfo GetStreamInfo() const;

    /**
     * @brief Reset audio statistics
     */
    void ResetStats();

    /**
     * @brief Enable/disable adaptive buffer sizing
     * @param enable Enable adaptive buffering
     */
    void SetAdaptiveBuffering(bool enable);

    /**
     * @brief Static method to enumerate available audio devices without initializing AudioEngine
     * @return Vector of available input devices with detailed information
     */
    static std::vector<DeviceInfo> EnumerateAudioDevices();

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
     * @brief Process audio buffer with format conversion
     */
    int ProcessAudio(const void* input, void* output, unsigned long frames,
                     PaStreamCallbackFlags statusFlags, const PaStreamCallbackTimeInfo* timeInfo);


    /**
     * @brief Convert audio format
     */
    void ConvertAudioFormat(const void* input, float* output, size_t frames,
                           AudioFormat inputFormat, int inputChannels);
    void ConvertAudioFormat(const float* input, void* output, size_t frames,
                           AudioFormat outputFormat, int outputChannels);

    /**
     * @brief Apply sample rate conversion
     */
    bool ApplySampleRateConversion(const float* input, float* output,
                                  size_t inputFrames, size_t outputFrames);

    /**
     * @brief Apply linear interpolation for sample rate conversion
     */
    bool ApplyLinearInterpolation(const float* input, float* output,
                                 size_t inputFrames, size_t outputFrames);

    /**
     * @brief Apply polyphase interpolation for high-quality sample rate conversion
     */
    bool ApplyPolyphaseInterpolation(const float* input, float* output,
                                   size_t inputFrames, size_t outputFrames);

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

    /**
     * @brief Create platform-specific virtual audio device
     * @return true if virtual device creation successful
     */
    bool CreateVirtualAudioDevice();

    /**
     * @brief Remove platform-specific virtual audio device
     * @return true if virtual device removal successful
     */
    bool RemoveVirtualAudioDevice();

    /**
     * @brief Setup platform-specific audio optimizations
     */
    bool SetupPlatformOptimizations();

    /**
     * @brief Configure ASIO-specific settings
     */
    bool ConfigureASIOSettings();

    /**
     * @brief Configure WASAPI-specific settings
     */
    bool ConfigureWASAPISettings();

    /**
     * @brief Setup real-time thread priority
     */
    bool SetupRealtimePriority();

    /**
     * @brief Monitor audio performance
     */
    void MonitorPerformance();

    /**
     * @brief Handle audio errors and recovery
     */
    void HandleAudioError(PaError error);

    /**
     * @brief Adaptive buffer size adjustment
     */
    void AdjustBufferSize();

    /**
     * @brief Detect if running in headless/WSL2 environment
     */
    bool IsHeadlessEnvironment() const;

    /**
     * @brief Initialize mock audio backend for testing environments
     */
    bool InitializeMockBackend();


    /**
     * @brief Mock processing thread loop
     */
    void MockProcessingLoop();

    // State
    std::atomic<bool> m_initialized{false};
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_exclusiveMode{false};
    std::atomic<bool> m_adaptiveBuffering{false};
    std::atomic<bool> m_mockBackend{false};

    // Audio configuration
    int m_sampleRate;
    int m_targetSampleRate;
    int m_bufferSize;
    int m_inputChannels;
    int m_outputChannels;
    AudioFormat m_inputFormat;
    AudioFormat m_outputFormat;
    HostAPI m_preferredHostAPI;

    // Devices
    int m_inputDevice;
    int m_outputDevice;
    std::string m_inputDeviceName;
    std::string m_outputDeviceName;
    std::string m_virtualOutputName;

    // PortAudio
    PaStream* m_stream;
    PaHostApiIndex m_hostApiIndex;

    // Processing
    HRTFProcessor* m_hrtf;
    std::unique_ptr<RingBuffer<float>> m_inputBuffer;
    std::unique_ptr<RingBuffer<float>> m_outputBuffer;
    std::unique_ptr<RingBuffer<float>> m_processingBuffer;

    // Format conversion buffers
    std::vector<float> m_conversionBufferInput;
    std::vector<float> m_conversionBufferOutput;
    std::vector<float> m_resampleBuffer;

    // Sample rate conversion
    struct SRCState;
    std::unique_ptr<SRCState> m_srcState;

    // Statistics and monitoring
    mutable std::atomic<float> m_cpuLoad{0.0f};
    mutable std::atomic<int64_t> m_framesProcessed{0};
    mutable std::atomic<int> m_underruns{0};
    mutable std::atomic<int> m_overruns{0};
    mutable std::atomic<int> m_bufferUnderruns{0};
    mutable std::atomic<int> m_bufferOverruns{0};
    mutable std::atomic<float> m_peakInputLevel{0.0f};
    mutable std::atomic<float> m_peakOutputLevel{0.0f};
    mutable std::atomic<int> m_droppedSamples{0};

    // Performance monitoring
    mutable std::mutex m_perfMutex;
    std::chrono::steady_clock::time_point m_lastCallbackTime;
    std::chrono::microseconds m_maxCallbackDuration{0};
    std::chrono::microseconds m_avgCallbackDuration{0};
    std::vector<std::chrono::microseconds> m_callbackHistory;
    size_t m_callbackHistoryIndex{0};
    static constexpr size_t CALLBACK_HISTORY_SIZE = 1000;

    // Platform-specific handles
#ifdef _WIN32
    HANDLE m_avrtHandle{nullptr};
    DWORD m_taskIndex{0};
#elif defined(__linux__)
    bool m_realtimeScheduled{false};
#elif defined(__APPLE__)
    thread_time_constraint_policy_data_t m_timeConstraints{};
    bool m_timeConstraintsSet{false};
#endif

    // Thread synchronization
    std::mutex m_streamMutex;
    std::condition_variable m_streamCondition;
    std::thread m_monitorThread;
    std::atomic<bool> m_monitorRunning{false};

    // Mock backend state
    std::vector<DeviceInfo> m_mockInputDevices;
    std::vector<DeviceInfo> m_mockOutputDevices;
    std::thread m_mockProcessingThread;
    std::atomic<bool> m_mockProcessingRunning{false};
    std::chrono::steady_clock::time_point m_mockLastProcessTime;

    // Virtual device state
    std::atomic<bool> m_virtualDeviceCreated{false};
    std::string m_virtualDevicePath;  // Platform-specific virtual device identifier


    // Audio level monitoring for VR overlay
    mutable std::mutex m_levelMutex;
    std::atomic<float> m_inputLevel{0.0f};
    std::atomic<float> m_outputLevelLeft{0.0f};
    std::atomic<float> m_outputLevelRight{0.0f};
#ifdef __linux__
    pid_t m_pulseModulePid{-1};       // PulseAudio module PID for cleanup
    std::string m_pulseModuleName;    // Module name for removal
#endif
};

} // namespace vrb

// Platform-specific utilities
namespace vrb::audio_utils {
    /**
     * @brief Convert PortAudio format to internal format
     */
    AudioEngine::AudioFormat PAFormatToInternal(PaSampleFormat format);

    /**
     * @brief Convert internal format to PortAudio format
     */
    PaSampleFormat InternalToPAFormat(AudioEngine::AudioFormat format);

    /**
     * @brief Get sample size in bytes for given format
     */
    size_t GetSampleSize(AudioEngine::AudioFormat format);

    /**
     * @brief Check if sample rate is supported
     */
    bool IsSampleRateSupported(int deviceIndex, double sampleRate);

    /**
     * @brief Get optimal buffer size for device
     */
    int GetOptimalBufferSize(int deviceIndex, int targetLatencyMs);

    /**
     * @brief Setup platform-specific real-time audio threading
     */
    bool SetupRealtimeThread();
}