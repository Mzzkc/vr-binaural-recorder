// windows_wasapi_virtual_device.cpp - Windows WASAPI Virtual Audio Device Implementation

#ifdef _WIN32

#include "windows_wasapi_virtual_device.h"
#include <spdlog/spdlog.h>
#include <comdef.h>
#include <objbase.h>
#include <avrt.h>

#pragma comment(lib, "avrt.lib")

namespace vrb {

WindowsWASAPIVirtualDevice::WindowsWASAPIVirtualDevice()
    : m_waveFormat(nullptr)
    , m_bufferFrameCount(0)
    , m_bytesPerFrame(0)
    , m_audioSampleReadyEvent(nullptr)
    , m_shutdownEvent(nullptr)
{
    // Initialize COM
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        spdlog::error("Failed to initialize COM: {}", GetErrorDescription(hr));
    }
}

WindowsWASAPIVirtualDevice::~WindowsWASAPIVirtualDevice()
{
    Shutdown();
    CoUninitialize();
}

bool WindowsWASAPIVirtualDevice::Initialize(const DeviceConfig& config)
{
    if (m_state != DeviceState::Uninitialized) {
        spdlog::warn("Virtual device already initialized");
        return false;
    }

    m_config = config;

    // Create event objects
    m_audioSampleReadyEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    m_shutdownEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    if (!m_audioSampleReadyEvent || !m_shutdownEvent) {
        spdlog::error("Failed to create event objects");
        return false;
    }

    // Create device enumerator
    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        reinterpret_cast<void**>(m_deviceEnumerator.GetAddressOf())
    );

    if (FAILED(hr)) {
        spdlog::error("Failed to create device enumerator: {}", GetErrorDescription(hr));
        return false;
    }

    // Try to create virtual endpoint or use default output device
    if (!CreateVirtualEndpoint()) {
        spdlog::warn("Failed to create virtual endpoint, using default output device");

        // Fall back to default output device
        hr = m_deviceEnumerator->GetDefaultAudioEndpoint(
            eRender, eConsole, m_device.GetAddressOf());

        if (FAILED(hr)) {
            spdlog::error("Failed to get default audio endpoint: {}", GetErrorDescription(hr));
            return false;
        }
    }

    // Initialize audio client
    if (!InitializeAudioClient()) {
        spdlog::error("Failed to initialize audio client");
        return false;
    }

    // Register for device notifications
    RegisterDeviceNotifications();

    m_state = DeviceState::Created;
    spdlog::info("WASAPI virtual device initialized successfully");
    return true;
}

bool WindowsWASAPIVirtualDevice::Start()
{
    if (m_state != DeviceState::Created) {
        spdlog::error("Virtual device not in created state");
        return false;
    }

    // Start the audio client
    HRESULT hr = m_audioClient->Start();
    if (FAILED(hr)) {
        spdlog::error("Failed to start audio client: {}", GetErrorDescription(hr));
        return false;
    }

    // Setup and start render thread
    if (!SetupRenderThread()) {
        m_audioClient->Stop();
        return false;
    }

    m_state = DeviceState::Active;
    spdlog::info("WASAPI virtual device started successfully");
    return true;
}

void WindowsWASAPIVirtualDevice::Stop()
{
    if (m_state != DeviceState::Active) {
        return;
    }

    // Signal shutdown
    if (m_shutdownEvent) {
        SetEvent(m_shutdownEvent);
    }

    // Stop render thread
    m_renderThreadRunning = false;
    if (m_renderThread.joinable()) {
        m_renderThread.join();
    }

    // Stop audio client
    if (m_audioClient) {
        m_audioClient->Stop();
    }

    m_state = DeviceState::Created;
    spdlog::info("WASAPI virtual device stopped");
}

