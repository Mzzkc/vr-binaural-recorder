# Windows User Testing Guide - VR Binaural Recorder

## Overview

**Purpose**: Validate the core ASMRtist workflow - transform microphone input into spatialized audio using VR positioning.

**Not Testing**: Complex VR features or technical showcases. Focus on real-world content creator usability.

**Target Users**: ASMRtists and content creators who want to add spatial audio effects to their recordings.

---

## Setup Instructions

### System Requirements
- **Windows 10 (1903+) or Windows 11 (64-bit only)**
- **4GB RAM minimum, 8GB+ recommended**
- **500MB free disk space**
- **VR headset** (Oculus, HTC Vive, Windows Mixed Reality, or Steam VR compatible)
- **Microphone** (any USB/audio interface microphone)

### Installation Process

1. **Download** the installer: `VRBinauralRecorder-1.0.0-Setup.exe`

2. **Run as Administrator**:
   - Right-click installer → "Run as administrator"
   - Windows may show security warnings - this is normal for unsigned software

3. **Follow installer prompts**:
   - Accept license agreement
   - Select installation components:
     - ✅ **Core Application** (Required)
     - ✅ **Visual C++ Redistributable** (Required - auto-installs if missing)
     - ✅ **OpenVR Runtime** (Essential for VR - installs SteamVR if missing)
     - ⚪ **Windows Audio Virtual Device Driver** (Optional - for system-wide capture)
     - ⚪ **ASIO Driver Support** (Optional - for professional audio interfaces)
   - Choose installation directory (default recommended: `C:\Program Files\VR Binaural Recorder`)

4. **Complete installation**:
   - Let installer finish all components
   - Launch application when prompted (or launch manually from Start Menu)

### First Launch Setup

1. **VR Setup**:
   - Ensure VR headset is connected and SteamVR is running
   - If no VR headset: App will run in "desktop mode" with mouse simulation

2. **Audio Setup**:
   - Connect and test your microphone
   - Verify headphones/speakers are working

---

## Feature Overview

### What This Application Does

**Core Function**: Turn any microphone into a spatial audio source by positioning it in VR space.

**ASMRtist Workflow**:
1. Put on VR headset
2. Position virtual microphone in 3D space using hand controllers
3. Move around to hear different spatial audio effects
4. Record with spatial positioning applied
5. Export spatially-enhanced audio for use in videos/streams

### What to Expect

- **Simple Interface**: Minimal VR overlay focused on microphone positioning
- **Real-time Audio**: Immediate feedback as you move the virtual microphone
- **Low Latency**: <10ms delay from input to spatially-processed output
- **Easy Recording**: One-button recording with spatial effects applied
- **Desktop Fallback**: Works without VR using mouse controls

---

## Testing Scenarios

### Test 1: Basic Installation & Launch
**Goal**: Verify the application installs and launches correctly

**Steps**:
1. Install using provided installer
2. Launch from Start Menu: "VR Binaural Recorder"
3. Check main interface appears

**Success Criteria**:
- ✅ Installation completes without errors
- ✅ Application launches within 10 seconds
- ✅ Main interface displays properly
- ✅ No error messages on startup

**Log Issues**: Installation problems, missing files, startup crashes

---

### Test 2: VR Hardware Detection
**Goal**: Verify VR headset is detected and tracking works

**Steps**:
1. Connect VR headset and ensure SteamVR is running
2. Put on headset
3. Look for VR overlay interface
4. Move head around to test tracking

**Success Criteria**:
- ✅ VR headset detected automatically
- ✅ Head tracking works smoothly
- ✅ VR overlay appears in headset
- ✅ Interface follows head movement

**Fallback Test** (no VR):
- ✅ App runs in desktop mode
- ✅ Mouse controls work for positioning
- ✅ Clear messaging about VR not available

**Log Issues**: VR detection problems, tracking glitches, overlay positioning

---

### Test 3: Microphone Input Detection
**Goal**: Validate microphone input is working and being processed

**Steps**:
1. Connect microphone (USB or audio interface)
2. Check audio input levels in application
3. Speak/make sounds into microphone
4. Verify input is being detected

**Success Criteria**:
- ✅ Microphone detected automatically
- ✅ Input levels show activity when speaking
- ✅ Clear audio quality (no distortion/noise)
- ✅ Adjustable input gain if needed

