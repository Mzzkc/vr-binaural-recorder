# GOVERNANCE
Standards+compliance for VR Binaural (ASMRtist spatial audio).

## TECH
Audio: PortAudio+FFTW | VR: OpenVR+OpenXR | Build: CMake3.16+vcpkg
❌ custom drivers, homebrew FFT, direct HW access, non-RT libs

## ARCH
1comp=1concern | industry>custom | <10ms latency
≤50L/fn, ≤20methods/class, ≤10cyclo
Components: AudioEngine(route), HRTFProcessor(spatial), VRTracker(pose), ConfigManager(cfg)

## GATES
✅ Compile before merge | Tests required | <10ms verified
SA+VE approval | clang-tidy pass | No RT allocs

## SCOPE
CORE: ASMRtist mic→VR spatial
✅ audio.*routing|hrtf|spatial.*audio|vr.*tracking|real.*time
❌ record.*audio|process.*file|media.*player|offline.*process
Auto-detect→block violations

## ROLES
SA: arch+src+build+config | VE: src+tests+build | CC: src+UI+config
QA: tests+build+docs | PM: docs | GM: NO src (attrib only)
Pre-commit enforced. Violations=blocked.

## LIMITS
Fns: ≤50L, ≤5params, ≤10cyclo | Classes: ≤20methods, ≤3depth
Files: ≤1000L | Dup: <5% | Deps: ≤7coupling
Auto-monitor+alerts.

## SUCCESS
Tech: 100% compile, 0 crit violations, <10ms, 95% coverage
Process: 100% dialogue doc, 24h violation fix, 90% peer sat
Business: ASMRtist workflow validated, stakeholder >4.0/5.0

Enforcement: ALL comply. Non-compliance=escalation+access restriction.
