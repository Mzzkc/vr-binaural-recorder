# CORRECTED INDUSTRY-STANDARD ARCHITECTURE DESIGN

## Executive Summary

This document defines the **correct architecture** for a VR spatial audio routing/transformation system, replacing the incorrectly implemented recording application with a proper real-time audio processing solution following industry standards.

## Core Use Case Definition

### Primary Function: Real-Time Audio Routing & Transformation
1. **Input**: Live audio from microphones or applications
2. **Process**: Apply spatial audio transformations based on VR tracking
3. **Output**: Route processed audio to virtual audio device
4. **Control**: VR overlay interface for real-time parameter adjustment

### Target Workflow
```
Game Audio → VR Audio Routing System → Virtual Audio Device → Streaming Software
                    ↑
                VR Tracking Data (HMD + Controllers)
                    ↑
                VR Overlay Controls
```

## Corrected Architecture Overview

### Layer 1: Virtual Audio Device Layer
**Responsibility**: Create system-level virtual audio device that other applications can use

```cpp
class VirtualAudioDevice {
public:
    // Platform-specific virtual device creation
    bool CreateDevice(const std::string& deviceName);
    bool DestroyDevice();

    // Real-time audio output to system
    bool WriteAudioFrame(const float* leftChannel, const float* rightChannel, size_t frames);

    // Device state management
    bool IsActive() const;
    AudioFormat GetFormat() const;

private:
#ifdef _WIN32
    // WASAPI virtual device implementation
    std::unique_ptr<WASAPIVirtualDevice> m_wasapiDevice;
#elif __linux__
    // PulseAudio null-sink implementation
    std::unique_ptr<PulseAudioDevice> m_pulseDevice;
#elif __APPLE__
    // Core Audio aggregate device implementation
    std::unique_ptr<CoreAudioDevice> m_coreDevice;
#endif
};
```

### Layer 2: Real-Time Audio Processing Engine
**Responsibility**: Process audio in real-time with spatial transformations

```cpp
class SpatialAudioEngine {
public:
    // Core processing interface
    bool Initialize(int sampleRate, int bufferSize);
    void ProcessAudioBlock(const float* input, float* outputL, float* outputR, size_t frames);

    // Spatial positioning
    void UpdateListenerPose(const VRPose& hmdPose);
    void UpdateSourcePose(const VRPose& sourcePose);

    // Real-time parameter control
    void SetSpatialMix(float mix);           // 0.0 = mono, 1.0 = full spatial
    void SetHRTFEnabled(bool enabled);
    void SetDistanceAttenuation(float factor);

private:
    std::unique_ptr<HRTFProcessor> m_hrtf;
    std::unique_ptr<VirtualAudioDevice> m_outputDevice;

    // High-performance circular buffers
    RingBuffer<float> m_inputBuffer;
    RingBuffer<float> m_outputBufferL;
    RingBuffer<float> m_outputBufferR;

    // Real-time safe state
    std::atomic<float> m_spatialMix{1.0f};
    std::atomic<bool> m_hrtfEnabled{true};
};
```

### Layer 3: VR Integration Layer
**Responsibility**: Track VR poses and provide overlay controls

```cpp
class VRSpatialController {
public:
    bool Initialize();
    void Update();  // Call each frame

    // Tracking integration
    VRPose GetHMDPose() const;
    VRPose GetControllerPose(int controllerIndex) const;

    // Audio engine connection
    void ConnectAudioEngine(SpatialAudioEngine* engine);

private:
    std::unique_ptr<VRTracker> m_tracker;           // Simple OpenVR wrapper
    std::unique_ptr<VROverlayUI> m_overlay;         // Minimal control interface
    SpatialAudioEngine* m_audioEngine;
};
```

### Layer 4: Minimal VR Overlay Interface
**Responsibility**: Real-time controls accessible during gameplay

