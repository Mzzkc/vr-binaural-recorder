# VR Binaural Recorder - Beta Testing Guide

**Welcome Beta Testers!**

Thank you for participating in the VR Binaural Recorder beta program. You're helping shape the future of spatial ASMR recording. This guide will walk you through setup, usage, and how to provide valuable feedback.

## What is VR Binaural Recorder?

VR Binaural Recorder transforms any microphone into a spatialized binaural audio source based on your VR headset position. Move your head naturally while wearing your VR headset, and the audio positioning updates in real-time to create immersive 3D audio experiences for your ASMR content.

**Key Features:**
- Real-time binaural spatial audio processing
- Natural VR head tracking controls audio positioning
- Visual Audio Cockpit interface in VR
- Professional-quality HRTF (Head-Related Transfer Function) processing
- OBS integration for live streaming
- Low-latency audio pipeline optimized for content creation

## System Requirements

### Minimum Requirements:
- **Operating System**: Windows 10 (version 1903+) or Windows 11 (64-bit)
- **Processor**: Intel Core i5-8400 or AMD Ryzen 5 2600 (6+ cores recommended)
- **Memory**: 8GB RAM minimum (16GB recommended)
- **Graphics**: NVIDIA GTX 1060 / AMD RX 580 or better (VR-capable GPU)
- **Storage**: 500MB free disk space
- **VR Headset**: Oculus Quest 2/3/Pro, HTC Vive, Valve Index, or Windows Mixed Reality
- **VR Runtime**: SteamVR or Oculus Runtime installed
- **Audio**: Any microphone (USB, XLR interface, or wireless)

### Recommended Setup:
- **Processor**: Intel Core i7-9700K or AMD Ryzen 7 3700X (8+ cores)
- **Memory**: 16GB+ RAM
- **Graphics**: NVIDIA RTX 2060 / AMD RX 5700 or better
- **Audio Interface**: ASIO-compatible interface for lowest latency (Focusrite Scarlett, PreSonus AudioBox, etc.)
- **USB**: USB 3.0+ ports for VR headset

## Installation Instructions

### Step 1: Pre-Installation Checks

Before installing, ensure your system is ready:

1. **Update Windows**: Install all Windows updates
2. **Update Graphics Drivers**: NVIDIA GeForce Experience or AMD Radeon Software
3. **Install VR Runtime**:
   - **Oculus**: Download Oculus Desktop from https://www.oculus.com/setup/
   - **SteamVR**: Install Steam, then install SteamVR from the Tools library
   - **WMR**: Install Windows Mixed Reality from the Microsoft Store

4. **Close Audio Applications**: Close Discord, OBS, Spotify, Chrome, and other apps using audio

### Step 2: Run the Installer

1. Download **VRBinauralRecorder-Windows-x64-Setup.exe**
2. Right-click the installer and select **Run as Administrator**
3. Follow the installation wizard:
   - Accept the license agreement
   - Choose installation location (default: `C:\Program Files\VR Binaural Recorder`)
   - Select components (all recommended for beta testing)
   - Complete installation

4. **Verify Installation**:
   - Desktop shortcut should appear: "VR Binaural Recorder"
   - Start Menu entry: Programs > VR Binaural Recorder
   - Installation folder should contain:
     - `VRBinauralRecorder.exe` (main executable)
     - `openvr_api.dll` (OpenVR library)
     - `hrtf_data/` folder (spatial audio data)
     - `vr_binaural_config.json` (configuration file)

### Step 3: First-Time Setup

After installation, run the diagnostic wizard:

1. Launch **VR Binaural Recorder** from the desktop shortcut
2. First launch will perform system checks:
   - VR runtime detection
   - Audio device enumeration
   - HRTF data validation
   - OpenVR compatibility check

3. **Expected Results**:
   - "VR System: Connected" (if SteamVR/Oculus is running)
   - "Audio Devices: Found [X] input devices"
   - "HRTF Data: Loaded successfully"
   - "System Ready for Recording"

## VR Headset Setup

### Oculus Quest 2/3/Pro Setup

