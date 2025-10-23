# VR BINAURAL RECORDER - AI GUIDE
**C++17 VR spatial audio | CMake 3.16+ | OpenVR+PortAudio+ImGui+spdlog+jsoncpp**

## BUILD
```bash
mkdir -p build && cd build
cmake .. && cmake --build . --config Release
ctest --verbose  # tests
cmake --build . --target package  # installer
```

**Platform deps:**
- Win: ASIO SDK, NSIS
- Linux: X11, OpenGL dev
- macOS: System OpenGL

## ARCH
**Core:** audio_engine(PortAudio ASIO/WASAPI), hrtf_processor(binaural synth+convolution), vr_tracker(OpenVR pose+thread), overlay_ui(ImGui VR), config(JSON hot-reload)

**Deps:** PortAudio(full), OpenVR(v1.23.7 real), ImGui+GLFW(VR), spdlog(perf log), jsoncpp(config), HRTF(MIT KEMAR 368 WAV)

**Config:** vr_binaural_config.json(root) - audio(rate,buf,ASIO), hrtf(dataset,conv), vr(track,overlay), perf(thread,SIMD,mem), log(level,async)

**Tests:** tests/test_suite.cpp(Google Test auto-built)

**Valid:** ./check_status.sh | python3 tools/validator.py

**Debug:** config→"logging.level":"debug", "debug.profiling":true

## PERF
PROD-READY: AVX2 SIMD, real-time thread, lock-free rings, cache-align, <10ms latency target, adaptive buf, mem pools

## STATUS
SUBSTANTIAL: functional VR track, binaural proc, prod infra | PENDING: hardware valid

## FILES
hrtf_data/(368 WAV), third_party/(CMake), logs/(auto-rot), build/(ops), include/(7400+L impl), configs(root)

## DECISION PROTOCOL (RLF-Enhanced)

**Skill:** `/home/emzi/.claude/skills/rlf-decision-checkpoint.md`

### Project-Specific Triggers

**INVOKE RLF CHECKPOINT when:**

1. **Build/CMake Errors** (especially "file not found", "target not found")
   - Before: making targets optional, excluding subdirectories, disabling features
   - Questions: Who created the CMakeLists? Why reference this file? What breaks if excluded?

2. **Test Failures/Missing Tests**
   - Before: skipping tests, marking as optional, removing from build
   - Questions: Who wrote these tests? What do they validate? Why would they be missing?

3. **Missing Dependencies** (files exist locally but not in repo)
   - Before: assuming they're temporary, gitignoring, excluding from build
   - Check: `git ls-files`, `git check-ignore -v`, git history
   - Questions: Intentionally excluded or accidentally? Who added locally?

4. **"Quick Fix" Impulse** (workaround vs. solution)
   - Before: patching symptoms, adding conditionals, making things optional
   - Meta-check: Am I in single-domain (COMP-only) loop? What's my recognition level?

5. **Windows-Specific Issues**
   - Context: Windows dependency verification tests exist for a reason
   - Before: disabling Windows features, making platform-specific code optional
   - Questions: What Windows functionality is being validated? Why does this exist?

### Quick Checkpoint

```
[COMP] What's broken technically?
[SCI] What does git/filesystem show?
[CULT] Who created? Why exists? What breaks?
[EXP] Workaround or real fix? Confidence?
[META] Single-domain loop? Recognition P⁰→P³?
```

**REQUIRED:** ≥3 boundaries active (P>0.7) + recognition ≥ P³ before skip/exclude/disable decisions

### Historical Patterns (Learn From)

**Pattern 1:** "CMake references missing test files"
- ❌ Quick fix: Make subdirectory optional
- ✅ RLF: Check git (files gitignored), ask who/why (Mike Rodriguez Windows validation)
- Learning: Missing files in CMake → investigate git history and creator intent FIRST

**Pattern 2:** "Dependency not found"
- ❌ Quick fix: Disable dependency, make feature optional
- ✅ RLF: Check if intentionally excluded, download mechanism needed, platform-specific
- Learning: Large files (OpenVR 147MB) may be gitignored - need download logic, not skip

**Pattern 3:** "Test compilation errors"
- ❌ Quick fix: Comment out tests, skip test subdirectory
- ✅ RLF: What do tests validate? Who wrote them? Are dependencies available?
- Learning: Tests exist for deployment validation - fix dependencies, don't skip validation

### Core Principle

**Before you skip something, ask: Why does this exist? Who created it? What would break if it didn't exist?**

Multi-domain reasoning prevents single-perspective blindness. Boundary recognition reveals hidden context. Meta-awareness catches repetitive mistake patterns.

**Intelligence = oscillating_recognition_interfaces(domains, boundaries)**
