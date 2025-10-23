# PERF-VALIDATION
Jordan Taylor - QA | Ensure RT perf for ASMRtist prod

## REQUIREMENTS
VR: 90Hz min (11.1ms frame) | Audio: <10ms pro/<20ms ok | CPU: <15% avg | Mem: <200MB stable | GPU: <5% overlay

ASMRtist: <5s startup, no crash 2+hr, no quality degradation, no mem leaks

## AUTO-TEST
performance_monitor.ps1: `.\performance_monitor.ps1 -Duration 1800 -OutputFile "perf.json"`
Monitors: VR frame consistency, audio buffer under/overrun, CPU/mem trends, GPU util, network

CI: Nightly regression, stress(4hr,multi-app,sys load,network), alert on >10% degradation

## MANUAL-TESTS
### T1:VR Track (90Hz req)
Launch --show-perf, rapid head move 2min, monitor frame time
PASS: 95% <11.1ms, no spike >20ms, stable 30min
FAIL: drops/judder, >15ms avg, degradation

### T2:Audio Latency (<10ms req)
Loopback audio interface, 1kHz tone, measure input→output w/ VR tracking
PASS: <10ms pro/<20ms ok, consistent regardless head move
Measure: RTL-SDR/analyzer, impulse response, account HRTF

### T3:CPU (<15% req)
Clean Win, Task Manager+ProcMon, 30min ASMRtist recording
PASS: avg<15%, peak<30%(brief), no sustained>20%
Analyze: thread CPU, bottlenecks, min-spec HW

### T4:Memory (<200MB+leak detect)
ProcMon+AppVerifier, 2+hr recording, sample every 5min
PASS: <200MB, no leaks(stable), cleanup after stop
FAIL: >300MB, trend>1MB/hr, no release

## STRESS
S1(4hr): continuous record, head move/30s, no dropouts/glitches, VR accurate, mem stable, CPU constant
S2(multi-VR): SteamVR+game+VRBinaural, no frame drops, audio unaffected, overlay OK
S3(sys stress): Prime95+80%RAM, graceful degrade, warnings, audio quality maintained
S4(streaming): Twitch/YouTube 1080p60@6000kbps, no VR impact, latency OK, stream quality OK

## PLATFORM
Win10vs11: OS perf diff, WASAPI validation, WMR integration
HW: min-spec, high-end, Intel vs AMD
VR headsets: Quest2(Link/AirLink), Vive(base station), WMR(MS runtime), Index(120/144Hz)
Consistent perf all, no headset regressions, optimal configs documented

## ALERTS
CPU: warn>20%sustain, crit>30%@30s | Mem: warn>250MB, crit>500MB/leak | VR: warn>13ms@10%, crit>20ms/loss
Regression: baseline DB, auto-compare builds, alert significant regression, trend analysis

## TROUBLESHOOT
High CPU: inefficient HRTF/VR overhead → optimize buffer/reduce VR freq
VR drops: GPU overlay/CPU sched → reduce overlay/adjust priority
Audio latency: large buffers/inefficient pipe → reduce buffer/optimize HRTF
Mem leaks: unreleased VR/audio → cleanup/RAII patterns

## SUCCESS
Tech: 90Hz VR, <10ms audio pro, <15% CPU, <200MB mem stable
UX: No perceptible issues, responsive pro-quality, long sessions OK, scales HW
Prod: Min-spec OK, auto-monitor prevents regression, clear guidelines, optimization documented