1. **Enable Link/Air Link**:
   - Connect Quest to PC via USB-C cable (Link) or WiFi (Air Link)
   - Open Oculus Desktop app
   - Enable Oculus Link in the Quest headset

2. **Configure Audio**:
   - Oculus Desktop → Settings → Devices → Your Quest
   - Audio Output: "Oculus Virtual Audio Device"
   - Audio Input: "Oculus Virtual Audio Device"

3. **Enable Developer Mode** (for advanced audio routing):
   - Oculus app → Settings → General → Unknown Sources: ON

4. **Test Connection**:
   - Put on headset
   - Launch VR Binaural Recorder
   - Should see "Oculus Quest detected" in application log

**Common Issue**: If audio doesn't work, restart the Oculus VR Runtime Service:
- Press Win+R, type `services.msc`, press Enter
- Find "Oculus VR Runtime Service"
- Right-click → Restart

### HTC Vive / Valve Index Setup

1. **Launch SteamVR**:
   - Steam → Library → Tools → SteamVR
   - Ensure headset is detected and tracking

2. **Configure Audio**:
   - SteamVR → Settings → Audio
   - Playback Device: "Index Audio" (or "Vive Audio")
   - Recording Device: "Index Audio" (or "Vive Audio")
   - Mirror Audio: Select your recording interface

3. **Test Tracking**:
   - Put on headset
   - Verify room setup is complete
   - Controllers should be visible in SteamVR

**Common Issue**: If tracking is jittery, reduce refresh rate:
- SteamVR → Settings → Developer → Refresh Rate: 72Hz

### Windows Mixed Reality Setup

1. **Launch WMR Portal**:
   - Windows → Mixed Reality Portal
   - Complete initial room setup if first time

2. **Enable SteamVR Compatibility**:
   - WMR Portal → Settings → Startup and Desktop
   - Enable "Use SteamVR for apps"

3. **Test Headset**:
   - Put on headset
   - Should see WMR environment
   - Launch SteamVR from within WMR

**Common Issue**: If OpenVR fails, ensure WMR Portal is running BEFORE launching the app

## First Recording Session

### Pre-Recording Checklist

Before your first recording, validate the complete audio pipeline:

1. **Position Your Microphone**:
   - Place microphone 30cm in front of where you'll stand/sit
   - Mouth height (approximately 1.5m from floor)
   - Pointing toward your recording position

2. **Calibrate VR Space**:
   - Put on VR headset
   - Stand at your recording position
   - Launch VR Binaural Recorder
   - The Audio Cockpit overlay will appear in front of you

3. **Test Spatial Audio**:
   - Speak into the microphone
   - Turn your head left/right slowly
   - Audio should pan opposite to head movement:
     - Turn head LEFT → mic sounds to the RIGHT
     - Turn head RIGHT → mic sounds to the LEFT
     - This is CORRECT binaural behavior!

4. **Verify Audio Levels**:
   - Watch the Audio Cockpit meters in VR
   - Speak at normal recording volume
   - Levels should peak around -12dB to -6dB (green zone)
   - Adjust microphone gain if needed

### Basic Recording Workflow

1. **Put on VR Headset**
2. **Launch VR Binaural Recorder** (if not already running)
3. **See Audio Cockpit**:
   - Virtual microphone appears as a blue orb
   - Audio level meters show real-time input
   - Recording controls visible (Record/Stop buttons)

4. **Position Virtual Microphone** (optional):
   - Point VR controller at the blue microphone orb
   - Hold trigger to grab microphone
   - Move controller to reposition in 3D space
   - Release trigger to place

5. **Start Recording**:
   - Point controller at green "RECORD" button
   - Pull trigger to start
   - Button turns red and pulses
   - Controller vibrates to confirm
   - "REC" indicator appears in corner

6. **Create Your Content**:
   - Move your head naturally
   - Spatial audio follows head movement
   - Speak/whisper into microphone as normal
   - Watch audio meters to ensure good levels

7. **Stop Recording**:
   - Point controller at red "STOP" button
   - Pull trigger to stop
   - Controller vibrates to confirm
   - File saves automatically to `Documents/VR Binaural Recordings/`