```cpp
class VROverlayUI {
public:
    bool Initialize();
    void Update();

    // Simple real-time controls only
    void RenderControls();

private:
    // Control elements (no recording session management!)
    float m_spatialMix = 1.0f;
    bool m_hrtfEnabled = true;
    float m_distanceAttenuation = 1.0f;
    bool m_bypassProcessing = false;

    // VR overlay state
    vr::VROverlayHandle_t m_overlayHandle;
    GLuint m_framebuffer;
    GLuint m_texture;
};
```

## Platform-Specific Virtual Audio Device Implementation

### Windows Implementation (WASAPI)
```cpp
class WASAPIVirtualDevice {
public:
    bool CreateVirtualDevice(const std::string& deviceName);
    bool WriteAudioFrame(const float* leftChannel, const float* rightChannel, size_t frames);

private:
    // Create WASAPI loopback capture device
    // Applications can select this as their audio output
    Microsoft::WRL::ComPtr<IAudioClient> m_audioClient;
    Microsoft::WRL::ComPtr<IAudioRenderClient> m_renderClient;
    Microsoft::WRL::ComPtr<IMMDevice> m_virtualDevice;
};
```

### Linux Implementation (PulseAudio)
```cpp
class PulseAudioDevice {
public:
    bool CreateNullSink(const std::string& sinkName);
    bool WriteAudioFrame(const float* leftChannel, const float* rightChannel, size_t frames);

private:
    // Create PulseAudio null-sink that applications can use as output
    pa_simple* m_pulseConnection;
    pa_sample_spec m_sampleSpec;
    std::string m_sinkName;
};
```

### macOS Implementation (Core Audio)
```cpp
class CoreAudioDevice {
public:
    bool CreateAggregateDevice(const std::string& deviceName);
    bool WriteAudioFrame(const float* leftChannel, const float* rightChannel, size_t frames);

private:
    // Create Core Audio aggregate device
    AudioDeviceID m_aggregateDevice;
    AudioObjectID m_virtualDevice;
};
```

## Simplified Application Architecture

### Main Application (Minimal)
```cpp
int main() {
    // Initialize components
    SpatialAudioEngine audioEngine;
    VRSpatialController vrController;

    if (!audioEngine.Initialize(48000, 128)) {
        return 1; // Failed to create virtual audio device
    }

    if (!vrController.Initialize()) {
        LOG_WARN("VR not available - running in desktop mode");
    }

    // Connect VR to audio processing
    vrController.ConnectAudioEngine(&audioEngine);

    // Main processing loop (real-time)
    while (running) {
        vrController.Update();  // Update VR poses and overlay

        // Audio processing happens in callback
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60fps
    }

    return 0;
}
```

### Audio Callback (Real-Time Critical)
```cpp
void AudioCallback(const float* input, float* output, size_t frames) {
    // Get latest VR poses (lock-free)
    VRPose hmdPose = vrController.GetHMDPose();
    VRPose micPose = vrController.GetControllerPose(0);

    // Process spatial audio (real-time safe)
    audioEngine.UpdateListenerPose(hmdPose);
    audioEngine.UpdateSourcePose(micPose);
    audioEngine.ProcessAudioBlock(input, output, output + frames, frames);

    // Output to virtual device (no file I/O!)
    virtualDevice.WriteAudioFrame(output, output + frames, frames);
}
```

## Configuration Simplification

### Corrected Configuration Structure
```json
{
  "audioRouting": {
    "virtualDeviceName": "VR Spatial Audio",
    "sampleRate": 48000,
    "bufferSize": 128,
    "enableHRTF": true,
    "enableLowLatencyMode": true
  },

  "vrIntegration": {
    "overlayScale": 0.3,
    "overlayPosition": [0.0, 1.5, -0.8],
    "enableHeadTracking": true,
    "enableControllerTracking": true
  },

  "spatialAudio": {
    "hrtfDataset": "mit_kemar",
    "distanceAttenuation": true,
    "maxDistance": 10.0,
    "spatialMixDefault": 1.0
  },

  "performance": {
    "threadPriority": "realtime",
    "enableOptimizations": true,
    "targetLatency": 10.0
  }
}
```

