# Project Brief: VR Binaural Recorder

**Created:** 2025-10-24
**Status:** Beta Development
**License:** MIT

---

## Core Mission

Transform any standard microphone into an immersive binaural spatial audio recording tool using VR head tracking.

**Target Users:** ASMRtists and content creators on YouTube/Twitch

**Key Innovation:** Natural head movement controls virtual microphone positioning - no complex controls needed

---

## Problem Statement

### Current ASMR Recording Challenges
1. **Expensive Equipment:** Traditional binaural recording requires dummy head microphones ($500-$5000)
2. **Complex Setup:** Multiple microphones, precise positioning, post-processing
3. **Limited Flexibility:** Fixed microphone positions, can't adjust during recording
4. **Barrier to Entry:** Most creators can't afford professional spatial audio equipment

### Our Solution
- Use VR headset for natural head tracking (equipment creators already own)
- Any standard microphone becomes binaural source (USB, XLR, wireless)
- Real-time HRTF processing creates spatial audio (<10ms latency)
- Intuitive VR overlay interface - no technical knowledge required

---

## Success Criteria

### Technical Requirements (MUST HAVE)
- ✅ Real-time audio processing with <10ms latency
- ✅ Clear spatial differentiation (L/R ratio >1.1x minimum)
- ⏳ Windows 10/11 compatibility with major VR headsets
- ✅ Professional audio quality using MIT KEMAR HRTF dataset
- ⏳ Stable operation for 30+ minute recording sessions

### User Experience (MUST HAVE)
- ⏳ Setup time <5 minutes for new users
- ⏳ Intuitive VR overlay interface (Audio Cockpit)
- ⏳ Works with Quest 2/3, Vive, Index, Windows Mixed Reality
- ⏳ OBS integration for live streaming

### Quality Markers (SHOULD HAVE)
- Multiple HRTF datasets for personalization
- Audio recording to file (not just real-time monitoring)
- Gesture controls for recording start/stop
- Audio level meters and visual feedback

---

## Scope

### In Scope (Beta Release)
- Real-time binaural audio monitoring
- VR head tracking integration (OpenVR)
- Audio Cockpit overlay interface
- Windows installer with user documentation
- OBS virtual audio cable integration
- Basic configuration via JSON file

### Out of Scope (Future Versions)
- Multi-microphone support
- Audio file recording (currently monitoring only)
- macOS/Linux builds
- Mobile VR support (Quest standalone)
- Advanced DSP effects (reverb, EQ, compression)
- Cloud storage integration

### Explicitly NOT in Scope
- Professional DAW integration
- Video recording/synchronization
- Network streaming (beyond OBS)
- Commercial licensing/DRM

---

## Target Timeline

### Beta Release (Current Goal)
- **Status:** Implementation complete, validation in progress
- **Remaining:** Windows testing, ASMRtist validation
- **Timeline:** 8-13 days from beta approval

### Version 1.0 (Future)
- Post-beta feedback integration
- File recording capability
- Enhanced HRTF selection
- Performance optimizations

---

## Technical Architecture (High Level)

```
VR Headset (OpenVR) → VR Tracker → Pose Data
                                      ↓
User Microphone → Audio Engine → HRTF Processor → Binaural Output
                                      ↑
                           MIT KEMAR Dataset

Overlay UI (ImGui + GLFW) → Visual Feedback in VR
```

**Core Components:**
- Audio Engine (PortAudio - ASIO/WASAPI)
- HRTF Processor (Real-time convolution, SIMD optimized)
- VR Tracker (OpenVR integration, 90Hz tracking)
- Overlay UI (ImGui-based Audio Cockpit)
- Config System (JSON hot-reload)

---

## Key Constraints

### Performance
- Must maintain <10ms audio latency (professional requirement)
- CPU usage <25% on recommended hardware
- Memory footprint <500MB
- No audio dropouts during head movement

### Platform
- Windows-first approach (90% of target users)
- VR headset required (cannot work without tracking)
- Requires OpenVR/SteamVR or Oculus Runtime

### User Experience
- Zero audio production knowledge assumed
- Must be simpler than traditional binaural recording
- Natural interaction (head movement, not keyboard/mouse)

---

## Stakeholders

### Primary Users
- **ASMRtists:** Content creators producing spatial audio
- **Technical Level:** Beginner to intermediate
- **Budget:** Consumer-grade equipment ($0-500 microphone budget)

### Secondary Users
- **Streamers:** Live spatial audio on Twitch/YouTube
- **Audio Enthusiasts:** Experimenting with binaural recording
- **Developers:** Building on open-source foundation

---

## Risks & Mitigation

### Technical Risks
| Risk | Impact | Mitigation | Status |
|------|--------|------------|--------|
| VR tracking latency | High | Optimized 90Hz polling, prediction | ✅ Mitigated |
| Audio quality issues | High | MIT KEMAR HRTF dataset | ✅ Mitigated |
| Windows compatibility | Medium | Extensive testing, MCP validation | ⏳ In Progress |
| Driver conflicts | Medium | Documentation, troubleshooting guide | ✅ Mitigated |

### User Adoption Risks
| Risk | Impact | Mitigation | Status |
|------|--------|------------|--------|
| Too complex to setup | High | Comprehensive guides, <5min setup goal | ⏳ Testing |
| VR headset requirement | Medium | Clear communication, target users already have VR | ✅ Accepted |
| Audio quality expectations | Medium | Demo videos, realistic marketing | ⏳ Pending |

---

## Success Metrics

### Beta Success (Deployment Criteria)
- ✅ All BLOCKING tests pass on Windows
- ⏳ 3+ ASMRtist beta testers complete full workflow
- ⏳ Zero critical bugs in 30-minute recording sessions
- ⏳ Setup time measured <8 minutes for new users

### Version 1.0 Success (Future)
- 100+ active users within 3 months
- 10+ YouTube videos using the tool
- <5% reported critical bugs
- 80%+ positive user feedback

---

## References

- **README.md** - User-facing documentation
- **CLAUDE.md** - AI development instructions
- **memory-bank/systemPatterns.md** - Detailed architecture
- **memory-bank/techContext.md** - Technology stack details