### Understanding Spatial Audio

**Key Concept**: The microphone stays in 3D space, your HEAD moves around it.

- **Move head LEFT** → Mic position appears to shift RIGHT → Audio pans to RIGHT ear
- **Move head RIGHT** → Mic position appears to shift LEFT → Audio pans to LEFT ear
- **Move head FORWARD** → Mic gets closer → Audio gets louder
- **Move head BACKWARD** → Mic gets farther → Audio gets quieter

This creates natural binaural spatialization that listeners experience as 3D positioned audio.

## Audio Cockpit Features

The Audio Cockpit is your VR control interface:

### Display Elements:
- **Virtual Microphone Orb**: Blue sphere showing mic position in 3D space
- **Audio Level Meters**: Real-time left/right channel levels
- **Recording Status**: "REC" indicator when recording active
- **Controller Indicators**: Shows which controllers are detected
- **System Status**: VR tracking status, audio engine status

### Controls:
- **Record Button**: Green when stopped, red when recording
- **Stop Button**: Available only during recording
- **Microphone Positioning**: Grab and drag the blue orb
- **Auto-Hide**: UI fades after 3 seconds of inactivity (move controller to show)

## OBS Integration for Streaming

Want to stream your spatial ASMR sessions live? See the detailed **OBS Integration Guide** in `docs/OBS_INTEGRATION_GUIDE.md`.

**Quick Setup**:
1. Start VR Binaural Recorder BEFORE OBS
2. In OBS: Sources → + → Audio Input Capture (WASAPI)
3. Device: Select "VR Binaural Recorder (Virtual)"
4. Configure audio settings: 48kHz sample rate, 160kbps bitrate
5. Start streaming!

Your spatial audio will be captured and streamed to Twitch/YouTube in real-time.

## Known Issues and Limitations

### Beta Testing Phase Known Issues:

1. **Audio Routing Detection**:
   - Some systems may require restarting the application to detect audio devices
   - Workaround: Close all audio apps, restart VR Binaural Recorder

2. **VR Overlay Visibility**:
   - Audio Cockpit may not appear on first launch
   - Workaround: Press the VR headset menu button, then return to app

3. **Controller Tracking Loss**:
   - If controllers lose tracking, microphone positioning may stop working
   - Workaround: Move controllers back into tracking bounds

4. **Windows Audio Exclusive Mode Conflicts**:
   - Discord/OBS may lock audio devices
   - Workaround: Close other audio applications before launching

5. **Performance on Lower-End Systems**:
   - Systems with 6-core CPUs may experience occasional frame drops
   - Workaround: Close background applications, reduce VR supersampling

### Expected Behavior (Not Bugs):

- **Audio pans opposite to head movement** - This is CORRECT binaural behavior
- **Slight latency in headphones** - 10-20ms is normal for spatial processing
- **VR overlay doesn't follow head** - Overlay is fixed in world space by design
- **No visible output on monitor** - This is a VR-only interface (monitor shows status)

## Troubleshooting Guide

### Problem: "VR Runtime Not Detected"

**Solutions**:
1. Launch SteamVR or Oculus Desktop BEFORE the application
2. Restart the VR runtime service (see VR Headset Setup sections above)
3. Verify VR headset USB and DisplayPort connections
4. Update VR runtime to latest version

### Problem: "No Audio Devices Found"

**Solutions**:
1. Verify microphone is connected and powered on
2. Check Windows Sound Settings → ensure mic is enabled
3. Close Discord, OBS, Spotify, Chrome
4. Restart Windows Audio Service:
   - Open Services (Win+R → services.msc)
   - Find "Windows Audio"
   - Right-click → Restart

### Problem: "Audio Sounds Normal Stereo, Not Spatial"

**Diagnosis**: Spatial processing is bypassed

**Solutions**:
1. Verify HRTF data loaded (check application log)
2. Ensure VR tracking is active (should see "VR System: Connected")
3. Try turning head significantly (small movements = subtle panning)
4. Check configuration file: `vr_binaural_config.json` → `"audio": {"sampleRate": 48000}`
5. Restart application

