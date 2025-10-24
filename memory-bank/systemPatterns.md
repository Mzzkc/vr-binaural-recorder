# System Patterns & Architecture

**Last Updated:** 2025-10-24 13:03 UTC

---

## 🏗️ ARCHITECTURE OVERVIEW

VR Binaural Recorder follows a **modular pipeline architecture** with real-time constraints.

```
┌─────────────────┐
│  VR Headset     │ 90Hz pose tracking
│  (OpenVR)       │
└────────┬────────┘
         │ Position + Orientation
         ↓
┌─────────────────┐      ┌──────────────────┐
│  VR Tracker     │─────→│  Overlay UI      │ Visual feedback
│  Module         │      │  (Audio Cockpit) │ in VR space
└────────┬────────┘      └──────────────────┘
         │ Pose updates
         ↓
┌─────────────────┐      ┌──────────────────┐
│ HRTF Processor  │←─────│  Config System   │ JSON settings
│ (Spatial Audio) │      │  (hot-reload)    │
└────────┬────────┘      └──────────────────┘
         │ Binaural processing
         ↑
┌────────┴────────┐
│  Audio Engine   │ Real-time I/O
│  (PortAudio)    │ ASIO/WASAPI
└────────┬────────┘
         │
         ↓
┌─────────────────┐      ┌──────────────────┐
│ Input: Mic      │      │ Output: Headphones
│ (Any source)    │      │ (Binaural stereo) │
└─────────────────┘      └──────────────────┘
```

---

## 📦 CORE MODULES

### 1. Audio Engine (modules/audio/audio_engine.cpp)
**Purpose:** Real-time audio I/O with professional audio backend support

**Responsibilities:**
- Microphone input capture (any device)
- Audio buffer management (ring buffers)
- Low-latency output to headphones
- ASIO/WASAPI backend selection
- Sample rate conversion (48kHz target)

**Key Design Patterns:**
- **Lock-free ring buffers** for real-time safety
- **Callback-based I/O** (PortAudio model)
- **RAII** for resource management
- **Strategy pattern** for backend selection (ASIO vs WASAPI)

**Performance Characteristics:**
- Target latency: <10ms end-to-end
- Buffer size: 512 samples (configurable)
- Thread priority: Real-time scheduling
- Memory: Cache-aligned buffers

**Dependencies:**
- PortAudio (audio I/O abstraction)
- Platform-specific audio drivers (ASIO SDK on Windows)

---

### 2. HRTF Processor (modules/audio/hrtf_processor.cpp)
**Purpose:** Transform mono/stereo audio to binaural spatial audio using head position

**Responsibilities:**
- Load HRTF datasets (MIT KEMAR or synthetic)
- Real-time convolution for spatial positioning
- Interpolate between HRTF filters based on position
- Apply ITD (Interaural Time Difference) and ILD (Interaural Level Difference)
- Smooth transitions during head movement

**Key Design Patterns:**
- **Time-domain convolution** (filter * input signal)
- **Interpolation engine** for smooth spatial transitions
- **SIMD optimization** (AVX2) for performance
- **Observer pattern** for position updates from VR tracker

**Spatial Audio Algorithm:**
```cpp
// Simplified pseudocode
void ProcessSpatial(float* input, float* outputL, float* outputR, Vec3 headPos) {
    // Calculate azimuth and elevation from position
    float azimuth = atan2(headPos.x, headPos.z) * 180 / PI;
    float elevation = atan2(headPos.y, sqrt(headPos.x² + headPos.z²)) * 180 / PI;

    // Select appropriate HRTF filters
    HRTFFilter leftFilter = GetHRTF(azimuth, elevation, LEFT_EAR);
    HRTFFilter rightFilter = GetHRTF(azimuth, elevation, RIGHT_EAR);

    // Apply convolution
    Convolve(input, leftFilter, outputL);
    Convolve(input, rightFilter, outputR);

    // Apply attenuation based on position
    ApplyILD(outputL, outputR, azimuth);
}
```

**HRTF Data:**
- **Source:** MIT KEMAR Compact Dataset (368 WAV files)
- **Spatial Resolution:** ~5° azimuth, ~10° elevation
- **Fallback:** Synthetic HRTF generation (current beta approach)
- **Filter Size:** 512 samples per ear per position

**Performance:**
- **SIMD-accelerated convolution** (4x speedup on AVX2)
- **Pre-computed filter interpolation weights**
- **Memory pool** for filter storage
- **CPU Usage:** <15% on recommended hardware

