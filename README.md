# VR Binaural Recorder

**Transform any microphone into spatialized binaural audio using VR head tracking**

[![Platform](https://img.shields.io/badge/platform-Windows%2010%2F11-blue)](https://www.microsoft.com/windows)
[![VR Support](https://img.shields.io/badge/VR-Quest%20%7C%20Vive%20%7C%20Index%20%7C%20WMR-green)](https://store.steampowered.com/app/250820/SteamVR/)
[![Audio](https://img.shields.io/badge/audio-48kHz%20Binaural-orange)](https://en.wikipedia.org/wiki/Binaural_recording)
[![Status](https://img.shields.io/badge/status-Beta-yellow)](docs/BETA_TESTING_GUIDE.md)

VR Binaural Recorder is a spatial audio creation tool for ASMRtists and content creators. Record with any standard microphone while wearing a VR headset—your natural head movements create immersive 3D binaural audio that listeners experience as true spatial positioning.

## Features

- **Real-Time Spatial Audio Processing**: HRTF-based binaural synthesis with <10ms latency
- **Natural VR Head Tracking**: Move your head naturally—no complex controls needed
- **Visual Audio Cockpit**: Intuitive VR interface with microphone positioning and audio level meters
- **Professional Audio Quality**: MIT KEMAR HRTF dataset for authentic spatial positioning
- **OBS Integration**: Stream spatial ASMR live to Twitch/YouTube
- **Universal Microphone Support**: Works with USB, XLR, wireless—any microphone you own

## Quick Start

### 1. Install
Run `VRBinauralRecorder-Windows-x64-Setup.exe` (requires administrator privileges)

### 2. Setup VR
- **Quest 2/3/Pro**: Connect via Link/Air Link, install Oculus Desktop
- **Vive/Index**: Install SteamVR, complete room setup
- **Windows Mixed Reality**: Install WMR Portal from Microsoft Store

### 3. Launch
Start **VR Binaural Recorder** from the desktop shortcut or Start Menu

### 4. Record
- Put on your VR headset
- Audio Cockpit overlay appears in front of you
- Move your head naturally to create spatial audio
- Use VR controllers to position the virtual microphone
- Point and click to start/stop recording

## System Requirements

### Minimum Requirements
- **Operating System**: Windows 10 (version 1903+) or Windows 11 (64-bit)
- **Processor**: Intel Core i5-8400 or AMD Ryzen 5 2600 (6+ cores)
- **Memory**: 8GB RAM
- **Graphics**: NVIDIA GTX 1060 / AMD RX 580 (VR-capable GPU)
- **Storage**: 500MB free disk space
- **VR Headset**: Oculus Quest 2/3/Pro, HTC Vive, Valve Index, or Windows Mixed Reality
- **VR Runtime**: SteamVR or Oculus Runtime installed
- **Audio**: Any microphone (USB, XLR interface, or wireless)

### Recommended Setup
- **Processor**: Intel Core i7-9700K or AMD Ryzen 7 3700X (8+ cores)
- **Memory**: 16GB+ RAM
- **Graphics**: NVIDIA RTX 2060 / AMD RX 5700 or better
- **Audio Interface**: ASIO-compatible (Focusrite Scarlett, PreSonus AudioBox, etc.)

## Documentation

### Getting Started
- **[Beta Testing Guide](docs/BETA_TESTING_GUIDE.md)** - Complete setup, usage, and troubleshooting guide
- **[Windows VR Setup Guide](docs/WINDOWS_VR_SETUP_GUIDE.md)** - Technical setup and diagnostics
- **[VR Testing Guide](docs/VR_TESTING_GUIDE.md)** - Validation procedures for technical users

### Advanced Features
- **[OBS Integration Guide](docs/OBS_INTEGRATION_GUIDE.md)** - Stream spatial ASMR to Twitch/YouTube
- **Configuration**: Edit `vr_binaural_config.json` for advanced audio settings

## How It Works

1. **VR Head Tracking**: Your VR headset tracks head position and orientation at 90Hz
2. **Spatial Processing**: HRTF processing converts microphone audio to binaural stereo
3. **Real-Time Output**: <10ms latency ensures natural, immersive spatial audio
4. **Natural Control**: Head movement = microphone positioning (no manual controls needed)

**Example**: Turn your head LEFT → microphone appears to shift RIGHT → audio pans to RIGHT ear

This creates authentic binaural spatialization that listeners experience as 3D positioned audio through headphones.

## Technical Architecture

- **Audio Engine**: PortAudio with ASIO/WASAPI support
- **HRTF Processing**: MIT KEMAR dataset (368 spatial positions)
- **VR Integration**: OpenVR SDK v1.23.7 (SteamVR compatible)
- **UI System**: ImGui with OpenGL VR overlay rendering
- **Performance**: SIMD optimization, lock-free audio buffers, real-time thread scheduling

## Beta Program

This is a **beta release** for ASMRtist testing and validation. Your feedback helps improve the tool!

### Providing Feedback
- **Email**: asmr-support@vr-binaural-recorder.com
- **Subject Line**: "[BETA] Your Issue Description"
- **Include**: System info, logs from `%APPDATA%\VRBinauralRecorder\logs\`, detailed description

### What to Test
- Installation and setup process
- VR headset compatibility
- Spatial audio quality
- Recording workflow
- OBS streaming integration
- Performance on your system

See the [Beta Testing Guide](docs/BETA_TESTING_GUIDE.md) for detailed testing procedures and bug report templates.

## Known Issues (Beta)

- **Audio Routing Detection**: Some systems may require restarting the application to detect audio devices
- **VR Overlay Visibility**: Audio Cockpit may not appear on first launch (press VR menu button, then return)
- **Windows Audio Conflicts**: Discord/OBS may lock audio devices (close before launching)
- **Performance on 6-Core CPUs**: Occasional frame drops on lower-end systems (close background apps)

See the [Beta Testing Guide](docs/BETA_TESTING_GUIDE.md) for complete troubleshooting procedures.

## Building from Source

### Prerequisites
- CMake 3.16+
- C++17 compatible compiler (MSVC 2019+, GCC 9+, Clang 10+)
- OpenVR SDK
- PortAudio
- spdlog, jsoncpp, ImGui (fetched automatically by CMake)

### Build Commands
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### Windows Installer Build
```bash
cmake --build . --target windows_installer
```

See [CLAUDE.md](CLAUDE.md) for detailed build instructions and platform-specific dependencies.

## File Structure

```
VR Binaural Recorder/
├── VRBinauralRecorder.exe      # Main application
├── openvr_api.dll              # OpenVR runtime library
├── vr_binaural_config.json     # Configuration file
├── hrtf_data/                  # Spatial audio datasets
│   └── mit_kemar_compact/      # 368 HRTF measurements
├── docs/                       # User documentation
│   ├── BETA_TESTING_GUIDE.md
│   ├── WINDOWS_VR_SETUP_GUIDE.md
│   └── OBS_INTEGRATION_GUIDE.md
└── logs/                       # Application logs (auto-created)
```

## Support

### Self-Service Resources
- **Documentation**: `/docs` folder
- **Troubleshooting**: [Beta Testing Guide - Troubleshooting](docs/BETA_TESTING_GUIDE.md#troubleshooting-guide)
- **Logs**: `%APPDATA%\VRBinauralRecorder\logs\` (include with bug reports)
- **Configuration**: `vr_binaural_config.json` (see [Advanced Configuration](docs/BETA_TESTING_GUIDE.md#advanced-configuration))

### Beta Tester Priority Support
- **Email**: asmr-support@vr-binaural-recorder.com
- **Response Time**: Blocking issues <24hr, General issues 48-72hr
- **Include**: System report (see [Windows VR Setup Guide](docs/WINDOWS_VR_SETUP_GUIDE.md)), log files, detailed description

## License

See [LICENSE](LICENSE) for licensing terms.

## Credits

- **HRTF Dataset**: MIT Media Lab KEMAR measurements
- **OpenVR**: Valve Corporation
- **Audio Processing**: PortAudio community
- **Beta Testers**: ASMRtist community

## Version

**Beta 1.0** - 2025-10-22

Production-ready core functionality with Windows VR support. Beta testing phase for ASMRtist validation and workflow refinement.

---

**Ready to create immersive spatial ASMR? Start with the [Beta Testing Guide](docs/BETA_TESTING_GUIDE.md)!**
