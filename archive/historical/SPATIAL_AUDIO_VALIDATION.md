# SPATIAL-VALIDATION
Jordan Taylor - QA | Verify TRUE binaural (not mono/stereo pan)

## PROBLEM
Many VR audio apps claim "spatial" but output mono/stereo pan. ASMRtists need TRUE binaural: head move→3D positioning via HRTF.

## QUICK-TEST (2min)
1. VR+headset, speak+turn head L/R
   PASS: voice "stays in place" | FAIL: voice follows head (stereo pan)
2. Win Sound→VRBinaural props→Levels→Balance, speak+move
   PASS: L/R meters differ/change | FAIL: identical (mono)

## COMPREHENSIVE
### P1:Binaural Sweep
`--test-mode`→"Binaural Sweep Test", VR+headphones
EXPECT: tone orbits 3D, outside head (externalized), no in-head
Record OBS 30s, analyze Audacity/Reaper
PASS: L/R phase diff+amplitude var | FAIL: identical waveforms

### P2:Head Move Track
Record: center"center", 90°L"left", 90°R"right", 180°"behind", center"back"
Playback stereo headphones:
EXPECT: center(balanced L/R), left(stronger R ear), right(stronger L), behind(muffled/filtered HRTF), back(balanced)
FAIL: all identical(no spatial), L/R pan(stereo not binaural), no behind filter(missing HRTF)

### P3:Distance Sim
Face mic normal dist, record"close", move far, record"distant"
EXPECT: distance→volume+timbre, advanced:reverb increases
ADVANCED: room reverb sim

## OBS-INTEGRATION
Visual: Audio Input Capture→VRBinaural, Monitor+Output, Filters→Compressor
TEST: speak+move VR, monitor meters | PASS:L/R differ | FAIL:identical
Record 5min WAV, export, Audacity visual
PASS: L/R waveforms differ, freq spectrum binaural | FAIL: mono/identical
Stream: YouTube/Twitch, viewers confirm spatial, analysis tools verify stereo

## TROUBLESHOOT
### Mono (L/R identical)
CAUSE: HRTF not init, missing data, mono audio device
FIX: check /hrtf/ files, restart, verify "2ch 16bit 48kHz"

### Stereo Pan (not binaural)
CAUSE: HRTF disabled, VR not connected audio, fallback stereo
FIX: enable VR track settings, check headset, restart --force-vr

### Inside Head (no external)
CAUSE: wrong HRTF dataset, poor headphones, high latency
FIX: try diff HRTF profile, quality closed-back phones, reduce buffer

### No Behind
CAUSE: incomplete HRTF, missing head shadow, VR yaw-only
FIX: verify full HRTF install, enable "Advanced HRTF", check 6DOF

## ADVANCED
Freq Response: pink noise→spectrum(REW/SMAART), diff freq by pos, head shadow reduces highs behind
Phase Coherence: 1kHz sine, 360° rotation, analyze
MEASURE: phase diff L/R varies smooth | BENCH: center≈0°, 90°L(R lead), 180°(complex HRTF)
Latency: loopback interface, test signal→monitor output
MEASURE: <10ms pro/<20ms ok

## ASMR-SPECIFIC
Whisper: quiet whisper, slow head move
VALIDATE: details preserved spatial | FAIL: noise reduction destroys cues
Multi-Source: multi mics, diff VR locs, move between
EXPECT: each mic distinct spatial
Long-Session: 2hr record, periodic head move
VALIDATE: quality maintained | FAIL: drift, mem leaks→degradation, thermal throttle

## CHECKLIST
Quick(5min): [ ]L/R differ [ ]head→positioning [ ]voice stays [ ]OBS L/R differ
Comprehensive(30min): [ ]binaural sweep [ ]4 cardinal dirs differ [ ]behind filtered [ ]distance timbre [ ]no dropouts [ ]latency<20ms
Pro(2hr): [ ]freq varies by pos [ ]phase correct [ ]long stable [ ]multi HRTF [ ]stream integrated [ ]viewer confirms

## SUCCESS
ASMRtist: viewers ID mic pos audio-only, spatial>mono/stereo immersion, pro quality
Tech: L/R measurably differ, <10ms latency, HRTF maintains fidelity+spatial, stable multi-hr
Integration: OBS/stream proper stereo binaural, viewers experience positioning, ASMRtists focus content
