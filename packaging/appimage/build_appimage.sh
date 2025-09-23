#!/bin/bash
set -e

# VR Binaural Recorder AppImage Build Script
# This script creates a self-contained AppImage for Linux distribution

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
APPDIR="$PROJECT_ROOT/VRBinauralRecorder.AppDir"

echo "=== VR Binaural Recorder AppImage Builder ==="
echo "Project Root: $PROJECT_ROOT"
echo "Build Directory: $BUILD_DIR"
echo "AppDir: $APPDIR"

# Check if project is built
if [ ! -f "$BUILD_DIR/bin/vr_binaural_recorder" ]; then
    echo "Error: Application not built. Run 'cmake --build build' first."
    exit 1
fi

# Clean previous AppDir
rm -rf "$APPDIR"
mkdir -p "$APPDIR"

# Create directory structure
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/lib"
mkdir -p "$APPDIR/usr/share/vr-binaural-recorder"
mkdir -p "$APPDIR/usr/share/applications"
mkdir -p "$APPDIR/usr/share/icons/hicolor/128x128/apps"
mkdir -p "$APPDIR/usr/share/icons/hicolor/scalable/apps"

echo "--- Copying executable ---"
cp "$BUILD_DIR/bin/vr_binaural_recorder" "$APPDIR/usr/bin/"
strip "$APPDIR/usr/bin/vr_binaural_recorder"

echo "--- Copying libraries ---"
# Copy our built libraries
if [ -d "$BUILD_DIR/lib" ]; then
    cp -r "$BUILD_DIR/lib/"* "$APPDIR/usr/lib/"
fi

# Copy configuration and data files
echo "--- Copying configuration and data ---"
if [ -f "$PROJECT_ROOT/config/vr_binaural_config.json" ]; then
    cp "$PROJECT_ROOT/config/vr_binaural_config.json" "$APPDIR/usr/share/vr-binaural-recorder/"
fi

# Copy HRTF data if it exists
if [ -d "$PROJECT_ROOT/deps/hrtf" ]; then
    cp -r "$PROJECT_ROOT/deps/hrtf" "$APPDIR/usr/share/vr-binaural-recorder/"
fi

# Copy desktop file and icons
echo "--- Installing desktop integration ---"
cp "$SCRIPT_DIR/VRBinauralRecorder.desktop" "$APPDIR/usr/share/applications/"
cp "$SCRIPT_DIR/VRBinauralRecorder.desktop" "$APPDIR/"

# Copy icons
if [ -f "$SCRIPT_DIR/vr_binaural_recorder.svg" ]; then
    cp "$SCRIPT_DIR/vr_binaural_recorder.svg" "$APPDIR/usr/share/icons/hicolor/scalable/apps/"
    cp "$SCRIPT_DIR/vr_binaural_recorder.svg" "$APPDIR/"
fi

if [ -f "$SCRIPT_DIR/vr_binaural_recorder.png" ]; then
    cp "$SCRIPT_DIR/vr_binaural_recorder.png" "$APPDIR/usr/share/icons/hicolor/128x128/apps/"
    cp "$SCRIPT_DIR/vr_binaural_recorder.png" "$APPDIR/"
else
    # Create a simple PNG icon as fallback
    echo "--- Creating fallback PNG icon ---"
    if command -v convert >/dev/null 2>&1 && [ -f "$SCRIPT_DIR/vr_binaural_recorder.svg" ]; then
        convert "$SCRIPT_DIR/vr_binaural_recorder.svg" -resize 128x128 "$APPDIR/vr_binaural_recorder.png"
        cp "$APPDIR/vr_binaural_recorder.png" "$APPDIR/usr/share/icons/hicolor/128x128/apps/"
    else
        # Create a minimal PNG with ImageMagick if available
        if command -v convert >/dev/null 2>&1; then
            convert -size 128x128 xc:blue \
                    -fill white -draw "circle 64,64 64,20" \
                    -fill blue -draw "circle 44,54 44,42" \
                    -fill blue -draw "circle 84,54 84,42" \
                    "$APPDIR/vr_binaural_recorder.png"
            cp "$APPDIR/vr_binaural_recorder.png" "$APPDIR/usr/share/icons/hicolor/128x128/apps/"
        fi
    fi
fi

# Copy AppRun
echo "--- Installing AppRun ---"
cp "$SCRIPT_DIR/AppRun" "$APPDIR/"
chmod +x "$APPDIR/AppRun"

# Bundle system libraries
echo "--- Bundling dependencies ---"

# Function to copy library dependencies
copy_deps() {
    local binary="$1"
    local target_dir="$2"

    echo "  Analyzing dependencies for: $(basename "$binary")"

    # Use simpler approach - copy non-system libraries
    ldd "$binary" 2>/dev/null | grep "=>" | while read -r line; do
        lib_path=$(echo "$line" | awk '{print $3}')

        if [ -z "$lib_path" ] || [ "$lib_path" = "(0x" ]; then
            continue
        fi

        lib_name=$(basename "$lib_path")

        # Skip system libraries
        case "$lib_name" in
            ld-linux-x86-64.so*|libc.so*|libm.so*|libdl.so*|librt.so*|libpthread.so*|libgcc_s.so*|libstdc++.so*)
                continue
                ;;
        esac

        # Skip if already bundled
        if [ -f "$target_dir/$lib_name" ]; then
            continue
        fi

        # Bundle audio libraries and our custom libraries
        case "$lib_path" in
            */build/lib/*|*jack*|*pulse*|*spdlog*|*jsoncpp*|*glfw*)
                if [ -f "$lib_path" ]; then
                    echo "    Bundling: $lib_name"
                    cp "$lib_path" "$target_dir/"
                fi
                ;;
        esac
    done
}

# Bundle dependencies for main executable
copy_deps "$APPDIR/usr/bin/vr_binaural_recorder" "$APPDIR/usr/lib"

# Download AppImageTool if not present
APPIMAGETOOL="$PROJECT_ROOT/appimagetool-x86_64.AppImage"
if [ ! -f "$APPIMAGETOOL" ]; then
    echo "--- Downloading AppImageTool ---"
    wget -O "$APPIMAGETOOL" "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"
    chmod +x "$APPIMAGETOOL"
fi

# Create AppImage
echo "--- Creating AppImage ---"
export ARCH=x86_64
"$APPIMAGETOOL" "$APPDIR" "$PROJECT_ROOT/VRBinauralRecorder-x86_64.AppImage"

# Make executable
chmod +x "$PROJECT_ROOT/VRBinauralRecorder-x86_64.AppImage"

echo "=== AppImage Build Complete ==="
echo "AppImage: $PROJECT_ROOT/VRBinauralRecorder-x86_64.AppImage"
echo "Size: $(du -h "$PROJECT_ROOT/VRBinauralRecorder-x86_64.AppImage" | cut -f1)"
echo ""
echo "Test the AppImage:"
echo "  $PROJECT_ROOT/VRBinauralRecorder-x86_64.AppImage --help"
echo ""
echo "Install for testing:"
echo "  chmod +x VRBinauralRecorder-x86_64.AppImage"
echo "  ./VRBinauralRecorder-x86_64.AppImage"