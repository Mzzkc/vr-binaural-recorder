# Windows Build Guide - MCP Remote Execution Approach

**Author:** Project Coordination Team
**Date:** 2025-10-22
**Purpose:** Build and test VR Binaural Recorder on Windows with VR hardware using MCP remote execution
**Status:** READY FOR EXECUTION

---

## Overview

This guide enables building and testing the VR Binaural Recorder on a Windows machine with VR hardware **directly from WSL** using Model Context Protocol (MCP) remote execution.

**Why MCP Instead of Cross-Compilation:**
- ✅ Real Windows environment (Visual Studio, not MinGW hacks)
- ✅ Immediate VR hardware testing on same machine
- ✅ Native Windows APIs (WASAPI, ASIO, OpenVR)
- ✅ Fast iteration (execute commands from WSL → instant Windows results)
- ❌ MinGW cross-compile has fatal issues with ASIO SDK, OpenVR

---

## Prerequisites

### On Windows Machine (with VR Hardware)

#### 1. Visual Studio Build Tools
**Required for C++ compilation**

```powershell
# Install via winget (recommended)
winget install Microsoft.VisualStudio.2022.BuildTools --override "--wait --passive --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"

# This installs:
# - MSVC C++ compiler
# - Windows SDK
# - CMake integration
# - MSBuild tools
```

**Verification:**
```powershell
# Check cl.exe (MSVC compiler) is available
where cl.exe

# Should show path like:
# C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.xx\bin\Hostx64\x64\cl.exe
```

#### 2. CMake
```powershell
winget install Kitware.CMake
```

#### 3. Git
```powershell
winget install Git.Git
```

#### 4. Node.js (for MCP server)
```powershell
winget install OpenJS.NodeJS
```

#### 5. NSIS (for installer creation)
```powershell
winget install NSIS.NSIS
```

#### 6. VR Requirements
- SteamVR installed
- VR headset connected (Oculus/Vive/Index)
- SteamVR can initialize headset

---

## MCP Server Setup

### Option 1: MCPControl (Recommended for Remote Execution)

#### Installation
```powershell
npm install -g mcp-control
```

#### Start Server
```powershell
# Local network access (simpler setup)
mcp-control --sse --port 3232

# OR secure/remote access with HTTPS
mcp-control --sse --port 3232 --https --cert path\to\cert.pem --key path\to\key.pem
```

#### Get Windows Machine IP
```powershell
ipconfig
# Note the IPv4 Address (e.g., 192.168.1.100)
```

### Option 2: Windows CLI MCP Server (Alternative)

```powershell
npm install -g @modelcontextprotocol/server-windows-cli

# Start with PowerShell support
mcp-server-windows-cli --shell powershell --port 3232
```

---

## WSL Configuration

### Configure MCP Client in WSL