**Log Issues**: Microphone not detected, poor audio quality, no input signal

---

### Test 4: Core ASMRtist Workflow - Spatial Positioning
**Goal**: Test the main feature - positioning microphone in VR space

**Steps**:
1. With VR headset on and microphone working
2. Locate virtual microphone in VR space (should appear as visual element)
3. Use VR controllers to grab and move the microphone to different positions:
   - Close to left ear
   - Close to right ear
   - Above head
   - In front of face
   - Behind head
4. Speak into real microphone while moving virtual position
5. Listen to how spatial effects change

**Success Criteria**:
- ✅ Virtual microphone appears clearly in VR
- ✅ Controllers can grab and move microphone smoothly
- ✅ Spatial audio effects are immediately audible
- ✅ Left ear positioning creates strong left-channel effect
- ✅ Right ear positioning creates strong right-channel effect
- ✅ Distance affects volume/intimacy of effect
- ✅ No audio dropouts or glitches during positioning

**CRITICAL**: This is the core feature - spatial positioning must work flawlessly

**Log Issues**: Controller interaction problems, spatial effects not working, audio glitches

---

### Test 5: Real-time Spatial Audio Quality
**Goal**: Validate the quality and responsiveness of spatial audio processing

**Steps**:
1. Position virtual microphone close to left ear
2. Speak continuously while slowly moving your head left and right
3. Listen for smooth audio transitions
4. Move microphone to different positions while speaking
5. Test with different sound types:
   - Whispers (quiet sounds)
   - Normal speech
   - Tapping/clicking sounds
   - Paper rustling

**Success Criteria**:
- ✅ Smooth audio transitions (no pops/clicks)
- ✅ Latency feels immediate (<10ms perceived delay)
- ✅ Quiet sounds (whispers) processed clearly
- ✅ Spatial effects are convincing and immersive
- ✅ Audio quality maintains fidelity (not muffled/distorted)

**Log Issues**: High latency, audio artifacts, poor spatial quality

---

### Test 6: Recording Workflow
**Goal**: Test the recording functionality with spatial effects

**Steps**:
1. Position virtual microphone for good ASMR effect (close to ear)
2. Start recording using application controls
3. Create 30-second test recording:
   - Speak normally for 10 seconds
   - Move virtual microphone to different position
   - Speak for another 10 seconds
   - Move microphone again
   - Speak for final 10 seconds
4. Stop recording
5. Play back recorded audio
6. Verify spatial effects are captured in recording

**Success Criteria**:
- ✅ Recording starts/stops reliably
- ✅ Recorded audio contains spatial positioning effects
- ✅ Playback matches real-time experience
- ✅ Audio file saved to accessible location
- ✅ Recording includes all spatial changes made during session

**Log Issues**: Recording failures, spatial effects not captured, file save problems

---

### Test 7: Performance & Stability
**Goal**: Validate system performance and stability under normal use

**Steps**:
1. Run application for 15+ minutes of continuous use
2. Monitor system performance (CPU/memory usage)
3. Test various scenarios:
   - Frequent microphone repositioning
   - Speaking continuously while moving in VR
   - Multiple recording sessions
   - Switching between VR and desktop modes

**Success Criteria**:
- ✅ CPU usage <25% on recommended hardware
- ✅ Memory usage stable (no memory leaks)
- ✅ VR frame rate maintains 90fps (no dropped frames)
- ✅ Audio remains glitch-free during intensive use
- ✅ No application crashes or freezes

**Log Issues**: High CPU usage, memory leaks, frame rate drops, crashes

---

## Troubleshooting

### Installation Issues

**Problem**: "Installation failed" or "Missing dependencies"
**Solution**:
- Ensure running as Administrator
- Temporarily disable antivirus during installation
- Check available disk space (need 500MB minimum)

**Problem**: "VR Runtime not found"
**Solution**:
- Install SteamVR manually from Steam
- Restart computer after VR software installation
- Ensure VR headset is connected before launch

**Problem**: Windows Defender blocks installation
**Solution**:
- Temporarily disable Real-time Protection
- Add installer to Windows Defender exclusions
- This is normal for unsigned software

### Runtime Issues

**Problem**: "No microphone detected"
**Solution**:
- Check Windows audio settings - ensure microphone is set as default
- Try different USB port for USB microphones
- Update audio drivers
- Test microphone in Windows Sound settings first

