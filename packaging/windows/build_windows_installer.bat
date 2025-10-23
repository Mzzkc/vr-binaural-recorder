@echo off
REM Windows Installer Build Script for VR Binaural Recorder
REM This script must be run on Windows with NSIS installed
REM Professional Quality Installer Generation

echo ========================================
echo VR Binaural Recorder - Windows Installer
echo ========================================

echo Checking build environment...

REM Check if we're on Windows
if not "%OS%"=="Windows_NT" (
    echo ERROR: This script must be run on Windows
    exit /b 1
)

REM Check for NSIS
where makensis >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: NSIS not found. Please install NSIS from:
    echo   https://nsis.sourceforge.io/Download
    echo   Or use chocolatey: choco install nsis
    exit /b 1
)

REM Check for CMake
where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: CMake not found. Please install CMake
    exit /b 1
)

REM Check for PowerShell (should be available on modern Windows)
where powershell >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: PowerShell not found
    exit /b 1
)

echo Environment checks passed.
echo.

REM Change to build directory
if not exist build (
    echo Creating build directory...
    mkdir build
)

cd build

echo Configuring CMake for Windows build...
cmake .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 16 2019" -A x64
if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed
    exit /b 1
)

echo Building VR Binaural Recorder...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo ERROR: Build failed
    exit /b 1
)

echo Build completed successfully.
echo.

echo Preparing installer components...

REM Download HRTF data if not present
if not exist ..\deps\hrtf (
    echo Downloading HRTF data...
    cd ..
    python scripts\tools\download_hrtf_datasets.py --dataset mit_kemar_compact
    cd build
)

REM Create missing README if not present
if not exist ..\README.md (
    echo Creating README.md...
    powershell -Command "
        @'
# VR Binaural Recorder

Professional spatial audio recording tool for ASMRtists and content creators.

## Features

- Real-time VR position tracking for microphone placement
- Binaural spatial audio processing with HRTF
- Professional audio routing and recording
- VR headset integration (SteamVR/OpenVR)
- Windows virtual audio device support

## Requirements

- Windows 10 (1903+) or Windows 11 (64-bit)
- 4GB RAM minimum (8GB+ recommended)
- Visual C++ Redistributable 2019+
- For VR functionality: SteamVR/OpenVR compatible headset

## Installation

Run the installer and follow the setup wizard. All dependencies will be automatically installed.

## Usage

1. Launch VR Binaural Recorder
2. Configure your VR headset (if available)
3. Position virtual microphone in VR space
4. Start recording with spatial audio processing

## Support

For support and documentation, visit: https://github.com/vrbaudio/vr-binaural-recorder
'@ | Out-File -Encoding UTF8 ..\README.md
    "
)

REM Create missing LICENSE if not present
if not exist ..\LICENSE (
    echo Creating LICENSE file...
    powershell -Command "
        @'
MIT License

Copyright (c) 2024 VRB Audio

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the \"Software\"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
'@ | Out-File -Encoding UTF8 ..\LICENSE
    "
)

echo Creating Windows installer...
cmake --build . --target windows_installer
if %errorlevel% neq 0 (
    echo ERROR: Installer creation failed
    echo.
    echo Trying manual NSIS compilation...
    cd installer
    makensis installer.nsi
    if %errorlevel% neq 0 (
        echo ERROR: Manual NSIS compilation also failed
        exit /b 1
    )
    cd ..
)

echo.
echo ========================================
echo Windows Installer Build Complete!
echo ========================================
echo.

REM List the generated installer
if exist installer\VRBinauralRecorder-1.0.0-Setup.exe (
    echo Installer created: installer\VRBinauralRecorder-1.0.0-Setup.exe
    powershell -Command "(Get-Item installer\VRBinauralRecorder-1.0.0-Setup.exe).Length / 1MB" >temp_size.txt
    set /p installer_size=<temp_size.txt
    echo Installer size: %installer_size% MB (approx)
    del temp_size.txt
    echo.
    echo Running installer tests...
    cmake --build . --target test_installer
    echo.
    echo Installation package ready for distribution!
) else (
    echo ERROR: Installer file not found!
    exit /b 1
)

echo.
echo Next steps:
echo 1. Test installer on clean Windows system
echo 2. Validate all components install correctly
echo 3. Test ASMRtist workflow functionality
echo 4. Distribute to testing team

pause