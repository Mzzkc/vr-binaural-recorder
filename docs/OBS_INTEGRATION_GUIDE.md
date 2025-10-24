# OBS Integration Guide - VR Binaural Recorder
**Professional ASMR Streaming with Spatial Audio**

![OBS](https://img.shields.io/badge/OBS-27.0+-blue) ![Audio](https://img.shields.io/badge/audio-48kHz%20Binaural-orange) ![Streaming](https://img.shields.io/badge/streaming-ASMR%20Ready-green)

## 🎯 Quick Start for ASMRtist Streamers

**Goal**: Stream real-time spatial ASMR with VR head tracking directly to Twitch/YouTube

**Setup Time**: 10 minutes | **Works With**: OBS Studio 27.0+ | **Audio Quality**: Professional 48kHz binaural

---

## 🔊 Virtual Audio Device Setup

### Phase 1: VR Binaural Recorder Configuration (3 minutes)

**Start VR Binaural Recorder FIRST** to create the virtual audio device:

1. **Launch VR Binaural Recorder**
   - Use the desktop shortcut or Start Menu
   - Application automatically creates virtual audio device on startup
   - Wait for "Virtual Audio Device: Created successfully" message in log

2. **Verify virtual audio device creation**:
   ```batch
   REM Check Windows audio devices
   powershell "Get-AudioDevice -List | Where-Object {$_.Name -like '*VR Binaural*'}"
   ```

**Expected Result**: Windows Sound panel shows "VR Binaural Recorder (Virtual)" as available audio device

### Phase 2: Audio Device Validation (2 minutes)

**IMPORTANT**: The virtual device operates at 48kHz (matching HRTF processing)

**Verify Sample Rate**:
```batch
REM Check virtual device sample rate
powershell "Get-AudioDevice | Where-Object {$_.Name -like '*VR Binaural*'} | Format-Table Name, SampleRate"

REM Should show: SampleRate: 48000
```

**If virtual device doesn't appear:**
1. Check application log: `%APPDATA%\VRBinauralRecorder\logs\`
2. Look for "Virtual Audio Device: Created successfully"
3. If missing, close and restart VR Binaural Recorder
4. Ensure no other audio applications are blocking device creation

---

## 🎥 OBS Studio Configuration

### Method 1: Standard Audio Input Capture (Recommended)

**Step 1: Add Audio Source**
```
OBS → Sources → + → Audio Input Capture (WASAPI)
Name: "VR Binaural Audio"
Device: "VR Binaural Recorder (Virtual)"
```

**Step 2: Configure Audio Properties**
```
Right-click Audio Source → Filters → + → Noise Suppression → NVIDIA Noise Removal (Optional)
Audio → Advanced Audio Properties:
  - VR Binaural Audio: Sync Offset: 0ms
  - Monitor and Output: Off (prevent feedback)
```

**Step 3: Audio Mixer Setup**
```
Audio Mixer → Settings Icon → Advanced Audio Properties
VR Binaural Audio:
  - Volume: 0 dB (unity gain)
  - Sync Offset: Test and adjust if needed
  - Audio Monitoring: Monitor Off
```

### Method 2: Virtual Audio Cable Integration (Pro Setup)

**For complex audio routing with multiple microphones:**

```
1. Install VB-Audio Virtual Cable (free)
2. Route: Microphone → VR Binaural Recorder → Virtual Cable → OBS
3. Benefit: Mix multiple audio sources before spatial processing
```

---

## 🎙️ ASMRtist Streaming Workflow

### Pre-Stream Setup Checklist (5 minutes)

```batch
REM Create this as streaming_check.bat
@echo off
echo === ASMRtist Streaming Validation ===

echo [1] VR System Check
VRBinauralRecorder.exe --test-vr-system --duration 30

echo [2] Virtual Audio Device Test
VRBinauralRecorder.exe --test-virtual-audio --duration 15

echo [3] Binaural Processing Test
echo Starting spatial audio test - move your head to verify binaural panning...
VRBinauralRecorder.exe --test-binaural-streaming --duration 60

echo [4] OBS Audio Path Validation
echo Check OBS Audio Mixer for signal from VR Binaural Audio source
echo Expected: Green bars moving with head movement in VR

pause
```

### Live Streaming Configuration

**OBS Scene Setup for ASMR**:

```
Scene: "ASMR VR Session"
├── Video Capture Device (Webcam - optional)
├── Display Capture (VR Mirror for viewers)
├── Audio Input Capture (VR Binaural Audio)
├── Text (Now Playing, donation alerts)
└── Browser Source (Chat overlay)
```

**Stream Settings**:
```
Output:
  - Streaming: Hardware (NVENC/Quick Sync)
  - Audio Bitrate: 160 kbps (preserves spatial audio quality)
  - Sample Rate: 48kHz (CRITICAL - matches VR Binaural processing)

Audio:
  - Desktop Audio: Disabled (prevent game sounds)
  - Mic/Auxiliary Audio: VR Binaural Audio only
```

---

## 🔧 Binaural Audio Validation

### Phase 1: Static Test (2 minutes)

**Verify spatial processing WITHOUT VR movement:**

```batch
REM 1. Start VR Binaural Recorder
REM 2. Put on headphones (not VR headset yet)
REM 3. Play test pattern through microphone
VRBinauralRecorder.exe --play-test-pattern

Expected in OBS:
✅ Audio meters show activity
✅ Left/right channels have different content
✅ Waveform visualization shows stereo separation
```

### Phase 2: Dynamic Test (3 minutes)

**Verify head tracking affects audio in real-time:**

```batch
REM 1. Put on VR headset
REM 2. Start recording in OBS
REM 3. Speak into microphone while turning head 360°
REM 4. Watch OBS waveform visualization

Expected Results:
✅ Audio panning changes as you turn head
✅ Sound moves opposite to head movement (proper spatial behavior)
✅ No audio dropouts during head movement
✅ Continuous smooth panning (no jumps/stuttering)
```

### Phase 3: Stream Quality Test (5 minutes)

**Test end-to-end streaming quality:**

```batch
REM 1. Start OBS recording (not streaming yet)
REM 2. Record 5-minute VR ASMR test session
REM 3. Review recording file for quality

Quality Checklist:
✅ Audio-video sync is maintained
✅ No clicking/popping during head movement
✅ Spatial effect is preserved in recording
✅ Background noise is minimal
✅ Voice clarity is professional quality
```

---

## 🚨 Troubleshooting Common Issues

### Problem 1: "No audio in OBS from VR Binaural source"

**Diagnosis Steps**:

```batch
REM Check virtual device status
powershell "Get-AudioDevice -List | Where-Object {$_.Name -like '*VR Binaural*'}"

REM Check OBS audio source configuration
REM OBS → Sources → Audio Input Capture → Properties
REM Verify device shows "VR Binaural Recorder (Virtual)"
```

**Solutions by Priority**:

1. **Restart Virtual Audio Device**
   ```batch
   REM Close VR Binaural Recorder
   taskkill /f /im VRBinauralRecorder.exe

   REM Wait 5 seconds, restart
   timeout /t 5
   VRBinauralRecorder.exe --streaming-mode
   ```

2. **Reset OBS Audio Source**
   ```
   OBS → Sources → Right-click "VR Binaural Audio" → Remove
   Add new: Audio Input Capture → Select VR Binaural device
   ```

3. **Check Windows Audio Exclusive Mode**
   ```
   Windows → Sound Settings → VR Binaural device → Device properties
   → Additional device properties → Advanced
   → Uncheck "Allow applications to take exclusive control"
   ```

### Problem 2: "Audio sounds like normal stereo, not spatial"

**Diagnosis**: Binaural processing disabled or bypassed

**Solutions**:

1. **Verify HRTF Dataset**
   ```batch
   REM Check HRTF data exists
   dir "C:\Program Files\VR Binaural Recorder\deps\hrtf\*.wav"

   REM Should show 360+ .wav files
   ```

2. **Check VR Tracking Status**
   ```batch
   VRBinauralRecorder.exe --debug-tracking

   REM Expected output:
   REM "VR System: Connected"
   REM "Head tracking: Active"
   REM "Spatial processing: Enabled"
   ```

3. **Reset Spatial Audio Engine**
   ```
   VR Binaural Recorder → Settings → Audio → Reset Spatial Engine
   Or: VRBinauralRecorder.exe --reset-hrtf-processor
   ```

### Problem 3: "Audio cutting out during head movement"

**Diagnosis**: Performance issue or USB bandwidth limitation

**Solutions**:

1. **Performance Optimization**
   ```batch
   REM Set high priority for VR app
   wmic process where name="VRBinauralRecorder.exe" CALL setpriority "high priority"

   REM Reduce OBS encoding load
   REM OBS → Output → Streaming → Encoder: Hardware (NVENC)
   ```

2. **USB Bandwidth Management**
   ```
   REM Check VR headset USB connection
   Device Manager → Universal Serial Bus controllers
   → Look for USB 3.0 ports being used by VR headset
   → Try different USB port if multiple available
   ```

3. **Buffer Size Adjustment**
   ```
   VR Binaural Recorder → Settings → Audio → Buffer Size: 256 (increase)
   Note: Higher buffer = more latency but more stable
   ```

---

## 🎬 Advanced Streaming Features

### Multi-Microphone Setup

**For professional ASMR studios with multiple microphones:**

```
Setup Flow:
Hardware Mics → Audio Interface → VR Binaural Recorder → OBS

Configuration:
1. Connect all mics to audio interface (Focusrite, PreSonus, etc.)
2. Set audio interface as input in VR Binaural Recorder
3. Use VR head tracking for spatial positioning of ALL microphones
4. Each mic appears at different virtual positions based on setup
```

### Real-Time Audio Effects Integration

**Add effects AFTER spatial processing:**

```
OBS Audio Chain:
VR Binaural Audio → Compressor → EQ → Noise Gate → Stream

Recommended Settings:
- Compressor: 3:1 ratio, -18dB threshold
- EQ: High-pass at 80Hz, gentle boost at 2-5kHz for voice clarity
- Noise Gate: -35dB threshold, 150ms hold
```

### Stream Interaction Features

**Viewer engagement with spatial audio:**

```batch
REM Enable chat-controlled positioning (advanced)
VRBinauralRecorder.exe --enable-chat-controls

Chat Commands (if enabled):
!moveclose - Move virtual microphone closer
!movefar - Move virtual microphone farther
!rotate - Slow rotation around listener
```

---

## 📊 Stream Quality Monitoring

### Real-Time Performance Metrics

**Monitor stream health during live sessions:**

```batch
REM Performance overlay in VR
VRBinauralRecorder.exe --enable-performance-overlay

Metrics to Watch:
- Frame Rate: >85 FPS (VR stability)
- Audio Latency: <50ms (real-time feeling)
- CPU Usage: <70% (headroom for encoding)
- Memory Usage: Stable (no memory leaks)
```

### Audio Quality Validation

**Ensure professional broadcast quality:**

```
OBS → View → Stats

Critical Audio Stats:
- Skipped frames due to encoding lag: 0%
- Dropped frames due to rendering lag: <0.1%
- Audio buffering: No warnings
- Bitrate: Stable at configured rate
```

---

## 🎯 ASMRtist Success Checklist

### Before Going Live

- [ ] VR system tracking smoothly (no jitter)
- [ ] Virtual audio device appearing in OBS
- [ ] Binaural test patterns working correctly
- [ ] 5-minute test recording successful
- [ ] Stream settings optimized for spatial audio
- [ ] Backup microphone configured (in case VR fails)

### During Stream

- [ ] Monitor OBS stats for dropped frames
- [ ] Check chat for audio quality feedback
- [ ] Watch VR performance overlay for issues
- [ ] Maintain head movement for spatial effect
- [ ] Keep backup recording running locally

### After Stream

- [ ] Review VOD for audio quality issues
- [ ] Check logs for any error messages
- [ ] Backup VR session data for later use
- [ ] Plan improvements for next session

---

## 💡 Pro Tips for ASMRtist Streamers

### Content Creation Optimization

**Maximize the spatial audio effect for viewers:**

1. **Choreographed Movement**: Plan head movements that showcase spatial positioning
2. **Multiple Trigger Positions**: Set up triggers at different VR locations
3. **Distance Play**: Move closer/farther from mic for intimacy effects
4. **360° Experiences**: Walk around microphone for full surround effect

### Technical Excellence

**Maintain professional quality throughout streams:**

1. **Consistent Positioning**: Use VR boundary system to maintain optimal distance
2. **Movement Smoothness**: Avoid jerky head movements (causes audio artifacts)
3. **Battery Management**: VR headsets need charging - plan for swaps
4. **Lighting Setup**: Good lighting for webcam even while wearing VR headset

---

## 🆘 Emergency Procedures

### Stream Failure Recovery

**If VR system fails during live stream:**

```batch
REM Quick fallback to desktop mode
VRBinauralRecorder.exe --desktop-fallback-mode

REM Announces to chat: "Switching to desktop mode, spatial audio continues"
REM Uses mouse movement to simulate head tracking
```

### Audio System Recovery

**If virtual audio device disappears:**

```batch
REM 1. Don't panic - viewers will notice immediately
REM 2. Quick restart (15 seconds)
taskkill /f /im VRBinauralRecorder.exe
timeout /t 3
VRBinauralRecorder.exe --streaming-mode --fast-init

REM 3. Verify OBS picks up audio source automatically
REM 4. Continue stream with minimal interruption
```

---

## 📈 Analytics & Improvement

### Stream Performance Analysis

**Post-stream review checklist:**

```batch
REM Generate session report
VRBinauralRecorder.exe --generate-session-report --session last

Report includes:
- Average tracking accuracy
- Audio dropout incidents
- Performance bottlenecks
- Spatial coverage map (where you moved)
- Quality metrics
```

### Viewer Feedback Integration

**Use viewer data to improve spatial experience:**

```
Common Feedback → Technical Adjustment:

"Can't tell it's spatial" → Increase head movement range
"Sounds too far away" → Adjust virtual microphone distance
"Audio cuts out" → Reduce VR tracking rate, increase buffers
"Amazing left ear effect!" → Note successful technique for future
```

---

## 🚀 Ready to Stream Spatial ASMR!

**You now have everything needed to create revolutionary spatial ASMR streams that put viewers directly into your recording space!**

**Key Success Factors**:
1. Always run pre-stream validation
2. Monitor performance during streams
3. Practice VR movement techniques
4. Engage with viewer feedback about spatial effects
5. Keep backup systems ready

**Happy Streaming!** 🎧🥽📺

---

## 📞 Streaming Support

### Priority Support for Content Creators

**Streaming-specific support channels:**
- Email: streaming-support@vr-binaural-recorder.com
- Discord: #asmr-streamers channel
- Response time: <2 hours during prime streaming hours (7-11 PM EST)

### Professional Creator Program

**For established ASMRtists with 1000+ followers:**
- Priority technical support
- Beta access to new features
- Custom spatial audio profiles
- Integration assistance with streaming software
- Revenue sharing opportunities

---

*Last Updated: 2025-09-27 | Guide Version: 1.0 | Supports OBS Studio 27.0+ | Windows 10/11 x64*