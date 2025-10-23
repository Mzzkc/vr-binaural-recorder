# CRITICAL TEST CASE: END-TO-END ASMR WORKFLOW ON CLEAN WINDOWS
## Jordan Taylor - Senior QA Engineer

**TEST PRIORITY: CRITICAL** - This test validates our complete ASMRtist value proposition

---

## TEST SPECIFICATION

**Test Name**: ASMR_WORKFLOW_CLEAN_INSTALL
**Environment**: Clean Windows 10/11 machine with ONLY Steam + SteamVR
**Duration**: 15 minutes maximum (including setup)
**Success Criteria**: User creates spatial ASMR recording using VR head movement

**Why this test is make-or-break:**
- Validates all three layers simultaneously (VR tracking + UI overlays + packaging)
- Proves the core ASMRtist value proposition actually works
- Catches integration failures (stub overlays, missing DLLs, dependency issues)
- Tests real user experience, not just technical functionality

---

## PHASE 1: INSTALLATION VALIDATION (5 minutes)

### Step 1.1: Download and Install
1. Download VR Binaural Recorder installer on completely clean Windows machine
2. Run installer as administrator
3. **EXPECTED**: Installation completes without "missing dependency" errors
4. **EXPECTED**: Installation directory contains `openvr_api.dll` (validates packaging fixes)

### Step 1.2: Initial Launch
1. Launch VR Binaural Recorder executable
2. **EXPECTED**: Application starts without "missing DLL" errors
3. **EXPECTED**: Application detects SteamVR availability (may show "VR not connected" - OK)
4. **EXPECTED**: Windows Audio panel shows "VR Binaural Recorder" virtual audio device

### Step 1.3: Dependency Verification
1. Check installation directory for critical files:
   - ✅ `vr_binaural_recorder.exe`
   - ✅ `openvr_api.dll` (critical VR runtime)
   - ✅ `hrtf/` directory with HRTF data files
   - ✅ `vr_binaural_config.json.example`

**FAIL CRITERIA**: Any missing files = packaging failure

---

## PHASE 2: VR INTEGRATION VALIDATION (5 minutes)

### Step 2.1: VR Hardware Connection
1. Connect VR headset and ensure SteamVR is running
2. Put on VR headset while VR Binaural Recorder is running
3. **EXPECTED**: VR overlay appears showing microphone visualization
4. **EXPECTED**: NO black screen (validates Alex's UI stub elimination)

### Step 2.2: Head Tracking Validation
1. Move head left and right while watching audio meters in application
2. **EXPECTED**: Audio meters show immediate response to head movement
3. **EXPECTED**: L/R channel differences visible in real-time
4. **EXPECTED**: No lag between head movement and audio processing

### Step 2.3: VR Controls Access
1. Look for VR recording controls in overlay
2. **EXPECTED**: Record button visible and interactive
3. **EXPECTED**: Monitor button visible and interactive
4. **EXPECTED**: Microphone visualization appears as blue orb

**FAIL CRITERIA**: Black screen, no controls, or no head tracking response = Layer 2 failure

---

## PHASE 3: CORE FUNCTIONALITY VALIDATION (5 minutes)

### Step 3.1: Spatial Recording Test
1. Start recording using VR controls (trigger on Record button)
2. Speak into microphone while slowly moving head in circle
3. Record for 30 seconds while maintaining head movement
4. Stop recording using VR controls

**EXPECTED BEHAVIORS:**
- Recording indicator shows pulsing red "REC" text
- Audio levels respond to voice input
- Head movement continues to affect spatial processing during recording

### Step 3.2: Playback Validation
1. Save recording and exit VR application
2. Play back recorded file using standard audio player (Windows Media Player, etc.)
3. **CRITICAL SUCCESS CRITERIA**:
   - Left and right channels contain DIFFERENT audio content
   - Spatial positioning changes are audible as head moved during recording
   - Audio quality is clear without dropouts or glitches

### Step 3.3: OBS Integration Test (Bonus)
1. Open OBS Studio
2. Add "Audio Input Capture" source
3. Select "VR Binaural Recorder" virtual audio device
4. Start VR application and test head movement
5. **EXPECTED**: OBS shows L/R channel activity with phase differences

**FAIL CRITERIA**: Mono output or identical L/R channels = spatial processing failure

---

## EDGE CASE VALIDATION

### No VR Hardware Test
1. Disconnect VR headset
2. Launch VR Binaural Recorder
3. **EXPECTED**: Graceful fallback to desktop mode with clear message
4. **EXPECTED**: Application doesn't crash or show confusing errors

### Audio Conflict Test
1. Open Discord or music application (creates audio exclusive access conflict)
2. Launch VR Binaural Recorder
3. **EXPECTED**: Clear error message explaining audio device conflict
4. **EXPECTED**: Suggestion to close other audio applications

### SteamVR Crash Test
1. Start recording session
2. Force-close SteamVR process
3. **EXPECTED**: Recording continues or saves gracefully
4. **EXPECTED**: Clear error message about VR connection loss

---

## PERFORMANCE VALIDATION

### Frame Rate Test
- VR tracking must maintain 90Hz
- Audio processing latency <10ms
- No frame drops during recording

### Memory Usage Test
- Record for 5 minutes continuously
- Memory usage should remain stable (<200MB)
- No gradual memory leaks

### CPU Usage Test
- CPU usage should remain <15% during VR recording
- No CPU spikes causing audio dropouts

---

## SUCCESS METRICS

### Technical Validation
- **Installation**: Completes without errors, all dependencies present
- **VR Integration**: Overlay appears, head tracking responds <5ms
- **Audio Processing**: Spatial audio confirmed via L/R channel differences
- **Performance**: 90Hz VR + <10ms audio latency maintained

### User Experience Validation
- **Setup Time**: <5 minutes from download to first recording
- **Learning Curve**: No manual controls needed (pure VR interaction)
- **Recording Quality**: Professional spatial audio output
- **Reliability**: No crashes during normal ASMRtist workflow

---

## FAILURE ANALYSIS

### Layer 1 (VR Tracking) Failures
- Head movement has no effect on audio meters
- VR headset not detected despite SteamVR working
- Tracking latency >20ms (noticeable lag)

### Layer 2 (UI Overlays) Failures
- Black screen in VR headset
- Controls not visible or interactive
- No microphone visualization

### Layer 3 (Packaging) Failures
- Missing `openvr_api.dll` causing launch failures
- Missing HRTF data causing mono output
- Installation errors on clean Windows

### Integration Failures
- Audio routes to headset speakers instead of spatial processing
- VR overlay conflicts with other VR applications
- Recording sessions crash or corrupt audio

---

## TESTER INSTRUCTIONS

1. **Use ONLY clean Windows machines** - existing dev environments mask dependency issues
2. **Test the complete ASMRtist workflow** - not individual components
3. **Validate spatial audio output** - mono output = test failure
4. **Report specific failure modes** - use Layer 1/2/3 classification
5. **Test performance under load** - record for several minutes continuously

**CRITICAL**: If testers cannot complete this 15-minute workflow successfully on a clean Windows machine, we have NOT achieved the ASMRtist vision and must identify/fix the failing layer before deployment.

---

**This test validates our core promise: "Transform any microphone source into spatialized audio based on where you are in virtual space" for ASMRtists.**

**Document prepared by Jordan Taylor - Senior QA Engineer**
**Last updated: 2025-09-27**
**Test validation status: CRITICAL FOR DEPLOYMENT**