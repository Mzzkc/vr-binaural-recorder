# Technology Context

**Last Updated:** 2025-10-24 13:03 UTC

---

## 🛠️ TECHNOLOGY STACK

### Core Language & Standards
- **C++17** - Modern C++ features, `std::optional`, structured bindings
- **CMake 3.16+** - Cross-platform build system
- **Standard:** ISO C++17 (no GNU extensions)

### Primary Dependencies

| Library | Version | Purpose | License |
|---------|---------|---------|---------|
| **PortAudio** | v19.7.0 | Cross-platform audio I/O | MIT-like |
| **OpenVR** | v1.23.7 | VR headset tracking | BSD 3-Clause |
| **ImGui** | v1.89+ | VR overlay interface | MIT |
| **GLFW** | v3.3+ | OpenGL context creation | zlib |
| **spdlog** | v1.11+ | Async logging | MIT |
| **jsoncpp** | v1.9+ | JSON configuration | MIT/Public Domain |
| **Google Test** | v1.13+ | Unit testing framework | BSD 3-Clause |

### Platform-Specific Dependencies

**Windows:**
- **ASIO SDK** - Low-latency audio driver support
- **NSIS** - Installer generation
- **MSVC 2019+** - Microsoft Visual C++ compiler (C++17 support)

**Linux:**
- **ALSA** - Advanced Linux Sound Architecture
- **X11** - Window system for OpenGL
- **OpenGL development headers** - Graphics rendering

**macOS (Future):**
- **Core Audio** - macOS audio framework
- **Cocoa** - Native window management

---

## 📦 DEPENDENCY MANAGEMENT

### CMake FetchContent Strategy
All third-party dependencies are managed via CMake's `FetchContent` module:

```cmake
# Example: PortAudio
FetchContent_Declare(
  portaudio
  GIT_REPOSITORY https://github.com/PortAudio/portaudio.git
  GIT_TAG v19.7.0
)
FetchContent_MakeAvailable(portaudio)
```

**Benefits:**
- ✅ Reproducible builds (locked versions)
- ✅ No manual dependency installation
- ✅ Version control for all dependencies
- ✅ Cross-platform compatibility

**Exception: OpenVR SDK**
- Too large for FetchContent (147MB)
- Auto-download mechanism in `cmake/OpenVRDownload.cmake`
- Cached in `third_party/openvr/`

---

## 🏗️ BUILD SYSTEM

### CMake Configuration

**Project Structure:**
```
CMakeLists.txt              # Root configuration
├── core/CMakeLists.txt     # Core application
├── modules/                # Module libraries
│   ├── audio/
│   ├── vr/
│   └── ui/
├── tests/CMakeLists.txt    # Test suite
└── packaging/              # Platform-specific packaging
    └── windows/CMakeLists.txt
```

**Build Types:**
- **Debug** - No optimization, full debug symbols
- **Release** - `-O3` optimization, stripped symbols
- **RelWithDebInfo** - Optimized with debug info
- **MinSizeRel** - Size-optimized

**Key CMake Variables:**
```cmake
CMAKE_CXX_STANDARD = 17
CMAKE_BUILD_TYPE = Release
BUILD_TESTING = ON
ENABLE_ASIO = ON (Windows only)
```

### Build Commands

