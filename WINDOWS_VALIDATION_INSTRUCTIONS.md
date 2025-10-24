# Windows Validation Instructions - VR Binaural Recorder Beta

**Purpose:** Final validation before beta deployment
**Estimated Time:** 30-45 minutes
**Platform:** Native Windows 10/11 (NOT WSL)
**Critical:** Must prove spatial audio works on Windows with L≠R differentiation

---

## Prerequisites

### Required Software
- **Windows 10/11** (64-bit)
- **Visual Studio 2019 or 2022** with C++ Desktop Development workload
- **CMake 3.16+** (https://cmake.org/download/)
- **Git for Windows** (already installed)
- **NSIS** (for installer generation, optional for testing)

### Quick Install Commands (PowerShell as Administrator)
```powershell
# Install CMake via winget (Windows 10 version 1809+)
winget install Kitware.CMake

# Or install via Chocolatey
choco install cmake

# Verify installation
cmake --version  # Should show 3.16 or higher
```

---

## Phase 1: Windows Build Execution (15-20 minutes)

### Step 1: Navigate to Project Directory

Open **PowerShell** (NOT WSL) and navigate to project:

```powershell
# From WSL path, convert to Windows path
# WSL: /home/emzi/Projects/vr-binaural-recorder
# Windows: C:\Users\emzi\Projects\vr-binaural-recorder (or wherever mounted)

# Example if using WSL mount:
cd \\wsl$\Ubuntu\home\emzi\Projects\vr-binaural-recorder

# Or if cloned directly on Windows:
cd C:\Users\YourUsername\Projects\vr-binaural-recorder
```

### Step 2: Clean Build Directory

```powershell
# Remove old build artifacts
if (Test-Path build) { Remove-Item -Recurse -Force build }

# Create fresh build directory
mkdir build
cd build
```

### Step 3: Configure with CMake

```powershell
# Configure for Visual Studio 2019 (adjust if using VS 2022)
cmake .. -G "Visual Studio 16 2019" -A x64

# OR for Visual Studio 2022:
cmake .. -G "Visual Studio 17 2022" -A x64

# Expected output:
# - "Found OpenVR" or "Downloading OpenVR SDK..."
# - "Found PortAudio"
# - "Found jsoncpp"
# - "Configuring done"
# - "Generating done"
```

**TROUBLESHOOTING:**
- **If CMake errors about missing dependencies:** They should auto-download via FetchContent
- **If GLEW is required:** May need to add `-DGLEW_INCLUDE_DIR` and `-DGLEW_LIBRARY` flags
- **If OpenVR download fails:** Check internet connection, may need manual download

### Step 4: Build Release Configuration

```powershell
# Build the project (this will take 5-10 minutes)
cmake --build . --config Release --parallel

# Watch for:
# ✅ "Build succeeded" at the end
# ❌ Any compilation errors (especially in windows_wasapi_virtual_device.cpp)
# ⚠️ Warnings about double→float conversions (expected, non-blocking)
```

**EXPECTED WARNINGS (Non-Critical):**
- `warning C4244: conversion from 'double' to 'float'` - Already fixed in commits, OK
- `warning C4267: conversion from 'size_t' to 'int'` - Safe, OK

**CRITICAL ERRORS TO WATCH FOR:**
- Missing GLEW headers → May need to install GLEW separately
- jsoncpp link errors → Should use jsoncpp_static (already fixed in commit 7af23d3)
- OpenVR API not found → Ensure OpenVR auto-download completed

### Step 5: Verify Build Artifacts

```powershell
# Check that executables were created
dir bin\Release\*.exe

# Expected files:
# - vr_binaural_recorder.exe (main application)
# - vr_binaural_tests.exe (test suite, if tests built)
# - spatial_audio_validation_BLOCKING.exe (BLOCKING test)
# - ceo_spatial_validation.exe (BLOCKING test)
```

**IF EXECUTABLES MISSING:**
- Check build output for errors
- Verify CMake configuration included tests: `BUILD_TESTING=ON`
- May need to build test targets explicitly

---

## Phase 2: BLOCKING Test Execution (10-15 minutes)

### Critical Tests Required

These tests MUST pass with L≠R differentiation to deploy beta:

1. **spatial_audio_validation_BLOCKING.exe** - Proves spatial audio is NOT stub
2. **ceo_spatial_validation.exe** - Validates ASMRtist workflow

### Step 6: Run Spatial Audio Validation Test

```powershell
# Navigate to test directory
cd bin\Release

# Run BLOCKING spatial audio test
.\spatial_audio_validation_BLOCKING.exe

# Capture ALL output to file for analysis
.\spatial_audio_validation_BLOCKING.exe > ..\..\spatial_validation_output.txt 2>&1
```

**CRITICAL: What to Look For**

✅ **SUCCESS PATTERN (Expected):**
```
Position: FAR LEFT (-1, 0, 0)
  Left channel energy:  13047.9
  Right channel energy:  5512.75
  Ratio: 2.37x  ← Left channel 137% louder

Position: FAR RIGHT (1, 0, 0)
  Left channel energy:   5512.75
  Right channel energy: 13047.9
  Ratio: 2.37x  ← Right channel 137% louder

[PASS] Spatial differentiation validated
```

🔴 **FAILURE PATTERN (Wolf Prevention - BLOCKS DEPLOYMENT):**
```
Position: FAR LEFT (-1, 0, 0)
  Left channel energy:  252.728
  Right channel energy:  252.728  ← IDENTICAL VALUES
  Ratio: 1.00x

[FAIL] No spatial differentiation detected
```

**Analysis:**
- **Ratio > 1.1x** = MINIMUM acceptable spatial separation
- **Ratio ~2.37x** = EXPECTED (proven on Linux)
- **Ratio = 1.00x** (identical L/R) = CRITICAL FAILURE (stub implementation)

### Step 7: Run CEO Spatial Validation Test

```powershell
# Run ASMRtist workflow validation
.\ceo_spatial_validation.exe

# Capture output
.\ceo_spatial_validation.exe > ..\..\ceo_validation_output.txt 2>&1
```

**CRITICAL: What to Look For**

✅ **SUCCESS:**
```
[TEST] ASMRtist Workflow Simulation
[PASS] Head tracking integration
[PASS] Audio level meters functional
[PASS] Virtual microphone positioning
[PASS] L/R channel separation (ratio: X.XXx)
All tests passed!
```

🔴 **FAILURE:**
```
[FAIL] Any test failure
[ERROR] Spatial audio processing error
```

### Step 8: Collect Evidence

```powershell
# Copy both output files to project root for review
cd ..\..
Copy-Item spatial_validation_output.txt .
Copy-Item ceo_validation_output.txt .

# Display outputs for immediate review
Write-Host "=== SPATIAL AUDIO VALIDATION ==="
Get-Content spatial_validation_output.txt

Write-Host "`n=== CEO SPATIAL VALIDATION ==="
Get-Content ceo_validation_output.txt
```

---

## Phase 3: QA Analysis & Deployment Decision (5 minutes)

### Step 9: Analyze Test Results

**Open the output files and verify:**

1. **Wolf Prevention Check:**
   - ✅ PASS: L/R channel energy values are DIFFERENT
   - 🔴 FAIL: L/R channel energy values are IDENTICAL (252.728 = 252.728 pattern)

2. **Spatial Differentiation Check:**
   - ✅ PASS: Ratio ≥ 1.1x (minimum threshold)
   - ✅ EXCELLENT: Ratio ~2.37x (matches Linux results)
   - ⚠️ MARGINAL: Ratio 1.1x - 1.5x (acceptable but investigate)
   - 🔴 FAIL: Ratio < 1.1x (insufficient spatial separation)

3. **Test Completion Check:**
   - ✅ PASS: All tests show "[PASS]" status
   - 🔴 FAIL: Any "[FAIL]" status

### Step 10: Final GO/NO-GO Decision

**IF ALL CHECKS PASS:**
```
🟢 GREEN LIGHT - DEPLOY BETA

Evidence:
✅ Windows build succeeded
✅ Spatial audio validation: L≠R differentiation proven (ratio: X.XXx)
✅ CEO workflow validation: All tests passed
✅ Wolf prevention: No 252.728 = 252.728 pattern detected
✅ Documentation accurate (no false promises)

ACTION: Proceed to beta installer generation and deployment
```

**IF ANY CHECK FAILS:**
```
🔴 RED LIGHT - BLOCK DEPLOYMENT + INVESTIGATE

Failed Check: [Specific failure]
Evidence: [Copy relevant test output]

ACTION:
1. Investigate root cause of failure
2. Fix implementation
3. Re-run validation tests
4. DO NOT DEPLOY until all tests pass
```

---

## Phase 4: Beta Installer Generation (Optional - 5 minutes)

### Step 11: Build Windows Installer (If Tests Pass)

```powershell
# Navigate back to build directory
cd build

# Build installer target
cmake --build . --config Release --target windows_installer

# Installer will be created in:
# packaging/VRBinauralRecorder-Windows-x64-Setup.exe
```

**Verify Installer:**
```powershell
# Check installer exists
dir packaging\VRBinauralRecorder-Windows-x64-Setup.exe

# Note file size (should be 40-60MB)
```

---

## Expected Timeline Summary

| Phase | Task | Time | Status |
|-------|------|------|--------|
| 1 | Windows build execution | 15-20 min | Pending |
| 2 | BLOCKING tests execution | 10-15 min | Pending |
| 3 | QA analysis & decision | 5 min | Pending |
| 4 | Installer generation (optional) | 5 min | Pending |
| **TOTAL** | **End-to-end validation** | **30-45 min** | **Ready** |

---

## Troubleshooting Common Issues

### Issue: CMake can't find Visual Studio

```powershell
# List available generators
cmake --help

# Use specific generator
cmake .. -G "Visual Studio 17 2022" -A x64
```

### Issue: OpenVR download fails

```powershell
# Check cmake/OpenVRDownload.cmake for download URL
# May need to manually download and extract to third_party/openvr/
```

### Issue: Build fails with "GLEW not found"

```powershell
# GLEW may be required for OpenGL support
# Install via vcpkg:
vcpkg install glew:x64-windows

# Then configure with:
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake
```

### Issue: Tests executable not found

```powershell
# Tests may not be enabled, rebuild with:
cmake .. -DBUILD_TESTING=ON
cmake --build . --config Release --parallel
```

### Issue: Application crashes on launch

```powershell
# Check dependencies are in same directory as .exe:
dir bin\Release\

# Should see:
# - openvr_api.dll
# - vr_binaural_config.json
# - hrtf_data/ folder
```

---

## Reporting Results

### Success Report Template

```markdown
# Windows Validation - SUCCESS ✅

**Build Status:** PASSED
**Spatial Audio Test:** PASSED (ratio: X.XXx)
**CEO Validation Test:** PASSED
**Wolf Prevention:** PASSED (no identical L/R values)

**Evidence Files:**
- spatial_validation_output.txt
- ceo_validation_output.txt

**Deployment Decision:** 🟢 GREEN - READY FOR BETA DEPLOYMENT
```

### Failure Report Template

```markdown
# Windows Validation - FAILED ❌

**Build Status:** [PASSED/FAILED]
**Spatial Audio Test:** [PASSED/FAILED] (ratio: X.XXx)
**CEO Validation Test:** [PASSED/FAILED]
**Wolf Prevention:** [PASSED/FAILED]

**Failure Details:**
[Copy relevant error messages and test output]

**Deployment Decision:** 🔴 RED - BLOCK DEPLOYMENT
**Required Action:** [Describe investigation needed]
```

---

## Next Steps After Validation

### If Tests Pass (🟢 GREEN):
1. Generate Windows installer (`cmake --build . --target windows_installer`)
2. Test installer on clean Windows system (optional but recommended)
3. Update STATUS.md with deployment approval
4. Begin ASMRtist beta tester outreach
5. Archive coordination-workspace/ to docs/archive/

### If Tests Fail (🔴 RED):
1. Capture full error logs
2. Update STATUS.md with blocker details
3. Investigate root cause using RLF deep analysis
4. Fix implementation issues
5. Re-run validation process
6. DO NOT PROCEED to deployment until validation passes

---

## Contact & Support

**Questions during validation?**
- Review this document (WINDOWS_VALIDATION_INSTRUCTIONS.md)
- Check build logs in `build/` directory
- Check application logs (if app runs): `%APPDATA%\VRBinauralRecorder\logs\`

**Validation complete?**
- Report results in project STATUS.md
- Update deployment decision based on test outcomes
- Proceed to installer generation if GREEN LIGHT

---

**CRITICAL REMINDER:** The spatial audio MUST show L≠R differentiation. If you see identical values (252.728 = 252.728), this is the "wolf prevention pattern" indicating stub implementation. This is a DEPLOYMENT BLOCKER and requires immediate investigation.

**Good luck with validation!** 🚀