void WindowsWASAPIVirtualDevice::Shutdown()
{
    Stop();

    UnregisterDeviceNotifications();

    // Release interfaces
    m_endpointVolume.Reset();
    m_volumeControl.Reset();
    m_sessionControl.Reset();
    m_renderClient.Reset();
    m_audioClient.Reset();
    m_device.Reset();
    m_deviceEnumerator.Reset();

    // Close event handles
    if (m_audioSampleReadyEvent) {
        CloseHandle(m_audioSampleReadyEvent);
        m_audioSampleReadyEvent = nullptr;
    }

    if (m_shutdownEvent) {
        CloseHandle(m_shutdownEvent);
        m_shutdownEvent = nullptr;
    }

    // Free wave format
    if (m_waveFormat) {
        CoTaskMemFree(m_waveFormat);
        m_waveFormat = nullptr;
    }

    m_state = DeviceState::Uninitialized;
    spdlog::info("WASAPI virtual device shut down");
}

bool WindowsWASAPIVirtualDevice::CreateVirtualEndpoint()
{
    // Note: Creating true virtual endpoints requires Windows Driver Kit (WDK)
    // and driver development. For this implementation, we'll use the default
    // endpoint but with our custom configuration.

    // In a full implementation, this would involve:
    // 1. Installing a virtual audio driver
    // 2. Registering a new audio endpoint
    // 3. Creating device-specific registry entries

    // For now, we'll use the default output device
    HRESULT hr = m_deviceEnumerator->GetDefaultAudioEndpoint(
        eRender, eConsole, m_device.GetAddressOf());

    if (FAILED(hr)) {
        spdlog::error("Failed to get default audio endpoint for virtual device: {}",
                     GetErrorDescription(hr));
        return false;
    }

    // Get device ID
    LPWSTR deviceId = nullptr;
    hr = m_device->GetId(&deviceId);
    if (SUCCEEDED(hr)) {
        m_deviceId = deviceId;
        CoTaskMemFree(deviceId);
    }

    return true;
}

bool WindowsWASAPIVirtualDevice::InitializeAudioClient()
{
    // Get audio client
    HRESULT hr = m_device->Activate(
        __uuidof(IAudioClient),
        CLSCTX_ALL,
        nullptr,
        reinterpret_cast<void**>(m_audioClient.GetAddressOf())
    );

    if (FAILED(hr)) {
        spdlog::error("Failed to activate audio client: {}", GetErrorDescription(hr));
        return false;
    }

    // Create wave format
    m_waveFormat = CreateWaveFormat();
    if (!m_waveFormat) {
        spdlog::error("Failed to create wave format");
        return false;
    }

    // Calculate buffer size
    m_bufferFrameCount = CalculateBufferSize();
    m_bytesPerFrame = m_waveFormat->nBlockAlign;

    // Initialize audio client
    DWORD flags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK;
    if (m_config.lowLatencyMode) {
        flags |= AUDCLNT_STREAMFLAGS_RATEADJUST;
    }

    REFERENCE_TIME bufferDuration = static_cast<REFERENCE_TIME>(
        (10000000.0 * m_bufferFrameCount) / m_config.sampleRate + 0.5);

    hr = m_audioClient->Initialize(
        m_config.exclusiveMode ? AUDCLNT_SHAREMODE_EXCLUSIVE : AUDCLNT_SHAREMODE_SHARED,
        flags,
        bufferDuration,
        m_config.exclusiveMode ? bufferDuration : 0,
        m_waveFormat,
        nullptr
    );

    if (FAILED(hr)) {
        spdlog::error("Failed to initialize audio client: {}", GetErrorDescription(hr));
        return false;
    }

    // Get actual buffer size
    hr = m_audioClient->GetBufferSize(&m_bufferFrameCount);
    if (FAILED(hr)) {
        spdlog::error("Failed to get buffer size: {}", GetErrorDescription(hr));
        return false;
    }

    // Set event handle
    hr = m_audioClient->SetEventHandle(m_audioSampleReadyEvent);
    if (FAILED(hr)) {
        spdlog::error("Failed to set event handle: {}", GetErrorDescription(hr));
        return false;
    }

    // Get render client
    hr = m_audioClient->GetService(
        __uuidof(IAudioRenderClient),
        reinterpret_cast<void**>(m_renderClient.GetAddressOf())
    );

    if (FAILED(hr)) {
        spdlog::error("Failed to get render client: {}", GetErrorDescription(hr));
        return false;
    }

    // Get session control
    hr = m_audioClient->GetService(
        __uuidof(IAudioSessionControl),
        reinterpret_cast<void**>(m_sessionControl.GetAddressOf())
    );

    if (FAILED(hr)) {
        spdlog::warn("Failed to get session control: {}", GetErrorDescription(hr));
    }

    // Get volume control
    hr = m_audioClient->GetService(
        __uuidof(ISimpleAudioVolume),
        reinterpret_cast<void**>(m_volumeControl.GetAddressOf())
    );

    if (FAILED(hr)) {
        spdlog::warn("Failed to get volume control: {}", GetErrorDescription(hr));
    }

    spdlog::info("Audio client initialized - Buffer: {} frames, Sample Rate: {} Hz",
                m_bufferFrameCount, m_config.sampleRate);

    return true;
}