**Linux:**
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release -j$(nproc)
ctest --verbose
```

**Windows:**
```powershell
mkdir build; cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
ctest -C Release --verbose
```

**macOS:**
```bash
mkdir build && cd build
cmake .. -G Xcode
cmake --build . --config Release
ctest --verbose
```

---

## 🔧 COMPILER CONFIGURATIONS

### GCC/Clang (Linux/macOS)
```cmake
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fsanitize=address")
```

**Key Flags:**
- `-Wall -Wextra -Wpedantic` - Enable all warnings
- `-march=native` - Optimize for local CPU (AVX2, etc.)
- `-fsanitize=address` - Memory error detection (debug builds)

### MSVC (Windows)
```cmake
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /permissive-")
set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG /arch:AVX2")
set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /RTC1")
```

**Key Flags:**
- `/W4` - Warning level 4
- `/permissive-` - Strict standard conformance
- `/arch:AVX2` - Enable AVX2 SIMD instructions
- `/RTC1` - Runtime error checks (debug)

**Windows-Specific Fixes:**
- `_USE_MATH_DEFINES` - Enable M_PI constant
- `NOMINMAX` - Prevent min/max macro conflicts
- `WIN32_LEAN_AND_MEAN` - Reduce Windows.h bloat

---

## 🎮 PLATFORM CONFIGURATIONS

### Windows Build
**Audio Backend:** ASIO (primary), WASAPI (fallback)
**VR Runtime:** SteamVR or Oculus Runtime
**Installer:** NSIS (Nullsoft Scriptable Install System)

**Required Libraries:**
```
setupapi.lib    # Device enumeration
winmm.lib       # Multimedia timers
ole32.lib       # COM interfaces
```

**Installer Configuration:**
- Location: `packaging/windows/installer.nsi`
- Output: `VRBinauralRecorder-Windows-x64-Setup.exe`
- Install path: `C:\Program Files\VR Binaural Recorder\`
- Registry keys: HKLM for uninstall info

### Linux Build
**Audio Backend:** ALSA (Advanced Linux Sound Architecture)
**VR Runtime:** SteamVR (via Proton/native)
**Package Format:** AppImage (future)

**Required System Libraries:**
```bash
sudo apt-get install \
    libasound2-dev \      # ALSA
    libx11-dev \          # X11
    libgl1-mesa-dev \     # OpenGL
    libglu1-mesa-dev      # OpenGL utilities
```

### macOS Build (Future)
**Audio Backend:** Core Audio
**VR Runtime:** SteamVR
**Package Format:** .dmg

---

## 🔬 DEVELOPMENT TOOLS

### Required Tools
- **Git** - Version control (2.30+)
- **CMake** - Build system generator (3.16+)
- **C++17 Compiler** - GCC 7+, Clang 5+, MSVC 2019+
- **Python 3** - Build scripts and validation tools

### Optional Tools
- **ccache** - Compiler cache (speeds up rebuilds)
- **ninja** - Fast build executor (`cmake -G Ninja`)
- **clang-tidy** - Static analysis
- **clang-format** - Code formatting

### IDE Support
- **Visual Studio 2019/2022** - Full CMake integration
- **CLion** - JetBrains IDE with CMake support
- **VS Code** - With C++ and CMake extensions
- **Vim/Neovim** - Via clangd LSP

---

## 📊 PERFORMANCE PROFILING

### Linux Profiling Tools
```bash
# CPU profiling with perf
perf record -g ./vr_binaural_recorder
perf report

# Memory profiling with valgrind
valgrind --tool=massif ./vr_binaural_recorder

# Call graph with gprof
gcc -pg ...
./vr_binaural_recorder
gprof vr_binaural_recorder gmon.out > analysis.txt
```

### Windows Profiling Tools
- **Visual Studio Profiler** - CPU, memory, threading
- **Intel VTune** - Advanced CPU profiling
- **Windows Performance Analyzer** - System-level tracing

### Real-Time Audio Metrics
- **Buffer underruns** - Monitored via PortAudio callbacks
- **Latency measurements** - Loopback testing
- **CPU usage** - Per-thread monitoring

---

## 🧪 TESTING INFRASTRUCTURE

### Test Framework
**Google Test (gtest)** - C++ unit testing framework

**Test Organization:**
```
tests/
├── spatial_audio_validation_BLOCKING.cpp  # Deployment blockers
├── ceo_spatial_validation.cpp             # Workflow validation
├── compilation_fixes_validation.cpp       # Integration tests
├── windows_rc_validation.cpp              # Windows platform tests
└── dependency_verification/               # Dependency tests
    ├── test_jsoncpp.cpp
    ├── test_portaudio.cpp
    └── test_hrtf.cpp