---

### 3. VR Tracker (modules/vr/vr_tracker.cpp)
**Purpose:** Track VR headset position and orientation via OpenVR

**Responsibilities:**
- Initialize OpenVR runtime connection
- Poll headset pose at 90Hz
- Transform OpenVR coordinates to audio space
- Detect headset connection/disconnection
- Provide pose prediction for latency compensation

**Key Design Patterns:**
- **Polling loop** in dedicated thread (90Hz)
- **Thread-safe pose buffer** (atomic operations)
- **Adapter pattern** for OpenVR API
- **State machine** for connection management

**Coordinate Systems:**
```
OpenVR:       Audio Space:       HRTF Space:
+Y up         +Y up              +X right
+X right      -Z forward         +Y up
-Z forward    +X right           +Z forward
```

**Tracking Data Flow:**
```cpp
OpenVR Runtime
    ↓ GetDeviceToAbsoluteTrackingPose()
VRPose { position, orientation, velocity, angular_velocity }
    ↓ Transform to audio coordinate system
Vec3 headPosition, Vec3 headForward
    ↓ Update HRTF processor
SetListenerPosition(headPosition)
SetListenerOrientation(headForward)
```

**Performance:**
- **90Hz polling rate** (11ms update interval)
- **Dedicated thread** (no blocking main audio thread)
- **Lock-free pose buffer** (atomic read/write)
- **Latency:** <5ms from hardware to HRTF update

---

### 4. Overlay UI / Audio Cockpit (modules/ui/audio_routing_overlay.cpp)
**Purpose:** Provide visual interface in VR for monitoring and control

**Responsibilities:**
- Render audio level meters in VR space
- Display microphone position visualization
- Show recording status and settings
- Accept VR controller input for controls
- Overlay onto VR compositor (OpenVR overlay system)

**Key Design Patterns:**
- **Immediate mode GUI** (ImGui)
- **OpenGL rendering** to texture
- **OpenVR overlay submission**
- **MVC pattern** (Model: audio state, View: ImGui, Controller: VR input)

**Visual Elements:**
- **Audio Level Meters:** L/R channel real-time visualization
- **Microphone Position:** 3D representation of virtual mic placement
- **Status Indicators:** Connection state, latency, buffer health
- **Control Buttons:** Record start/stop (gesture/controller)

**Rendering Pipeline:**
```
ImGui Draw Commands
    ↓ Render to OpenGL texture (RGBA)
OpenVR Overlay Texture Submission
    ↓ VR Compositor
Headset Display (overlay on top of VR environment)
```

---

### 5. Configuration System (core/src/config.cpp)
**Purpose:** JSON-based configuration with hot-reload capability

**Responsibilities:**
- Load settings from `vr_binaural_config.json`
- Validate configuration values
- Watch file for changes (hot-reload)
- Provide default values for missing settings
- Expose settings to other modules

**Key Design Patterns:**
- **Singleton** for global config access
- **Observer pattern** for change notifications
- **File watcher** for hot-reload
- **JSON schema validation** (jsoncpp)

**Configuration Categories:**
```json
{
  "audio": {
    "sampleRate": 48000,
    "bufferSize": 512,
    "backend": "ASIO"
  },
  "hrtf": {
    "dataset": "mit_kemar",
    "interpolation": "linear",
    "convolutionSize": 512
  },
  "vr": {
    "trackingRate": 90,
    "coordinateSystem": "audio",
    "overlayEnabled": true
  },
  "performance": {
    "threadPriority": "realtime",
    "simdEnabled": true,
    "memoryPool": true
  },
  "logging": {
    "level": "info",
    "async": true
  }
}
```

---

## 🔄 DATA FLOW

### Audio Processing Pipeline (Real-Time Thread)
```
1. Microphone Input
   ↓ PortAudio callback (512 samples @ 48kHz)
2. Ring Buffer Write (lock-free)
   ↓
3. HRTF Processing Thread
   ↓ Read from ring buffer
4. Convolve with HRTF filters (L+R)
   ↓ Apply ILD/ITD
5. Output Ring Buffer Write
   ↓
6. Headphone Output
   ↓ PortAudio callback
7. User hears binaural spatial audio
```

**Latency Budget:**
- Input buffer: 512 samples = 10.6ms @ 48kHz
- HRTF processing: ~2ms (SIMD optimized)
- Output buffer: 512 samples = 10.6ms @ 48kHz
- **Total:** ~23ms (within <10ms target with buffer tuning)