## Module Structure Simplification

### Corrected Directory Structure
```
src/
├── core/
│   ├── spatial_audio_engine.h/cpp     // Core processing engine
│   └── virtual_audio_device.h/cpp     // Platform virtual device creation
├── vr/
│   ├── vr_tracker.h/cpp              // Simple OpenVR wrapper (minimal)
│   └── vr_overlay_ui.h/cpp           // Basic VR controls only
├── platform/
│   ├── windows_wasapi.h/cpp          // Windows virtual device
│   ├── linux_pulseaudio.h/cpp        // Linux virtual device
│   └── macos_coreaudio.h/cpp         // macOS virtual device
└── utils/
    ├── ring_buffer.h                 // Lock-free circular buffer
    └── logger.h/cpp                  // Minimal logging
```

**Total Target**: <5,000 lines of focused code (vs. current 30,000+)

## Industry Standard Compliance

### 1. Audio Routing Applications Pattern
- **VoiceMeeter**: Virtual audio mixer with real-time routing
- **Virtual Audio Cable**: Simple audio routing between applications
- **OBS Audio**: Real-time audio processing with virtual device output

### 2. WASAPI/Core Audio/JACK Standards
- Use platform-provided virtual device APIs
- Implement proper audio callback handling
- Follow real-time audio processing guidelines

### 3. VR Overlay Application Standards
- Minimal UI during gameplay
- Real-time parameter adjustment
- No session management or file operations

## Performance Requirements

### Real-Time Constraints
- **Audio Latency**: <10ms total system latency
- **VR Update Rate**: 90fps minimum for overlay
- **Memory Allocation**: Zero allocations in audio callback
- **CPU Usage**: <20% on target hardware

### System Resource Usage
- **Memory**: <100MB total application memory
- **CPU**: Single core for audio processing, separate thread for VR
- **GPU**: Minimal for simple overlay rendering

## Migration Strategy from Current Codebase

### Phase 1: Remove Recording Functionality
1. Remove all recording session management code
2. Remove file I/O from audio engine
3. Remove recording UI components
4. Remove recording configuration options

### Phase 2: Implement Virtual Audio Device
1. Create platform-specific virtual device implementations
2. Integrate with existing audio engine
3. Test with external applications (OBS, Discord, etc.)

### Phase 3: Simplify VR Integration
1. Replace complex VR modules with simple OpenVR wrapper
2. Create minimal overlay UI with real-time controls only
3. Remove microphone positioning system

### Phase 4: Optimize for Real-Time Performance
1. Implement lock-free audio processing
2. Optimize HRTF processing for real-time
3. Add performance monitoring and adaptive quality

## Success Criteria

### Functional Requirements
1. ✅ Other applications can select our virtual audio device as output
2. ✅ Real-time spatial audio processing with VR tracking
3. ✅ VR overlay controls work during gameplay
4. ✅ <10ms total audio latency
5. ✅ Stable operation for hours without degradation

### Technical Requirements
1. ✅ <5,000 lines of focused code
2. ✅ Zero memory allocations in audio callback
3. ✅ Platform-specific virtual device implementation
4. ✅ Industry-standard audio routing patterns
5. ✅ OpenVR integration without custom tracking

### User Experience Requirements
1. ✅ Install and use immediately (no complex setup)
2. ✅ Works with existing applications (OBS, Discord, etc.)
3. ✅ VR controls accessible during gameplay
4. ✅ Minimal CPU/GPU impact on games
5. ✅ Automatic VR tracking integration

---

**CONCLUSION**: This corrected architecture focuses on the actual requirement - real-time audio routing and transformation - rather than the incorrectly implemented recording application. The design follows industry standards and provides a clear migration path from the current codebase.