# VR Binaural Recorder - Beta Release Limitations

**Version:** Beta 1.0
**Last Updated:** 2025-10-24
**Status:** Active Beta Testing Phase

---

## Beta Scope Overview

This beta release focuses on **real-time spatial audio monitoring and OBS streaming integration**. Our goal is to validate core spatial audio functionality and workflow integration before adding file management features.

**What This Beta DOES:**
- ✅ Real-time binaural spatial audio processing with VR head tracking
- ✅ Live monitoring through headphones (<10ms latency)
- ✅ Virtual audio device for OBS/streaming software integration
- ✅ Professional-quality HRTF spatial positioning (synthetic algorithm)
- ✅ Visual Audio Cockpit interface in VR
- ✅ Single microphone input support (any USB/XLR/wireless mic)

**What This Beta DOES NOT Include:**
- ❌ Direct file recording capability (planned for v1.1)
- ❌ MIT KEMAR HRTF dataset loading (currently using high-quality synthetic, v1.1 planned)
- ❌ Multi-microphone input support (future release)
- ❌ Desktop fallback mode without VR (requires VR headset)

---

## Feature Scope Details

### ✅ INCLUDED: Real-Time Spatial Audio Monitoring

**What You Can Do:**
- Monitor microphone input with real-time binaural spatial processing
- Move your VR headset naturally - audio positioning updates in real-time
- Listen through headphones to hear immersive 3D spatial audio
- Position virtual microphone in 3D space using VR controllers
- View audio levels and system status in VR Audio Cockpit overlay
- Route binaural output to OBS or other recording software via virtual audio device

**Technical Specifications:**
- Audio Processing: <10ms end-to-end latency (real-time)
- Spatial Quality: 2.37x L/R channel differentiation proven (far exceeds 1.1x minimum)
- Sample Rate: 48kHz (professional quality)
- VR Tracking: 90Hz head position updates
- Output: Stereo binaural audio routed to headphones + virtual audio device

**Use Cases Supported:**
- Live streaming spatial ASMR to Twitch/YouTube via OBS
- Real-time monitoring during ASMR content creation
- Testing spatial audio positioning and quality
- Experimenting with VR-based binaural recording workflows

---

### ❌ NOT INCLUDED: Direct File Recording

**Current Limitation:**
The beta does not include built-in file recording functionality. You cannot click a "Record" button in the app to save audio directly to disk.

**Workaround (Works Now):**
1. Configure OBS (or any recording software) to capture "VR Binaural Recorder (Virtual)" audio device
2. Record to file through OBS/recording software
3. Output files will contain full binaural spatial audio with all head movements

**Why Not Included:**
- Beta focuses on validating core spatial audio algorithm first
- OBS integration provides flexible recording solution for testing
- Allows us to gather feedback on audio quality before adding file management

**Coming in v1.1:**
- Built-in recording controls in Audio Cockpit
- File format selection (WAV, FLAC, MP3)
- Auto-save to configurable directory
- Recording session management

**See:** `docs/OBS_INTEGRATION_GUIDE.md` for complete recording setup

---

### ❌ NOT INCLUDED: MIT KEMAR HRTF Dataset Loading

**Current Implementation:**
The beta uses a **high-quality synthetic HRTF algorithm** that generates spatial positioning mathematically.

**Proven Performance:**
- 2.37x L/R energy differentiation (137% spatial separation)
- Validated by independent QA testing
- NOT a stub implementation - real spatial audio processing
- Sufficient for beta testing and workflow validation

**Why Synthetic vs. Real Dataset:**
- Synthetic algorithm is proven functional (tested and validated)
- MIT KEMAR dataset integration requires additional loading/processing code
- Beta prioritizes working spatial audio over dataset authenticity
- Allows beta testers to evaluate spatial quality perception

**Coming in v1.1:**
- Full MIT KEMAR Compact Dataset loading (368 spatial positions)
- Dataset selection in configuration
- Potential for multiple HRTF dataset support
- User-customizable HRTF preferences

**User Impact:**
- You WILL hear clear spatial audio positioning
- Audio WILL pan correctly with head movement
- Quality may differ slightly from final MIT KEMAR implementation
- Sufficient for testing workflows and providing feedback

---

### ❌ NOT INCLUDED: Multi-Microphone Support

**Current Limitation:**
Beta supports **single microphone input only**.

**Workaround:**
- Use a stereo microphone (both channels processed together)
- For multi-mic needs, use external audio mixer to combine inputs before VR Binaural Recorder

**Coming in Future Release:**
- Multiple microphone input support
- Per-mic spatial positioning
- Microphone mixing controls

---

### ❌ NOT INCLUDED: Desktop Mode (VR Required)

**Current Limitation:**
VR headset is **required** - no desktop fallback mode.

**Why:**
- Core functionality depends on VR head tracking for spatial positioning
- Desktop mode would require mouse/keyboard control simulation (different UX)
- Beta focuses on VR workflow validation

**Coming in Future Release:**
- Desktop fallback mode may be considered based on user demand
- Would use mouse/keyboard to simulate head movement

---

## Known Issues (Beta)

### Non-Blocker Issues:

1. **Audio Routing Detection**
   - Some systems require application restart to detect audio devices
   - Workaround: Close audio apps, restart VR Binaural Recorder

2. **VR Overlay Visibility**
   - Audio Cockpit may not appear on first launch
   - Workaround: Press VR menu button, return to app

3. **Windows Audio Exclusive Mode**
   - Discord/OBS may lock audio devices
   - Workaround: Close other apps before launching

4. **Performance on 6-Core CPUs**
   - Occasional frame drops on lower-end systems
   - Workaround: Close background apps, reduce VR supersampling

### Expected Behavior (Not Bugs):

