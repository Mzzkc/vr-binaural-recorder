#!/bin/bash
# check_status.sh - Quick status check for VR Binaural Recorder

echo "=== VR Binaural Recorder Status Check ==="
echo

# Check directory structure
echo "📁 Directory Structure:"
[ -d "build" ] && echo "  ✓ build/" || echo "  ❌ build/ (missing)"
[ -d "config" ] && echo "  ✓ config/" || echo "  ❌ config/ (missing)"
[ -d "include" ] && echo "  ✓ include/" || echo "  ❌ include/ (missing)"
[ -d "src" ] && echo "  ✓ src/" || echo "  ❌ src/ (missing)"
[ -d "tests" ] && echo "  ✓ tests/" || echo "  ❌ tests/ (missing)"
[ -d "hrtf_data" ] && echo "  ✓ hrtf_data/" || echo "  ⚠️  hrtf_data/ (missing - will use synthetic)"
[ -d "logs" ] && echo "  ✓ logs/" || echo "  ⚠️  logs/ (will be created)"
echo

# Check essential files
echo "📄 Core Files:"
[ -f "CMakeLists.txt" ] && echo "  ✓ CMakeLists.txt" || echo "  ❌ CMakeLists.txt"
[ -f "config/vr_binaural_config.json" ] && echo "  ✓ Configuration file" || echo "  ❌ Configuration file"
[ -f "src/main.cpp" ] && echo "  ✓ Main entry point" || echo "  ❌ Main entry point"
echo

# Check headers
echo "📋 Header Files:"
headers=("audio_engine.h" "config.h" "hrtf_processor.h" "logger.h" "overlay_ui.h" "ring_buffer.h" "vr_tracker.h" "vr_types.h")
for header in "${headers[@]}"; do
    if [ -f "include/$header" ] && [ -s "include/$header" ]; then
        echo "  ✓ $header"
    else
        echo "  ❌ $header (missing or empty)"
    fi
done
echo

# Check source files
echo "🔧 Source Files:"
sources=("audio_engine.cpp" "config.cpp" "hrtf_processor.cpp" "main.cpp" "overlay_ui.cpp" "vr_tracker.cpp")
for source in "${sources[@]}"; do
    if [ -f "src/$source" ] && [ -s "src/$source" ]; then
        echo "  ✓ $source"
    else
        echo "  ❌ $source (missing or empty)"
    fi
done
echo

# Check for common issues
echo "⚠️  Known Issues to Address:"
echo "  • Some FFT implementations may need optimization"
echo "  • Virtual audio device setup may require platform-specific work"
echo "  • Network features are planned but not implemented"
echo "  • Hand tracking support is experimental"
echo

# Check dependencies (if available)
echo "🔗 Dependencies:"
if command -v cmake &> /dev/null; then
    echo "  ✓ CMake ($(cmake --version | head -1))"
else
    echo "  ❌ CMake (not found)"
fi

if command -v pkg-config &> /dev/null; then
    echo "  ✓ pkg-config"
else
    echo "  ⚠️  pkg-config (recommended for library detection)"
fi

echo

# System info
echo "💻 System Info:"
echo "  OS: $(uname -s)"
echo "  Architecture: $(uname -m)"
if [ -f /etc/os-release ]; then
    . /etc/os-release
    echo "  Distribution: $NAME $VERSION"
fi
echo

echo "=== Status Check Complete ==="
echo
echo "Next steps:"
echo "1. Install dependencies (PortAudio, OpenVR, ImGui, etc.)"
echo "2. Run: mkdir -p build && cd build && cmake .."
echo "3. Run: cmake --build . --config Release"
echo "4. Check VR headset connection"
echo "5. Configure audio devices in config/vr_binaural_config.json"