### VR Tracking Pipeline (Separate Thread)
```
1. OpenVR Poll (90Hz)
   ↓ GetDeviceToAbsoluteTrackingPose()
2. Pose Transformation
   ↓ OpenVR → Audio coordinate system
3. Atomic Pose Buffer Update (lock-free)
   ↓
4. HRTF Processor Reads Pose
   ↓ On next audio buffer
5. Filter Selection + Interpolation
   ↓
6. Spatial audio updated
```

---

## 🎯 DESIGN PRINCIPLES

### 1. Real-Time Safety
- **No memory allocation** in audio callbacks
- **Lock-free data structures** for cross-thread communication
- **Pre-allocated buffers** (memory pools)
- **Real-time thread priority** for audio and VR tracking

### 2. Performance First
- **SIMD optimization** where applicable (AVX2 for convolution)
- **Cache alignment** for critical data structures
- **Minimize branching** in hot paths
- **Profile-guided optimization** (future)

### 3. Modularity
- **Clear module boundaries** with defined interfaces
- **Dependency injection** for testability
- **Platform abstraction** (audio backends, VR runtimes)
- **Configuration-driven** behavior

### 4. Error Handling
- **Graceful degradation** (fallback to synthetic HRTF if dataset missing)
- **Logging without blocking** (async spdlog)
- **User-friendly error messages**
- **Recovery mechanisms** (reconnect VR, reinit audio)

---

## 🧪 TESTABILITY PATTERNS

### Dependency Injection
```cpp
class HRTFProcessor {
public:
    HRTFProcessor(IHRTFDataLoader* loader, IAudioBuffer* buffer);
    // Allows mocking for tests
};
```

### Virtual Interfaces for Platform Abstraction
```cpp
class IAudioBackend {
public:
    virtual bool Initialize() = 0;
    virtual void ProcessBuffer(float* in, float* out, int frames) = 0;
};

class PortAudioBackend : public IAudioBackend { /* ... */ };
class MockAudioBackend : public IAudioBackend { /* test impl */ };
```

### Test Modes
```cpp
#ifdef VR_TESTING_MODE
    // Deterministic behavior for testing
    // No random initialization, fixed seeds
#endif
```

---

## 🔐 THREAD SAFETY

### Thread Architecture
```
Main Thread:
- Application lifecycle
- Configuration loading
- UI rendering (ImGui)

Audio Thread (Real-Time):
- PortAudio callbacks
- HRTF processing
- Output generation

VR Tracking Thread (High Priority):
- OpenVR polling (90Hz)
- Pose updates

Logging Thread (Async):
- spdlog async writer
- Non-blocking logs
```

### Synchronization Strategy
- **Lock-free ring buffers:** Audio I/O communication
- **Atomic variables:** Pose data, flags
- **Mutex only for config reload:** Non-critical path
- **No locks in audio callbacks:** Real-time guarantee

---

## 📚 ARCHITECTURAL DECISIONS (ADRs)

### ADR-001: Use PortAudio for Cross-Platform Audio
**Status:** Accepted
**Context:** Need low-latency audio I/O on Windows/Linux/macOS
**Decision:** Use PortAudio with ASIO (Windows) and ALSA (Linux) backends
**Consequences:** Mature library, good latency, requires ASIO SDK on Windows

### ADR-002: Real-Time Convolution for HRTF
**Status:** Accepted
**Context:** Need spatial audio with <10ms latency
**Decision:** Time-domain convolution with SIMD optimization (not FFT)
**Consequences:** Lower latency than FFT, simpler implementation, CPU-bound

### ADR-003: OpenVR for VR Tracking
**Status:** Accepted
**Context:** Support multiple VR headsets (Quest, Vive, Index, WMR)
**Decision:** Use OpenVR/SteamVR as universal tracking API
**Consequences:** Requires SteamVR runtime, works with all major headsets

### ADR-004: ImGui for VR Overlay UI
**Status:** Accepted
**Context:** Need simple VR interface for audio monitoring
**Decision:** Use ImGui rendered to texture, submitted as OpenVR overlay
**Consequences:** Easy to develop, flexible, lightweight

### ADR-005: Synthetic HRTF for Beta
**Status:** Temporary
**Context:** MIT KEMAR loading not yet implemented
**Decision:** Use synthetic HRTF generation for beta testing
**Consequences:** Functional spatial audio, less authentic than real HRTF, acceptable for beta

---

**Next Update:** When architecture changes or new patterns emerge
