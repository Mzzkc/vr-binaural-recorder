// windows_wasapi_virtual_device.h - Windows WASAPI Virtual Audio Device Implementation
// Creates a virtual audio device for binaural output on Windows

#pragma once

#ifdef _WIN32

#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <functiondiscoverykeys_devpkey.h>
#include <propvarutil.h>
#include <wrl/client.h>
#include <atomic>
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "propsys.lib")

namespace vrb {

/**
 * @brief Windows WASAPI Virtual Audio Device for VR Binaural Output
 *
 * This class creates and manages a virtual audio device on Windows that appears
 * in the Windows Sound settings. Applications can route audio to this device,
 * and the VR Binaural Recorder will process it through HRTF for spatial audio.
 *
 * Features:
 * - Creates virtual audio endpoint visible in Windows Sound
 * - Supports exclusive and shared mode operation
 * - Real-time audio streaming with low latency
 * - Automatic device discovery and enumeration
 * - Integration with Windows Audio Session API (WASAPI)
 * - Support for multiple sample rates and formats
 */
class WindowsWASAPIVirtualDevice {
public:
    struct DeviceConfig {
        std::wstring deviceName = L"VR Binaural Recorder";
        std::wstring deviceDescription = L"Virtual Binaural Audio Output for VR";
        UINT32 sampleRate = 48000;
        UINT32 channels = 2;
        WORD bitsPerSample = 32;  // 32-bit float
        UINT32 bufferFrames = 512;
        bool exclusiveMode = false;
        bool lowLatencyMode = true;
    };

    enum class DeviceState {
        Uninitialized,
        Created,
        Active,
        Error
    };

    WindowsWASAPIVirtualDevice();
    ~WindowsWASAPIVirtualDevice();

    /**
     * @brief Initialize the virtual audio device
     * @param config Device configuration
     * @return true if successful
     */
    bool Initialize(const DeviceConfig& config);

    /**
     * @brief Start the virtual audio device
     * @return true if successful
     */
    bool Start();

    /**
     * @brief Stop the virtual audio device
     */
    void Stop();

    /**
     * @brief Shutdown and cleanup the virtual device
     */
    void Shutdown();

    /**
     * @brief Get current device state
     */
    DeviceState GetState() const { return m_state; }

    /**
     * @brief Check if device is active
     */
    bool IsActive() const { return m_state == DeviceState::Active; }

    /**
     * @brief Get device endpoint ID
     */
    std::wstring GetDeviceId() const { return m_deviceId; }

    /**
     * @brief Set audio data callback
     * @param callback Function to call when audio data is available
     */
    void SetAudioCallback(std::function<void(const float*, UINT32)> callback);

    /**
     * @brief Write audio data to the virtual device
     * @param audioData Pointer to audio samples (32-bit float)
     * @param frameCount Number of frames to write
     * @return Number of frames actually written
     */
    UINT32 WriteAudioData(const float* audioData, UINT32 frameCount);

    /**
     * @brief Get current latency in milliseconds
     */
    double GetCurrentLatency() const;

    /**
     * @brief Get buffer usage percentage
     */
    float GetBufferUsage() const;

    /**
     * @brief Check if exclusive mode is available
     */
    bool IsExclusiveModeSupported() const;

    /**
     * @brief Set device volume (0.0 to 1.0)
     */
    bool SetVolume(float volume);

    /**
     * @brief Get current device volume
     */
    float GetVolume() const;

    /**
     * @brief Mute or unmute the device
     */
    bool SetMute(bool mute);

    /**
     * @brief Check if device is muted
     */
    bool IsMuted() const;

private:
    /**
     * @brief Create the virtual audio endpoint
     */
    bool CreateVirtualEndpoint();

    /**
     * @brief Initialize WASAPI audio client
     */
    bool InitializeAudioClient();

    /**
     * @brief Setup audio rendering thread
     */
    bool SetupRenderThread();