```

**Test Execution:**
```bash
cd build
ctest --verbose               # Run all tests
ctest -R BLOCKING             # Run only BLOCKING tests
ctest --output-on-failure     # Show failures only
```

### Continuous Integration (Future)
- **GitHub Actions** - Automated builds on push
- **Windows/Linux/macOS** - Multi-platform testing
- **Code coverage** - gcov/lcov reporting
- **Static analysis** - clang-tidy integration

---

## 🔐 SECURITY & COMPLIANCE

### Code Security
- **No eval/exec** - All code statically analyzable
- **Input validation** - Config file JSON schema validation
- **Memory safety** - Modern C++ (RAII, smart pointers)
- **Sanitizers** - AddressSanitizer in debug builds

### License Compliance
- **MIT License** - Permissive, commercial-friendly
- **Third-party licenses** - All compatible (MIT, BSD, zlib)
- **No GPL dependencies** - Avoid copyleft requirements
- **Attribution** - MIT KEMAR dataset cited

---

## 📚 DEPENDENCY DETAILS

### PortAudio
**Version:** v19.7.0
**Purpose:** Cross-platform audio I/O abstraction
**Backends Used:**
- Windows: ASIO (primary), WASAPI (fallback), DirectSound
- Linux: ALSA
- macOS: Core Audio

**Configuration:**
```cmake
set(PA_BUILD_SHARED OFF)              # Static linking
set(PA_USE_ASIO ON)                   # Windows ASIO support
set(PA_USE_WASAPI ON)                 # Windows WASAPI support
set(PA_DISABLE_INSTALL ON)            # No system install
```

### OpenVR
**Version:** v1.23.7
**Purpose:** Universal VR headset tracking API
**Supported Headsets:**
- Meta Quest 2/3/Pro (via Link/Air Link)
- HTC Vive / Vive Pro
- Valve Index
- Windows Mixed Reality headsets

**Download Mechanism:**
- Auto-download if not present
- Cached in `third_party/openvr/`
- Provides `openvr_api.dll` (Windows) or `libopenvr_api.so` (Linux)

### ImGui
**Version:** v1.89+
**Purpose:** Immediate-mode GUI for VR overlay
**Backends Used:**
- Rendering: OpenGL 3.x (`imgui_impl_opengl3`)
- Platform: GLFW (`imgui_impl_glfw`)

**Integration:**
```cpp
// Render ImGui to texture
ImGui::Render();
ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

// Submit texture to OpenVR overlay
vr::VROverlay()->SetOverlayTexture(overlayHandle, &texture);
```

### spdlog
**Version:** v1.11+
**Purpose:** Fast asynchronous logging
**Configuration:**
- **Async mode** - Non-blocking logs from real-time threads
- **Rotating files** - `logs/vr_binaural_YYYY-MM-DD.log`
- **Console + file** - Dual output sinks
- **Log levels** - trace, debug, info, warn, error, critical

### jsoncpp
**Version:** v1.9+
**Purpose:** JSON parsing for configuration
**Usage:**
```cpp
Json::Value root;
Json::Reader reader;
reader.parse(configFile, root);

int sampleRate = root["audio"]["sampleRate"].asInt();
```

**Windows Compatibility:**
- Using `jsoncpp_static` target (not `jsoncpp_lib`)
- Fixes symbol conflicts with Windows SDK

---

## 🌐 NETWORK & EXTERNAL SERVICES

### No External Services Required
- ✅ Fully offline operation
- ✅ No telemetry/analytics
- ✅ No cloud dependencies
- ✅ No license activation

### Optional Integrations
- **OBS Studio** - Virtual audio cable for streaming (local)
- **SteamVR** - VR runtime (local service)

---

## 📝 BUILD ARTIFACTS

### Outputs (Release Build)
```
build/
├── bin/
│   ├── vr_binaural_recorder           # Main executable (12.1MB Linux)
│   ├── vr_binaural_tests              # Test suite
│   └── dependency verification tests
├── lib/
│   └── (static libraries)
└── packaging/
    └── VRBinauralRecorder-Windows-x64-Setup.exe  # Windows installer
```

### Runtime Dependencies
**Windows:**
- `openvr_api.dll` (bundled in installer)
- `vr_binaural_config.json` (user-editable)
- HRTF dataset WAV files (368 files, ~50MB)

**Linux:**
- `libopenvr_api.so` (bundled)
- `vr_binaural_config.json`
- HRTF dataset

---

## 🔄 DEPLOYMENT WORKFLOW

### Development Build
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
./bin/vr_binaural_recorder
```

### Release Build
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
ctest  # Validate before deployment
```

### Windows Installer
```powershell
cd build
cmake --build . --target windows_installer
# Output: packaging/VRBinauralRecorder-Windows-x64-Setup.exe
```

### Packaging Checklist
- [ ] All tests pass (BLOCKING + recommended)
- [ ] Version number updated in CMakeLists.txt
- [ ] CHANGELOG.md updated
- [ ] User documentation reviewed
- [ ] Installer tested on clean Windows install
- [ ] HRTF dataset included
- [ ] License file included

---

**Next Update:** When dependencies change or new tools are added
