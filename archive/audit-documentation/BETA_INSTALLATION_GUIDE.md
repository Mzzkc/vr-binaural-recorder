# VR Binaural Recorder - Beta Installation Guide
Professional VR Spatial Audio Recording

![Version](https://img.shields.io/badge/version-1.0.0--beta-orange)
![Platform](https://img.shields.io/badge/platform-Linux%20x86__64-blue)
![License](https://img.shields.io/badge/license-Open%20Source-green)

## Quick Start (60 Seconds)

```bash
# 1. Download (replace with actual release URL)
wget https://github.com/your-org/vr-binaural-recorder/releases/download/v1.0.0-beta.1/VRBinauralRecorder-x86_64.AppImage

# 2. Make executable
chmod +x VRBinauralRecorder-x86_64.AppImage

# 3. Run
./VRBinauralRecorder-x86_64.AppImage
```

**That's it!** No installation, no dependencies, no root access required.

## System Requirements

### Minimum Requirements
| Component | Requirement |
|-----------|-------------|
| OS | Linux kernel 3.2+ (Ubuntu 20.04+, Fedora 35+, Arch Linux) |
| Architecture | x86_64 (64-bit Intel/AMD) |
| RAM | 4GB (8GB recommended) |
| Graphics | OpenGL 3.3+ support |
| Audio | ALSA, PulseAudio, or JACK |
| Filesystem | FUSE2 support (usually pre-installed) |

### VR Requirements (Optional)
| Component | Requirement |
|-----------|-------------|
| VR Headset | OpenVR compatible (Valve Index, HTC Vive, Oculus Rift) |
| GPU | Dedicated graphics card (GTX 1060+ / RX 580+) |
| USB | Available USB 3.0 ports for VR hardware |

### Professional Audio (Optional)
| Component | Requirement |
|-----------|-------------|
| Audio Interface | ASIO-compatible (via JACK bridge) |
| Sample Rate | 48kHz+ capability |
| Latency | <10ms round-trip (JACK recommended) |

## Installation Methods

### Method 1: Direct Download (Recommended)

```bash
# Create a directory for the application
mkdir -p ~/Applications
cd ~/Applications

# Download the latest beta
curl -L -o VRBinauralRecorder-x86_64.AppImage \
  "https://github.com/your-org/vr-binaural-recorder/releases/latest/download/VRBinauralRecorder-x86_64.AppImage"

# Make executable
chmod +x VRBinauralRecorder-x86_64.AppImage

# Optional: Verify download integrity
curl -L -o VRBinauralRecorder-x86_64.AppImage.sha256 \
  "https://github.com/your-org/vr-binaural-recorder/releases/latest/download/VRBinauralRecorder-x86_64.AppImage.sha256"
sha256sum -c VRBinauralRecorder-x86_64.AppImage.sha256
```

### Method 2: Desktop Integration

```bash
# Install to desktop applications
./VRBinauralRecorder-x86_64.AppImage --appimage-integrate

# Or manually copy desktop file
mkdir -p ~/.local/share/applications
./VRBinauralRecorder-x86_64.AppImage --appimage-extract-and-run --desktop-file > ~/.local/share/applications/VRBinauralRecorder.desktop
```

### Method 3: System-Wide Installation

```bash
# For system administrators (optional)
sudo mv VRBinauralRecorder-x86_64.AppImage /opt/
sudo chmod +x /opt/VRBinauralRecorder-x86_64.AppImage
sudo ln -s /opt/VRBinauralRecorder-x86_64.AppImage /usr/local/bin/vr-binaural-recorder
```

## First Launch Setup

### 1. Initial Configuration
On first launch, VR Binaural Recorder will:
- Create `~/.config/vr-binaural-recorder/` directory
- Copy default configuration file
- Set up logging directory
- Detect available audio devices

### 2. Audio Configuration
```bash
# Check available audio devices
./VRBinauralRecorder-x86_64.AppImage --list-audio-devices

# Configure JACK (for professional setups)
sudo apt install jackd2  # Ubuntu/Debian
qjackctl  # Launch JACK control panel
```

### 3. VR Setup (Optional)
```bash
# Install SteamVR (if using Steam)
# 1. Install Steam
# 2. Install SteamVR from Steam store
# 3. Set up your headset

# Or install OpenVR standalone
# Follow your headset manufacturer's Linux setup guide
```

## Usage Examples

### Basic Recording
```bash
# Start with default settings
./VRBinauralRecorder-x86_64.AppImage

# Start with custom config
./VRBinauralRecorder-x86_64.AppImage --config /path/to/custom-config.json

# Start in headless mode (no UI)
./VRBinauralRecorder-x86_64.AppImage --headless
```

### Advanced Options
```bash
# Show all command line options
./VRBinauralRecorder-x86_64.AppImage --help

# Enable debug logging
./VRBinauralRecorder-x86_64.AppImage --debug

# Test audio setup
./VRBinauralRecorder-x86_64.AppImage --test-audio
```

## Configuration

### Configuration File Location
```
~/.config/vr-binaural-recorder/
├── vr_binaural_config.json  # Main configuration
├── logs/                    # Application logs
└── profiles/               # User-defined profiles
```

### Basic Configuration
```json
{
  "audio": {
    "sampleRate": 48000,
    "bufferSize": 128,
    "api": "auto",
    "preferredInputDevice": ""
  },
  "vr": {
    "trackingRate": 90,
    "smoothingFactor": 0.95
  }
}
```

### Audio API Selection
```json
{
  "audio": {
    "api": "jack",          // Professional: JACK
    "api": "pulseaudio",    // Desktop: PulseAudio
    "api": "alsa",          // Low-level: ALSA
    "api": "auto"           // Automatic detection
  }
}
```

## Troubleshooting

### Common Issues

#### AppImage Won't Run
```bash
# Check FUSE support
ls /dev/fuse
# If missing, install FUSE
sudo apt install fuse  # Ubuntu/Debian
sudo dnf install fuse  # Fedora
sudo pacman -S fuse2   # Arch Linux

# Alternative: Extract and run
./VRBinauralRecorder-x86_64.AppImage --appimage-extract
cd squashfs-root
./AppRun
```

#### Audio Issues
```bash
# Check audio system
pulseaudio --check -v  # PulseAudio
jack_control status    # JACK

# Check permissions
sudo usermod -a -G audio $USER  # Add user to audio group
# Log out and back in

# Test audio devices
aplay -l  # List playback devices
arecord -l  # List recording devices
```

#### VR Issues
```bash
# Check VR runtime
steamvr-status  # If using SteamVR

# Check USB permissions
ls -l /dev/hidraw*
sudo usermod -a -G plugdev $USER  # Add user to plugdev group

# Check graphics drivers
glxinfo | grep OpenGL  # Check OpenGL support
```

#### Performance Issues
```bash
# Check system resources
top -p $(pgrep vr_binaural_recorder)

# Enable performance mode
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# Check real-time permissions
ulimit -r  # Should show real-time priority limit
```

### Log Analysis
```bash
# View application logs
tail -f ~/.config/vr-binaural-recorder/logs/vr_binaural.log

# Filter for errors
grep ERROR ~/.config/vr-binaural-recorder/logs/vr_binaural.log

# Debug level logging
./VRBinauralRecorder-x86_64.AppImage --log-level debug
```

## Uninstallation

### Complete Removal
```bash
# Remove AppImage
rm ~/Applications/VRBinauralRecorder-x86_64.AppImage

# Remove configuration (optional)
rm -rf ~/.config/vr-binaural-recorder/

# Remove desktop integration (if installed)
rm ~/.local/share/applications/VRBinauralRecorder.desktop
```

### Preserve Settings
```bash
# Keep configuration for future installations
mv ~/.config/vr-binaural-recorder/ ~/.config/vr-binaural-recorder.backup
```

## Getting Help

### Documentation
- **User Manual**: [docs/user-manual.md](docs/user-manual.md)
- **FAQ**: [docs/faq.md](docs/faq.md)
- **API Reference**: [docs/api-reference.md](docs/api-reference.md)

### Community Support
- **GitHub Issues**: Bug reports and feature requests
- **Discord Community**: Real-time chat and support
- **Reddit**: r/VRBinauralRecorder for community discussions

### Beta Feedback
We need your feedback! Please report:
- ✅ **What works well**
- 🐛 **Bugs and crashes**
- 💡 **Feature suggestions**
- 📱 **Compatibility issues**
- 🎵 **Audio quality feedback**

#### Feedback Channels
```bash
# Create a bug report
./VRBinauralRecorder-x86_64.AppImage --create-bug-report

# Generate system info
./VRBinauralRecorder-x86_64.AppImage --system-info > system-info.txt
```

### Professional Support
For professional deployments:
- Email: support@vr-binaural-recorder.com
- Priority support available for organizations
- Custom configuration assistance
- Integration consulting

## Beta Limitations

### Known Issues
- ⚠️ JACK auto-connection needs manual setup
- ⚠️ Some USB audio interfaces require additional drivers
- ⚠️ VR overlay position may need adjustment per headset
- ⚠️ High CPU usage on older systems (<2015)

### Upcoming Features
- 🔄 Real-time audio effects processing
- 🔄 Multi-channel recording (>2 channels)
- 🔄 Automated HRTF calibration
- 🔄 Cloud sync for settings
- 🔄 Plugin architecture for custom effects

### Beta Stability
This is beta software:
- **Save your work frequently**
- **Test thoroughly before important recordings**
- **Keep backups of configuration files**
- **Report any crashes immediately**

## License & Legal

This software is distributed under an open-source license. See LICENSE file for details.

**Beta Disclaimer**: This software is provided "as-is" for testing purposes. Use at your own risk in production environments.

---

**Happy Recording!** 🎧🥽

For the latest updates and releases, visit: [GitHub Releases](https://github.com/your-org/vr-binaural-recorder/releases)