bool WindowsWASAPIVirtualDevice::SetupRenderThread()
{
    m_renderThreadRunning = true;
    m_renderThread = std::thread(&WindowsWASAPIVirtualDevice::RenderThreadFunction, this);

    // Set thread priority
    HANDLE threadHandle = m_renderThread.native_handle();
    if (!SetThreadPriority(threadHandle, THREAD_PRIORITY_TIME_CRITICAL)) {
        spdlog::warn("Failed to set render thread priority");
    }

    return true;
}

void WindowsWASAPIVirtualDevice::RenderThreadFunction()
{
    // Set thread name for debugging
    SetThreadDescription(GetCurrentThread(), L"VRB Audio Render");

    // Register with MMCSS for pro-audio
    DWORD taskIndex = 0;
    HANDLE mmcssHandle = AvSetMmThreadCharacteristics(L"Pro Audio", &taskIndex);
    if (!mmcssHandle) {
        spdlog::warn("Failed to register with MMCSS");
    }

    HANDLE waitArray[2] = { m_shutdownEvent, m_audioSampleReadyEvent };

    spdlog::info("Audio render thread started");

    while (m_renderThreadRunning) {
        DWORD waitResult = WaitForMultipleObjects(2, waitArray, FALSE, INFINITE);

        if (waitResult == WAIT_OBJECT_0) {
            // Shutdown event
            break;
        } else if (waitResult == WAIT_OBJECT_0 + 1) {
            // Audio sample ready event
            HandleAudioClientEvents();
        }
    }

    // Clean up MMCSS
    if (mmcssHandle) {
        AvRevertMmThreadCharacteristics(mmcssHandle);
    }

    spdlog::info("Audio render thread stopped");
}

void WindowsWASAPIVirtualDevice::HandleAudioClientEvents()
{
    UINT32 numFramesPadding = 0;
    HRESULT hr = m_audioClient->GetCurrentPadding(&numFramesPadding);
    if (FAILED(hr)) {
        return;
    }

    UINT32 numFramesAvailable = m_bufferFrameCount - numFramesPadding;
    if (numFramesAvailable == 0) {
        return;
    }

    // Get buffer
    BYTE* data = nullptr;
    hr = m_renderClient->GetBuffer(numFramesAvailable, &data);
    if (FAILED(hr)) {
        m_underruns++;
        return;
    }

    // Fill buffer with audio data
    if (m_audioCallback && data) {
        std::lock_guard<std::mutex> lock(m_callbackMutex);

        // Convert to float pointer for callback
        float* floatData = reinterpret_cast<float*>(data);
        m_audioCallback(floatData, numFramesAvailable);
    } else {
        // Fill with silence
        memset(data, 0, numFramesAvailable * m_bytesPerFrame);
    }

    // Release buffer
    hr = m_renderClient->ReleaseBuffer(numFramesAvailable, 0);
    if (FAILED(hr)) {
        spdlog::error("Failed to release buffer: {}", GetErrorDescription(hr));
    }

    // Update statistics
    m_framesProcessed += numFramesAvailable;
    m_bufferUsage = static_cast<float>(numFramesPadding) / m_bufferFrameCount;
}