### Problem: "Recording Files Are Empty or Silent"

**Solutions**:
1. Check microphone levels in Windows Sound Settings
2. Verify correct microphone selected in application
3. Check if another application has exclusive control of mic
4. Test microphone in another application first (Windows Voice Recorder)
5. Review application log file: `%APPDATA%\VRBinauralRecorder\logs\`

### Problem: "VR Tracking is Jittery or Laggy"

**Solutions**:
1. Close background applications (especially Chrome, Discord)
2. Reduce VR supersampling in SteamVR settings
3. Update graphics drivers
4. Check USB bandwidth (try different USB port for headset)
5. Verify CPU usage is below 80% (Task Manager)

### Problem: "Audio Cutting Out During Head Movement"

**Solutions**:
1. Increase audio buffer size in config: `"bufferSize": 256`
2. Set application priority to High:
   - Task Manager → Details → VRBinauralRecorder.exe → Right-click → Set Priority → High
3. Disable Windows power saving for USB:
   - Device Manager → USB controllers → Properties → Power Management → Uncheck "Allow computer to turn off device"
4. Close browser and streaming applications

## Providing Feedback

Your feedback is crucial for improving VR Binaural Recorder!

### What We Need to Know:

1. **System Information**:
   - Windows version (Win+R → winver)
   - VR headset model
   - CPU and GPU model
   - Audio interface (if using professional hardware)

2. **Session Details**:
   - What you were trying to do
   - What happened vs what you expected
   - How to reproduce the issue
   - Screenshots or video (if applicable)

3. **Application Logs**:
   - Location: `%APPDATA%\VRBinauralRecorder\logs\`
   - Include the most recent log file with bug reports

### How to Report Issues:

**Structured Bug Report Format**:
```
Title: [Brief description of issue]

System:
- Windows Version:
- VR Headset:
- CPU:
- GPU:
- RAM:

Steps to Reproduce:
1.
2.
3.

Expected Behavior:
[What should happen]

Actual Behavior:
[What actually happened]

Frequency:
[ ] Always  [ ] Sometimes  [ ] Once

Logs/Screenshots:
[Attach log file from %APPDATA%\VRBinauralRecorder\logs\]
```

**Send Reports To**:
- **Email**: asmr-support@vr-binaural-recorder.com
- **Subject Line**: "[BETA] Your Issue Description"
- **Priority Response**: Blocking issues (<24hr), General feedback (48-72hr)

### Feature Requests:

We love hearing your ideas! Format:
```
Feature Request: [Brief title]

Use Case:
[Why you need this feature]

Description:
[How it should work]

