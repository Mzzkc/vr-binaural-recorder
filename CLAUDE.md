# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

VR Binaural Recorder is a PROTOTYPE C++ application targeting real-time spatial audio recording and processing in VR environments. The current implementation provides basic infrastructure for VR head tracking and binaural audio processing, but is NOT yet a complete, production-ready solution.

## Build System and Commands

### Build Configuration
- **Build System**: CMake (minimum version 3.16)
- **C++ Standard**: C++17
- **Build Types**: Release (default), Debug

### Essential Commands

```bash
# Configure build system
mkdir -p build && cd build
cmake ..

# Build the project
cmake --build . --config Release

# Run tests
ctest --verbose

# Build installer package
cmake --build . --target package

# Quick status check
./check_status.sh

# System validation
python3 tools/validator.py
```

### Platform-Specific Build Notes
- **Windows**: Requires ASIO SDK for low-latency audio, NSIS for installer
- **Linux**: Requires X11, OpenGL development packages
- **macOS**: Uses system OpenGL frameworks

## Architecture Overview

### Core Components

1. **Audio Engine** (`src/audio_engine.cpp`, `include/audio_engine.h`)
   - Manages PortAudio streams with ASIO/WASAPI support
   - Real-time audio processing pipeline
   - Virtual audio device management

2. **HRTF Processor** (`src/hrtf_processor.cpp`, `include/hrtf_processor.h`)
   - Head-Related Transfer Function processing
   - Binaural audio synthesis
   - Distance modeling and near-field compensation

3. **VR Tracker** (`src/vr_tracker.cpp`, `include/vr_tracker.h`)
   - OpenVR integration for head tracking
   - Pose prediction and smoothing
   - Tracking thread management

4. **Overlay UI** (`src/overlay_ui.cpp`, `include/overlay_ui.h`)
   - ImGui-based VR overlay interface
   - Real-time parameter adjustment
   - OpenGL rendering backend

5. **Configuration System** (`src/config.cpp`, `include/config.h`)
   - JSON-based configuration management
   - Runtime parameter validation
   - Hot-reloading support

### Key Dependencies (PARTIALLY INTEGRATED)

- **PortAudio**: Audio I/O infrastructure (requires further integration)
- **OpenVR**: VR runtime integration (INCOMPLETE)
- **ImGui + GLFW**: UI framework added (basic setup)
- **spdlog**: Logging system (minimal implementation)
- **jsoncpp**: Configuration parsing (basic support)

## Configuration

Main configuration file: `config/vr_binaural_config.json`

Key configuration sections:
- `audio`: Sample rate, buffer size, device selection, ASIO settings
- `hrtf`: Dataset path, filter settings, convolution method
- `vr`: Tracking rate, prediction, overlay positioning
- `performance`: Threading, SIMD, memory management
- `logging`: Log levels, file rotation, async logging

## Development Workflow

### Testing
- Main test suite: `tests/test_suite.cpp` (uses Google Test)
- Audio engine, HRTF processor, ring buffer, and config tests included
- Run with: `./build/vr_binaural_tests`

### System Validation
Use `tools/validator.py` to check:
- OS compatibility and architecture
- Audio device capabilities
- VR runtime availability
- Performance characteristics
- Dependency versions

### Debugging
- Enable debug logging: Set `logging.level` to `"debug"` in config
- Performance profiling: Set `debug.profiling` to `true`
- Verbose audio/VR logging available via config flags

## Performance Considerations

- PROTOTYPE stage: Performance optimizations are planned but not fully implemented
- Thread management infrastructure in development
- SIMD and memory optimization strategies identified
- Current implementation focuses on architectural design
- Performance benchmarking and optimization is a future milestone

## File Structure Notes

- `hrtf_data/`: HRTF datasets (not version controlled, large binary files)
- `third_party/`: External dependencies (OpenVR, GLEW, ASIO SDK)
- `logs/`: Runtime log files (auto-rotation)
- `build/`: CMake build artifacts
- Header files in `include/` follow component naming