WAVEFORMATEX* WindowsWASAPIVirtualDevice::CreateWaveFormat() const
{
    WAVEFORMATEX* format = reinterpret_cast<WAVEFORMATEX*>(
        CoTaskMemAlloc(sizeof(WAVEFORMATEX)));

    if (!format) {
        return nullptr;
    }

    format->wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    format->nChannels = static_cast<WORD>(m_config.channels);
    format->nSamplesPerSec = m_config.sampleRate;
    format->wBitsPerSample = 32; // 32-bit float
    format->nBlockAlign = (format->nChannels * format->wBitsPerSample) / 8;
    format->nAvgBytesPerSec = format->nSamplesPerSec * format->nBlockAlign;
    format->cbSize = 0;

    return format;
}

UINT32 WindowsWASAPIVirtualDevice::CalculateBufferSize() const
{
    // Target latency in milliseconds
    const double targetLatencyMs = m_config.lowLatencyMode ? 10.0 : 20.0;

    UINT32 bufferSize = static_cast<UINT32>(
        (m_config.sampleRate * targetLatencyMs) / 1000.0);

    // Ensure buffer size is power of 2 for efficiency
    UINT32 powerOf2 = 1;
    while (powerOf2 < bufferSize) {
        powerOf2 *= 2;
    }

    return powerOf2;
}

void WindowsWASAPIVirtualDevice::SetAudioCallback(std::function<void(const float*, UINT32)> callback)
{
    std::lock_guard<std::mutex> lock(m_callbackMutex);
    m_audioCallback = callback;
}

UINT32 WindowsWASAPIVirtualDevice::WriteAudioData(const float* audioData, UINT32 frameCount)
{
    // This would be used for input mode - not implemented in this render-only version
    return 0;
}

double WindowsWASAPIVirtualDevice::GetCurrentLatency() const
{
    if (!m_audioClient) {
        return 0.0;
    }

    REFERENCE_TIME latency = 0;
    HRESULT hr = m_audioClient->GetStreamLatency(&latency);
    if (SUCCEEDED(hr)) {
        return static_cast<double>(latency) / 10000.0; // Convert to milliseconds
    }

    return 0.0;
}

float WindowsWASAPIVirtualDevice::GetBufferUsage() const
{
    return m_bufferUsage.load();
}

bool WindowsWASAPIVirtualDevice::RegisterDeviceNotifications()
{
    if (!m_deviceEnumerator) {
        return false;
    }

    m_notificationClient = std::make_unique<DeviceNotificationClient>(this);

    HRESULT hr = m_deviceEnumerator->RegisterEndpointNotificationCallback(
        m_notificationClient.get());

    if (FAILED(hr)) {
        spdlog::warn("Failed to register device notifications: {}", GetErrorDescription(hr));
        return false;
    }

    return true;
}

void WindowsWASAPIVirtualDevice::UnregisterDeviceNotifications()
{
    if (m_deviceEnumerator && m_notificationClient) {
        m_deviceEnumerator->UnregisterEndpointNotificationCallback(
            m_notificationClient.get());
    }
    m_notificationClient.reset();
}