**Problem**: "VR headset not detected"
**Solution**:
- Ensure SteamVR is running and headset is connected
- Check headset drivers are up to date
- Test VR headset in SteamVR Home first
- Try restarting SteamVR service

**Problem**: High latency or audio glitches
**Solution**:
- Close other audio applications (Discord, music players)
- Reduce buffer size in audio settings if available
- Update audio drivers
- Check for Windows audio driver conflicts

**Problem**: Spatial effects not working
**Solution**:
- Ensure headphones (not speakers) are being used
- Check that stereo output is enabled (not mono)
- Test with different headphones
- Verify virtual microphone is actually being moved in VR

### Performance Issues

**Problem**: Low VR frame rate or stuttering
**Solution**:
- Close other applications to free up resources
- Lower VR headset display resolution temporarily
- Check graphics driver updates
- Ensure power plan is set to "High Performance"

**Problem**: Application crashes or freezes
**Solution**:
- Check Windows Event Viewer for error details
- Ensure all Windows updates are installed
- Try running without VR first (desktop mode)
- Verify system meets minimum requirements

---

## Feedback Collection

### Required Information

Please provide the following information with your test results:

**System Information**:
- Windows version (exact build number)
- VR headset model and version
- Audio interface/microphone type
- Graphics card and driver version
- Available RAM and CPU specs

**Testing Results**:
- Which test scenarios passed/failed
- Specific error messages encountered
- Performance observations (smooth/laggy/choppy)
- Audio quality assessment
- Overall user experience rating (1-10)

### Specific Feedback Areas

**Installation Experience**:
- How long did installation take?
- Were instructions clear?
- Any confusing steps?

**Core ASMRtist Workflow**:
- Was microphone positioning intuitive?
- How convincing were the spatial effects?
- Would this be useful for your content creation?

**Audio Quality**:
- Rate audio fidelity (1-10)
- Any audio artifacts or glitches?
- Latency acceptable for real-time use?

**VR Experience**:
- Controllers comfortable for microphone positioning?
- VR interface clear and uncluttered?
- Any motion sickness or eye strain?

**Recording Functionality**:
- Recording workflow easy to use?
- Captured audio matched real-time experience?
- File formats and quality suitable?

### Bug Reports

**For each issue, please include**:
1. **Steps to reproduce** - exact sequence that causes the problem
2. **Expected behavior** - what should have happened
3. **Actual behavior** - what actually happened
4. **Screenshots/logs** - any error messages or visual issues
5. **Frequency** - does it happen every time or intermittently?

### Success Stories

**Please also share**:
- Features that worked particularly well
- Aspects that exceeded expectations
- Creative ways you used the spatial positioning
- How this would fit into your content creation workflow

---

## Test Completion Checklist

**Required Tests** (must pass for RC approval):
- [ ] Installation completes successfully
- [ ] Application launches without errors
- [ ] VR headset detection and tracking works
- [ ] Microphone input detection and processing
- [ ] Core spatial positioning workflow functions
- [ ] Real-time spatial audio effects are convincing
- [ ] Recording captures spatial effects properly
- [ ] Performance is acceptable for content creation use

**Optional Tests** (helpful for feedback):
- [ ] Desktop fallback mode works
- [ ] Different microphone types tested
- [ ] Extended stability testing
- [ ] Creative workflow integration tested

**Overall Assessment**:
- **Would you use this for ASMR/spatial audio content creation?** Yes/No
- **Rate overall experience** (1-10): ___
- **Primary strength**: _______________
- **Primary weakness**: ______________
- **Recommendation**: Release/Needs Work/Not Ready

---

## Contact & Support

**For technical issues during testing**:
- Create detailed bug reports with system information
- Include screenshots of error messages
- Note exact steps to reproduce issues

**For feedback submission**:
- Email test results and feedback to development team
- Include system specifications and test completion checklist
- Provide honest assessment of ASMRtist workflow viability

**Priority Issues**:
1. Installation failures (prevents testing)
2. Core spatial positioning not working (breaks main feature)
3. VR detection problems (blocks VR workflow)
4. Recording failures (prevents output creation)
5. Performance issues (makes unusable for content creation)

Thank you for helping validate the VR Binaural Recorder for the ASMRtist community!