Edit Claude Code config file (create if doesn't exist):

**File:** `~/.config/claude-code/config.json`

```json
{
  "mcpServers": {
    "windows-build-machine": {
      "transport": "sse",
      "url": "http://192.168.1.100:3232/mcp",
      "description": "Windows machine with VR hardware for building and testing"
    }
  }
}
```

Replace `192.168.1.100` with your actual Windows machine IP.

---

## Build Process

### Step 1: Sync Repository to Windows

**Option A: Git Clone on Windows**
```powershell
cd C:\Projects
git clone https://github.com/{your-repo}/vr-binaural-recorder.git
cd vr-binaural-recorder
git checkout recover-deleted-tests
```

**Option B: Access via WSL Network Share**
```powershell
# WSL filesystem accessible from Windows at:
cd \\wsl$\Ubuntu\home\{username}\Projects\vr-binaural-recorder

# Copy to Windows drive for better performance
xcopy /E /I \\wsl$\Ubuntu\home\{username}\Projects\vr-binaural-recorder C:\Projects\vr-binaural-recorder
```

### Step 2: Extract OpenVR SDK

```powershell
cd C:\Projects\vr-binaural-recorder\third_party

# Extract tarball (PowerShell 5.0+)
tar -xzf v1.23.7.tar.gz

# Create symlinks (run as Administrator)
New-Item -ItemType SymbolicLink -Path openvr -Target openvr-1.23.7
cd openvr-1.23.7
New-Item -ItemType SymbolicLink -Path include -Target headers
```

**Without Admin Rights (alternative):**
```powershell
# Use directory junction instead of symlink
mklink /J openvr openvr-1.23.7
cd openvr-1.23.7
mklink /J include headers
```

### Step 3: Configure CMake

```powershell
cd C:\Projects\vr-binaural-recorder

# Configure for Visual Studio 2022, 64-bit, Release build
cmake -B build -S . -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release

# Alternative: Use Ninja for faster builds
cmake -B build -S . -G "Ninja" -DCMAKE_BUILD_TYPE=Release
```

**Expected Output:**
```
-- The C compiler identification is MSVC ...
-- The CXX compiler identification is MSVC ...
-- Using OpenVR SDK for Windows: ...
-- Build spdlog: 1.11.0
-- Using system jsoncpp 1.9.5
-- Configuring done
-- Generating done
```

### Step 4: Build Test Executables

```powershell
# Build all test targets
cmake --build build --config Release

# OR build specific test targets
cmake --build build --config Release --target spatial_audio_validation_BLOCKING
cmake --build build --config Release --target ceo_spatial_validation
cmake --build build --config Release --target compilation_fixes_validation
cmake --build build --config Release --target vr_headset_tests
cmake --build build --config Release --target windows_rc_validation
cmake --build build --config Release --target audio_cockpit_validation
cmake --build build --config Release --target integration_tests
cmake --build build --config Release --target audio_performance_tests
```

**Build Output Location:**
```
build\bin\Release\
├── spatial_audio_validation_BLOCKING.exe
├── ceo_spatial_validation.exe
├── compilation_fixes_validation.exe
├── vr_headset_tests.exe
├── windows_rc_validation.exe
├── audio_cockpit_validation.exe
├── integration_tests.exe
└── audio_performance_tests.exe
```

### Step 5: Build Main Application

```powershell
cmake --build build --config Release --target vr_binaural_recorder
```

**Output:**
```
build\bin\Release\vr_binaural_recorder.exe
```

---

## Testing

### Test Execution Order (Per Sam Rivera's Strategy)

#### Phase 1: BLOCKING Tests (MUST PASS)

```powershell
cd build\bin\Release

# Test 1: Spatial Audio Validation (BLOCKING)
.\spatial_audio_validation_BLOCKING.exe
# Expected: [  PASSED  ] 6 tests
# If ANY fail: STOP - Deployment blocked

# Test 2: CEO Spatial Validation
.\ceo_spatial_validation.exe
# Expected: [  PASSED  ] 2 tests

# Test 3: Compilation Fixes Validation
.\compilation_fixes_validation.exe
# Expected: 6-8 tests pass (VR overlay may fail without SteamVR running)
```

**✅ ALL BLOCKING TESTS MUST PASS BEFORE PROCEEDING**

#### Phase 2: VR Hardware Tests (Requires SteamVR)

```powershell
# Start SteamVR first
# Ensure VR headset is connected and tracking

# Test 4: VR Headset Compatibility
.\vr_headset_tests.exe
# Tests: Quest Pro, Bigscreen Beyond, tracking jitter, latency

# Test 5: Audio Cockpit Validation (VR UI)
.\audio_cockpit_validation.exe
# Tests: Gesture controls, overlay rendering, VR workflow
```

#### Phase 3: Windows-Specific Tests

```powershell
# Test 6: Windows RC Validation (CRITICAL)
.\windows_rc_validation.exe
# Tests: WASAPI, audio devices, performance, COM/WMI APIs
# Expected: >90% pass rate (33+ tests)

# Test 7: Integration Tests
.\integration_tests.exe
# Full system integration

# Test 8: Audio Performance
.\audio_performance_tests.exe
# Latency <20ms, no dropouts
```

#### Phase 4: Full Suite with CTest

```powershell
cd C:\Projects\vr-binaural-recorder\build
ctest --build-config Release --verbose --output-on-failure
```

---

## Test Results Validation

### Success Criteria for Beta Release

**BLOCKING (Must Pass):**
- ✅ spatial_audio_validation_BLOCKING: 6/6 tests PASS
- ✅ ceo_spatial_validation: 2/2 tests PASS
- ✅ compilation_fixes_validation: ≥6/8 tests PASS (VR overlay fails acceptable)

**CRITICAL (Required for Production):**
- ✅ windows_rc_validation: >90% tests PASS (≥30/33)
- ✅ VR headset tests: PASS with real hardware
- ✅ Audio cockpit: VR UI functional

**HIGH (Important):**
- ✅ Integration tests: PASS
- ✅ Audio performance: Latency <20ms, no dropouts

### Known Acceptable Failures

**In WSL (Mock Mode):**
- VR overlay initialization (requires SteamVR)
- AudioRoutingOverlayIntegrationTest (2 tests) - Expected without VR runtime

**In Windows (Without VR Hardware):**
- VR tracking tests (require physical headset)
- Controller trigger tests

---

## Building Installer

### NSIS Installer Creation

```powershell
cd C:\Projects\vr-binaural-recorder

# Build installer target
cmake --build build --config Release --target windows_installer

# OR use NSIS directly
cd packaging\windows
makensis installer.nsi
```

**Output:**
```
packaging\windows\VRBinauralRecorder-1.0.0-Setup.exe
```

### Installer Testing

```powershell
# Test installer on clean Windows VM or fresh install
.\VRBinauralRecorder-1.0.0-Setup.exe /S  # Silent install

# Verify installation
"C:\Program Files\VR Binaural Recorder\vr_binaural_recorder.exe" --version

# Test uninstall
"C:\Program Files\VR Binaural Recorder\Uninstall.exe" /S
```

---

## Troubleshooting

### Build Issues

#### CMake Can't Find Visual Studio

```powershell
# List available generators
cmake --help

# Explicitly specify VS 2022
cmake -B build -S . -G "Visual Studio 17 2022" -A x64

# OR use vcvarsall.bat to setup environment
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
cmake -B build -S . -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
```

#### OpenVR SDK Not Found

```powershell
# Verify symlink/junction exists
dir third_party\openvr
dir third_party\openvr\include

# If missing, recreate
cd third_party
tar -xzf v1.23.7.tar.gz
mklink /J openvr openvr-1.23.7
cd openvr-1.23.7
mklink /J include headers
```

#### jsoncpp FetchContent Issues

CMakeLists.txt should use system jsoncpp on Windows:
```cmake
find_package(PkgConfig QUIET)
pkg_check_modules(JSONCPP jsoncpp)
```

If system jsoncpp not available, FetchContent fallback will trigger.

### Test Issues

#### VR Tests Fail with "SteamVR Not Installed"

```powershell
# Install SteamVR from Steam
# Start SteamVR before running VR tests
# Ensure headset is connected and tracking
```

#### Audio Tests Fail

```powershell
# Verify Windows audio service running
Get-Service | Where-Object {$_.Name -like "*audio*"}

# Check available audio devices
Get-AudioDevice -List
```

#### Permission Errors

```powershell
# Run PowerShell as Administrator for:
# - Creating symlinks
# - Installing to Program Files
# - Running installer tests
```

---

## MCP Execution Examples

### From WSL Claude Code

**Execute Windows Build Commands:**

```bash
# Claude Code will send commands via MCP to Windows machine

# Example commands that will run on Windows:
mcp-execute "cd C:\Projects\vr-binaural-recorder && cmake -B build -S . -G 'Visual Studio 17 2022' -A x64"
mcp-execute "cmake --build C:\Projects\vr-binaural-recorder\build --config Release"
mcp-execute "C:\Projects\vr-binaural-recorder\build\bin\Release\spatial_audio_validation_BLOCKING.exe"
```

**Check Build Status:**
```bash
mcp-execute "dir C:\Projects\vr-binaural-recorder\build\bin\Release"
```

---

## Success Validation

### Checklist Before Beta Release

- [ ] All BLOCKING tests pass (spatial_audio_validation_BLOCKING, ceo_spatial_validation)
- [ ] Windows RC validation >90% pass rate
- [ ] VR headset tests pass with real hardware
- [ ] Audio Cockpit VR UI functional
- [ ] Main application launches and connects to VR
- [ ] Installer builds successfully
- [ ] Installer tested on clean Windows system
- [ ] Documentation verified (executable names, paths, CLI flags match)
- [ ] LICENSE file created
- [ ] README.md exists

### Final Deliverables

1. **VRBinauralRecorder-1.0.0-Setup.exe** - Tested installer
2. **BETA_TESTING_GUIDE.md** - User instructions
3. **Test results** - All 8 test suites validated
4. **Known issues** - Documented limitations

---

## Quick Reference Commands

### Essential Windows Build Commands

```powershell
# Full build from scratch
cd C:\Projects\vr-binaural-recorder
cmake -B build -S . -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Run BLOCKING tests
cd build\bin\Release
.\spatial_audio_validation_BLOCKING.exe
.\ceo_spatial_validation.exe

# Build installer
cmake --build C:\Projects\vr-binaural-recorder\build --config Release --target windows_installer
```

### MCP Server Commands

```powershell
# Start MCPControl server
mcp-control --sse --port 3232

# Check server status
curl http://localhost:3232/health
```

---

## Next Steps After Successful Build

1. **Commit validated code** - Repository Manager can now safely commit 472 files
2. **Update documentation** - Alex Kim verifies executable names and paths
3. **Create beta package** - Installer + docs + test results
4. **Distribute to testers** - ASMRtists with VR hardware
5. **Collect feedback** - Bug reports, feature requests

---

## Estimated Timeline

| Task | Time | Dependencies |
|------|------|--------------|
| MCP Server Setup | 30 min | Windows machine accessible |
| Windows Build | 15 min | MCP server running |
| BLOCKING Tests | 5 min | Build complete |
| VR Hardware Tests | 15 min | SteamVR + headset |
| Windows RC Tests | 10 min | Native Windows |
| Installer Build | 5 min | All tests pass |
| Installer Testing | 15 min | Clean Windows VM |
| **TOTAL** | **~2 hours** | All prerequisites met |

**vs. Original estimate: 2 weeks for manual Windows migration**

---

## Support

**Issues? Check:**
1. beta-coordination/build-status.md - Linux build lessons
2. beta-coordination/test-strategy.md - Test requirements and priorities
3. beta-coordination/integration-report.md - Cross-team dependencies
4. docs/WINDOWS_VR_SETUP_GUIDE.md - VR setup and diagnostics

**Wolf Prevention:**
- Never commit without test validation
- Always verify spatial audio works (no 252.728 = 252.728)
- Test on real Windows, not just WSL mock mode
- Validate with VR hardware before claiming VR functionality works
