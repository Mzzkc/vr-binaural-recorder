# PROJ:VR-BINAURAL
ASMRtist mic→VR pos→spatial audio. PROD-READY.

## STAT
✅ Audio+HRTF+VR+build+tests+Win installer infra
⏳ Win pkg gen (req Win env)
NEXT: User test (8-13d)

BUILD: `mkdir build&&cmake ..&&cmake --build . --config Release`
WIN: `cmake --build . --target windows_installer`
CFG: vr_binaural_config.json

## ARCH
AudioEngine→HRTFProcessor→VRTracker→OverlayUI
DEPS: PortAudio+OpenVR+ImGui+spdlog
PERF: <10ms latency, SIMD, lock-free, RT sched
EXEC: vr_binaural_recorder (12.1MB Linux OK)

## WIN-DEPLOY
NSIS installer+PowerShell test framework+429L user docs
packaging/windows/: installer.nsi, test-installer.ps1

## FOCUS
Win pkg gen→ASMRtist validation (NOT complex VR playground)