    /**
     * @brief Audio rendering thread function
     */
    void RenderThreadFunction();

    /**
     * @brief Handle WASAPI events
     */
    void HandleAudioClientEvents();

    /**
     * @brief Convert internal format to WAVEFORMATEX
     */
    WAVEFORMATEX* CreateWaveFormat() const;

    /**
     * @brief Calculate buffer size based on latency requirements
     */
    UINT32 CalculateBufferSize() const;

    /**
     * @brief Handle audio client disconnect
     */
    void HandleDeviceDisconnect();

    /**
     * @brief Register for device notifications
     */
    bool RegisterDeviceNotifications();

    /**
     * @brief Unregister device notifications
     */
    void UnregisterDeviceNotifications();

    // Configuration
    DeviceConfig m_config;
    std::atomic<DeviceState> m_state{DeviceState::Uninitialized};

    // WASAPI interfaces
    Microsoft::WRL::ComPtr<IMMDeviceEnumerator> m_deviceEnumerator;
    Microsoft::WRL::ComPtr<IMMDevice> m_device;
    Microsoft::WRL::ComPtr<IAudioClient> m_audioClient;
    Microsoft::WRL::ComPtr<IAudioRenderClient> m_renderClient;
    Microsoft::WRL::ComPtr<IAudioSessionControl> m_sessionControl;
    Microsoft::WRL::ComPtr<ISimpleAudioVolume> m_volumeControl;
    Microsoft::WRL::ComPtr<IAudioEndpointVolume> m_endpointVolume;

    // Device identification
    std::wstring m_deviceId;
    std::wstring m_deviceFriendlyName;

    // Audio format
    WAVEFORMATEX* m_waveFormat;
    UINT32 m_bufferFrameCount;
    UINT32 m_bytesPerFrame;

    // Threading
    std::thread m_renderThread;
    std::atomic<bool> m_renderThreadRunning{false};
    HANDLE m_audioSampleReadyEvent;
    HANDLE m_shutdownEvent;

    // Audio callback
    std::function<void(const float*, UINT32)> m_audioCallback;
    std::mutex m_callbackMutex;

    // Performance monitoring
    mutable std::atomic<double> m_currentLatency{0.0};
    mutable std::atomic<float> m_bufferUsage{0.0f};
    mutable std::atomic<UINT32> m_framesProcessed{0};
    mutable std::atomic<UINT32> m_underruns{0};

    // Error handling
    std::atomic<HRESULT> m_lastError{S_OK};
    std::string m_lastErrorMessage;
    std::mutex m_errorMutex;

    // Device notifications
    class DeviceNotificationClient;
    std::unique_ptr<DeviceNotificationClient> m_notificationClient;

public:
    /**
     * @brief Get available audio devices on the system
     */
    static std::vector<DeviceConfig> EnumerateAudioDevices();

    /**
     * @brief Check if WASAPI is available
     */
    static bool IsWASAPISupported();

    /**
     * @brief Get default audio device ID
     */
    static std::wstring GetDefaultDeviceId();

    /**
     * @brief Create a virtual audio device with automatic configuration
     */
    static std::unique_ptr<WindowsWASAPIVirtualDevice> CreateAutoConfigured();

    /**
     * @brief Get error description for HRESULT
     */
    static std::string GetErrorDescription(HRESULT hr);
};

/**
 * @brief Device notification client for handling audio device events
 */
class WindowsWASAPIVirtualDevice::DeviceNotificationClient : public IMMNotificationClient {
public:
    DeviceNotificationClient(WindowsWASAPIVirtualDevice* parent);
    virtual ~DeviceNotificationClient();

    // IUnknown
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID** ppvInterface) override;

    // IMMNotificationClient
    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) override;
    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId) override;
    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId) override;
    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId) override;
    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) override;

private:
    WindowsWASAPIVirtualDevice* m_parent;
    std::atomic<ULONG> m_refCount{1};
};

} // namespace vrb

#endif // _WIN32