Priority:
[ ] Critical for my workflow
[ ] Would be nice to have
[ ] Future enhancement idea
```

### Positive Feedback:

Tell us what's working great! We want to know:
- Features you love
- Workflows that feel natural
- Quality of spatial audio
- Overall experience impressions

## Beta Testing Best Practices

### Getting the Best Results:

1. **Start Simple**:
   - First session: 30 seconds of speaking and head movement
   - Verify spatial audio works correctly
   - Then progress to longer recordings

2. **Test Different Scenarios**:
   - Standing vs sitting recording positions
   - Different microphone positions
   - Various head movement speeds
   - Different recording environments

3. **Document Your Setup**:
   - Take photos of your physical microphone placement
   - Note your typical recording workflow
   - Track what works well vs what doesn't

4. **Compare with Traditional Recording**:
   - Record the same content with traditional binaural mic
   - Compare quality and spatial effect
   - Provide feedback on differences

### Help Us Improve:

**We Especially Need Feedback On**:
- Audio quality compared to your traditional setup
- Ease of use and learning curve
- VR comfort during recording sessions
- Integration with your existing workflow
- OBS streaming integration
- Performance on different PC configurations
- Any features you expected but couldn't find

## Frequently Asked Questions

### Q: Do I need to own a binaural microphone?

**A**: No! VR Binaural Recorder works with ANY microphone. The application creates binaural spatialization using your VR headset position and HRTF processing. You can use a simple USB mic, XLR mic with interface, or even a wireless lavalier mic.

### Q: Can I use this without VR hardware?

**A**: Not in the beta. The core functionality requires VR head tracking to create spatial audio. A desktop fallback mode may be added in future versions.

### Q: What sample rate should I use?

**A**: The application is optimized for 48kHz (default). This matches the HRTF processing rate and provides the best quality. Using 44.1kHz may work but is not recommended.

### Q: How do I change the microphone position without using controllers?

**A**: Currently, controller interaction is the primary method. A future update may add keyboard/mouse positioning options.

### Q: Can I record multiple microphones simultaneously?

**A**: The beta supports single microphone input. Multi-mic support is planned for a future release.

### Q: Does this work with OBS streaming?

**A**: Yes! See the OBS Integration Guide for detailed setup instructions. The application creates a virtual audio device that OBS can capture.

### Q: What's the audio quality compared to a real binaural microphone?

**A**: It depends on the HRTF dataset and your microphone quality. The MIT KEMAR dataset provides professional-quality spatialization that many users find comparable to hardware binaural mics, but results vary by listener. We're eager for your feedback on this!

### Q: Can I adjust the distance between "ears"?

**A**: Not in the beta. The HRTF dataset uses standard head dimensions. Custom head models may be added in future versions.

### Q: How much storage do recordings use?

**A**: At 48kHz stereo, approximately 11MB per minute of recording (WAV format).

### Q: Can I use Bluetooth audio?

**A**: Not recommended due to latency. Use wired headphones for monitoring and wired microphones for input.

### Q: Does it support VR standalone mode (Quest native)?

**A**: No, this requires a PC and Link/Air Link connection. Quest standalone mode is not supported.

## Advanced Configuration

For advanced users who want to customize settings, edit:
`C:\Program Files\VR Binaural Recorder\vr_binaural_config.json`

**Basic Configuration Options**:
```json
{
  "audio": {
    "bufferSize": 128,     // Lower = less latency, higher = more stability
    "sampleRate": 48000    // Always use 48000 for best quality
  }
}
```

**After editing**, restart the application to apply changes.

**Warning**: Incorrect configuration may prevent the application from starting. Keep a backup of the original file.

## Getting Help

### Self-Service Resources:

1. **Windows VR Setup Guide**: `docs/WINDOWS_VR_SETUP_GUIDE.md`
2. **OBS Integration Guide**: `docs/OBS_INTEGRATION_GUIDE.md`
3. **Application Logs**: `%APPDATA%\VRBinauralRecorder\logs\`
4. **Configuration File**: `C:\Program Files\VR Binaural Recorder\vr_binaural_config.json`

### Before Contacting Support:

1. Check application logs for error messages
2. Review this guide's Troubleshooting section
3. Verify system requirements are met
4. Test with other VR applications (confirm VR system works)
5. Test microphone in Windows Voice Recorder (confirm mic works)

### Contact Support:

**Beta Tester Priority Support**:
- **Email**: asmr-support@vr-binaural-recorder.com
- **Subject**: "[BETA] Your Name - Issue Description"
- **Include**: System report, log files, detailed description
- **Response Time**: Blocking issues <24hr, General issues 48-72hr

**Generate System Report**:
See `docs/WINDOWS_VR_SETUP_GUIDE.md` for the system diagnostics script that generates a comprehensive report to include with support requests.

## Thank You!

Your participation in this beta is invaluable. You're helping create the future of spatial ASMR content creation!

**Beta Program Goals**:
- Validate core spatial audio functionality
- Identify workflow issues and usability improvements
- Test compatibility across different VR hardware
- Gather feedback on audio quality
- Ensure stability for production use

**Your Input Matters**: Every bug report, feature request, and piece of feedback helps make this tool better for the entire ASMR community.

Happy recording, and welcome to the future of spatial ASMR!

---

**Document Version**: 1.0 Beta
**Last Updated**: 2025-10-22
**Supports**: Windows 10/11 x64, VR Binaural Recorder Beta
**For Updates**: Check the installer package for the latest documentation