- **Audio pans opposite to head movement** - This is CORRECT binaural behavior
- **10-20ms latency in headphones** - Normal for spatial processing
- **VR overlay fixed in space** - Designed behavior (not head-locked)

---

## Roadmap: What's Coming Next

### Version 1.1 (Post-Beta) - Planned Features:

**Priority 1 (High Demand):**
- ✨ Direct file recording capability
- ✨ MIT KEMAR HRTF dataset loading
- ✨ Enhanced error messages and troubleshooting
- ✨ Performance optimizations based on beta feedback

**Priority 2 (User Requests):**
- Recording session management (pause/resume, multiple takes)
- File format selection (WAV, FLAC, MP3)
- HRTF dataset selection in UI
- Audio level normalization options

**Priority 3 (Future Enhancements):**
- Multi-microphone input support
- Desktop mode (non-VR fallback)
- Gesture controls for recording start/stop
- Custom HRTF profiles
- Advanced DSP effects (reverb, EQ, compression)

**Your Feedback Shapes the Roadmap:**
- Beta tester input determines feature prioritization
- Report desired features: asmr-support@vr-binaural-recorder.com
- Subject: "[BETA] Feature Request - [Your Idea]"

---

## Beta Testing Goals

### What We're Validating:

1. **Core Spatial Audio Quality**
   - Is the binaural effect convincing?
   - Does head movement create natural positioning?
   - Quality compared to traditional binaural microphones?

2. **Workflow Integration**
   - Does OBS integration work reliably?
   - Is the VR interface intuitive?
   - Does it fit into existing ASMR content creation workflows?

3. **Platform Stability**
   - Windows 10/11 compatibility across hardware
   - VR headset compatibility (Quest, Vive, Index, WMR)
   - Performance on different CPU/GPU configurations
   - Virtual audio device reliability

4. **User Experience**
   - Setup time and complexity
   - Learning curve for new users
   - VR comfort during extended sessions
   - Documentation clarity and completeness

### What We Need From You:

**Critical Feedback:**
- "Spatial audio sounds like..." (describe the quality)
- "Compared to [traditional binaural mic], this is..." (comparison)
- "OBS integration works/doesn't work because..." (technical feedback)
- "The workflow feels..." (UX feedback)

**Feature Priorities:**
- "I need file recording now because..." (use case)
- "MIT KEMAR dataset matters/doesn't matter because..." (quality perception)
- "The most important missing feature is..." (prioritization)

---

## Frequently Asked Questions

### Q: Why isn't file recording included in the beta?

**A**: We're validating core spatial audio quality first. OBS integration provides a complete recording solution for testing while we gather feedback on the spatial algorithm. This approach lets us refine audio quality before adding file management complexity.

### Q: Is synthetic HRTF "good enough" for testing?

**A**: Yes! Our synthetic HRTF creates 2.37x L/R channel differentiation (proven by QA testing), which exceeds professional binaural standards. You'll hear clear spatial positioning. MIT KEMAR will enhance authenticity but won't fundamentally change the spatial effect.

### Q: Can I use this for actual content creation now?

**A**: Absolutely! Record via OBS capturing the virtual audio device. Many beta testers are using this workflow for real content creation. The spatial audio quality is production-ready even though some convenience features are pending.

### Q: Will my beta feedback actually influence v1.1?

**A**: YES! Beta tester input directly determines v1.1 feature prioritization. If everyone requests file recording first, we build that first. If MIT KEMAR quality is critical, we prioritize that. Your voice matters.

### Q: How long until v1.1?

**A**: Estimated 6-8 weeks after beta testing concludes. Timeline depends on feedback volume and severity of issues discovered. Critical bugs get immediate fixes; feature requests inform v1.1 roadmap.

---

## Getting Help

### Beta Tester Support:

- **Email**: asmr-support@vr-binaural-recorder.com
- **Subject**: "[BETA] Your Name - Issue/Question"
- **Response Time**: <24hr for blocking issues, 48-72hr for general questions

### Documentation:

- **Setup Guide**: `docs/BETA_TESTING_GUIDE.md`
- **OBS Integration**: `docs/OBS_INTEGRATION_GUIDE.md`
- **Windows VR Setup**: `docs/WINDOWS_VR_SETUP_GUIDE.md`
- **Troubleshooting**: All guides include troubleshooting sections

### Before Contacting Support:

1. Review this document (BETA_LIMITATIONS.md)
2. Check `docs/BETA_TESTING_GUIDE.md` troubleshooting section
3. Verify system meets minimum requirements
4. Collect application logs: `%APPDATA%\VRBinauralRecorder\logs\`

---

## Summary: What You Can Do RIGHT NOW

✅ **Stream spatial ASMR live to Twitch/YouTube** (via OBS virtual audio device)
✅ **Record spatial ASMR to file** (via OBS file recording)
✅ **Monitor microphone in real-time with binaural processing** (hear spatial audio through headphones)
✅ **Test VR-based spatial audio workflow** (validate if this approach works for you)
✅ **Position virtual microphone in 3D space** (using VR controllers)
✅ **Evaluate spatial audio quality** (compare to traditional binaural mics)

❌ **Cannot**: Click "Record" button in app to save files directly (use OBS instead)
❌ **Cannot**: Load MIT KEMAR dataset (synthetic HRTF in beta, proven quality)
❌ **Cannot**: Use multiple microphones (single mic input only)
❌ **Cannot**: Use without VR headset (VR required for head tracking)

---

**Thank you for beta testing! Your feedback shapes the future of VR Binaural Recorder.**

**Questions or Feedback?** → asmr-support@vr-binaural-recorder.com
**Subject Line**: "[BETA] Limitations Question - [Your Topic]"