// Static methods
std::vector<WindowsWASAPIVirtualDevice::DeviceConfig> WindowsWASAPIVirtualDevice::EnumerateAudioDevices()
{
    std::vector<DeviceConfig> devices;

    Microsoft::WRL::ComPtr<IMMDeviceEnumerator> enumerator;
    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        reinterpret_cast<void**>(enumerator.GetAddressOf())
    );

    if (FAILED(hr)) {
        return devices;
    }

    Microsoft::WRL::ComPtr<IMMDeviceCollection> collection;
    hr = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE,
                                       collection.GetAddressOf());

    if (FAILED(hr)) {
        return devices;
    }

    UINT count = 0;
    collection->GetCount(&count);

    for (UINT i = 0; i < count; i++) {
        Microsoft::WRL::ComPtr<IMMDevice> device;
        hr = collection->Item(i, device.GetAddressOf());

        if (SUCCEEDED(hr)) {
            DeviceConfig config;

            // Get device friendly name
            Microsoft::WRL::ComPtr<IPropertyStore> propertyStore;
            hr = device->OpenPropertyStore(STGM_READ, propertyStore.GetAddressOf());

            if (SUCCEEDED(hr)) {
                PROPVARIANT varName;
                PropVariantInit(&varName);

                hr = propertyStore->GetValue(PKEY_Device_FriendlyName, &varName);
                if (SUCCEEDED(hr) && varName.vt == VT_LPWSTR) {
                    config.deviceName = varName.pwszVal;
                }

                PropVariantClear(&varName);
            }

            devices.push_back(config);
        }
    }

    return devices;
}

bool WindowsWASAPIVirtualDevice::IsWASAPISupported()
{
    Microsoft::WRL::ComPtr<IMMDeviceEnumerator> enumerator;
    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        reinterpret_cast<void**>(enumerator.GetAddressOf())
    );

    return SUCCEEDED(hr);
}

std::string WindowsWASAPIVirtualDevice::GetErrorDescription(HRESULT hr)
{
    _com_error err(hr);
    std::wstring wideStr = err.ErrorMessage();
    return std::string(wideStr.begin(), wideStr.end());
}

// DeviceNotificationClient implementation
WindowsWASAPIVirtualDevice::DeviceNotificationClient::DeviceNotificationClient(
    WindowsWASAPIVirtualDevice* parent)
    : m_parent(parent)
{
}

WindowsWASAPIVirtualDevice::DeviceNotificationClient::~DeviceNotificationClient()
{
}

ULONG STDMETHODCALLTYPE WindowsWASAPIVirtualDevice::DeviceNotificationClient::AddRef()
{
    return ++m_refCount;
}

ULONG STDMETHODCALLTYPE WindowsWASAPIVirtualDevice::DeviceNotificationClient::Release()
{
    ULONG result = --m_refCount;
    if (result == 0) {
        delete this;
    }
    return result;
}

HRESULT STDMETHODCALLTYPE WindowsWASAPIVirtualDevice::DeviceNotificationClient::QueryInterface(
    REFIID riid, VOID** ppvInterface)
{
    if (__uuidof(IUnknown) == riid) {
        AddRef();
        *ppvInterface = static_cast<IUnknown*>(this);
    } else if (__uuidof(IMMNotificationClient) == riid) {
        AddRef();
        *ppvInterface = static_cast<IMMNotificationClient*>(this);
    } else {
        *ppvInterface = nullptr;
        return E_NOINTERFACE;
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WindowsWASAPIVirtualDevice::DeviceNotificationClient::OnDeviceStateChanged(
    LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
    if (m_parent && m_parent->m_deviceId == pwstrDeviceId) {
        if (dwNewState == DEVICE_STATE_UNPLUGGED || dwNewState == DEVICE_STATE_DISABLED) {
            m_parent->HandleDeviceDisconnect();
        }
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WindowsWASAPIVirtualDevice::DeviceNotificationClient::OnDeviceAdded(
    LPCWSTR pwstrDeviceId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WindowsWASAPIVirtualDevice::DeviceNotificationClient::OnDeviceRemoved(
    LPCWSTR pwstrDeviceId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WindowsWASAPIVirtualDevice::DeviceNotificationClient::OnDefaultDeviceChanged(
    EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WindowsWASAPIVirtualDevice::DeviceNotificationClient::OnPropertyValueChanged(
    LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
{
    return S_OK;
}

void WindowsWASAPIVirtualDevice::HandleDeviceDisconnect()
{
    spdlog::warn("Audio device disconnected, stopping virtual device");
    Stop();
    m_state = DeviceState::Error;
}

} // namespace vrb

#endif // _WIN32