# VR BINAURAL RECORDER - AI GUIDE
**C++17 VR spatial audio | CMake 3.16+ | OpenVR+PortAudio+ImGui+spdlog+jsoncpp**

## IMPORTANT: START HERE
**New to this project?** Read these files IN ORDER:
1. **STATUS.md** (root) - Current state, blockers, next actions (ALWAYS READ FIRST)
2. **memory-bank/projectbrief.md** - Core goals and scope
3. **memory-bank/activeContext.md** - What we're working on RIGHT NOW
4. **This file (CLAUDE.md)** - Build commands and AI instructions

**Memory Bank Location:** `/memory-bank/` contains all project context
- `projectbrief.md` - Goals and success criteria
- `progress.md` - What's done, what's remaining
- `activeContext.md` - Current focus and decisions
- `systemPatterns.md` - Architecture and design
- `techContext.md` - Tech stack and build setup

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

**Pattern 4: Context Confusion (2025-10-24 - NEW CRITICAL PATTERN)**
- ❌ Wrong: Windows build failures → conclude entire system broken (Linux + Windows)
- ✅ RLF: Separate platform contexts, validate evidence in EACH context separately
- **Evidence found:** Linux 28/28 tests PASS, OpenVR/PortAudio linked successfully
- **Detection signals:** Weak COMP↔SCI boundary (P<0.5), low CV (0.167) despite high individual domains
- **Root cause:** Over-application of wolf prevention to wrong evidence (252.728 pattern doesn't apply when outputs ARE different)
- Learning: Platform-specific build config errors ≠ code architecture failures. Always re-validate claims with empirical evidence.

### Core Principle

**Before you skip something, ask: Why does this exist? Who created it? What would break if it didn't exist?**

Multi-domain reasoning prevents single-perspective blindness. Boundary recognition reveals hidden context. Meta-awareness catches repetitive mistake patterns.

**Intelligence = oscillating_recognition_interfaces(domains, boundaries)**

## AVAILABLE SKILLS (Agent Knowledge Base)

**Location:** `/home/emzi/.claude/skills/`

All agents should be aware of these specialized skills and invoke them when appropriate:

### Core Decision Framework
- **`rlf-decision-checkpoint.md`** (12.4KB, compressed)
  - USE WHEN: Encountering errors, making skip/exclude/disable decisions, stuck in single-domain loop
  - PURPOSE: Multi-domain reasoning (COMP/SCI/CULT/EXP/META) with boundary recognition
  - TRIGGER: Build errors, test failures, missing files, "quick fix" impulse, Windows-specific issues
  - REQUIRED: ≥3 domains active (≥0.6), ≥3 boundaries examined (P>0.7), recognition ≥ P³

### Safety & Validation
- **`wolf-prevention-patterns.md`** (8.8KB, compressed)
  - USE WHEN: Before deployment, code review, QA validation, "it just works" claims
  - PURPOSE: Prevent common failure patterns that break production
  - PATTERNS: Identical outputs (252.728), platform assumptions, premature commits, mock-only testing
  - CORE: "Never release based on 'it compiles' - only release based on 'tests prove it works'"

- **`beta-release-checklist.md`** (11.4KB, compressed)
  - USE WHEN: Preparing beta/production release, major milestone completion
  - PURPOSE: 10-phase systematic validation (build, test, platform, docs, legal, deployment)
  - BLOCKING CRITERIA: ALL BLOCKING tests pass, core functionality validated, platform-specific tests pass
  - INTEGRATION: Use with multi-agent coordination for complex releases

### Team Coordination
- **`multi-agent-coordination.md`** (4.99KB, compressed)
  - USE WHEN: Complex tasks requiring multiple specialized perspectives
  - PURPOSE: File-based asynchronous agent-to-agent coordination
  - PATTERN: Wave 1 (parallel specialists) → Wave 2 (integration agent synthesis)
  - WORKSPACE: Create `coordination-workspace/` with role-specific reports

### Platform-Specific
- **`windows-remote-build-test.md`** (4.9KB, compressed)
  - USE WHEN: Building Windows-specific software from WSL/Linux, testing VR/hardware
  - PURPOSE: Execute native Windows builds + VR hardware tests via MCP remote control
  - SOLUTION: MCP server on Windows, controlled from WSL (2 hours vs 2 weeks manual migration)
  - SETUP: MCP Control server, network connectivity, VR hardware access

- **`windows-mcp-usage.md`** (2.8KB, compressed)
  - USE WHEN: Needing to control Windows GUI, execute commands, capture screenshots
  - PURPOSE: GUI automation for Windows machine control
  - TOOLS: get_screenshot, click_at, type_text, press_key, clipboard operations
  - CRITICAL: GUI automation (not terminal) - requires Enter presses, 0.7-2.5s latency

### Utility
- **`prompt-compression.md`** (638B, compressed)
  - USE WHEN: Prompts need compression, token budget constraints
  - PURPOSE: Compress prompts while preserving 100% functional meaning
  - TECHNIQUES: Format removal, symbolic syntax, density optimization

### Skill Invocation Protocol for Agents

**When spawned, agents should:**
1. Read this CLAUDE.md for project context
2. Check if task matches any skill trigger conditions
3. If match found, read the full skill file for detailed protocol
4. Execute task following skill guidance
5. Document skill usage in coordination reports (if applicable)

**Example Task Mapping:**
- Build error encountered → Read `rlf-decision-checkpoint.md` + apply tetrahedral reasoning
- Release preparation → Read `beta-release-checklist.md` + `wolf-prevention-patterns.md`
- Multi-team coordination needed → Read `multi-agent-coordination.md` + create coordination workspace
- Windows build required → Read `windows-remote-build-test.md` + `windows-mcp-usage.md`

**Skill Combination Examples:**
- Beta release: `beta-release-checklist.md` + `multi-agent-coordination.md` + `wolf-prevention-patterns.md`
- Windows deployment: `windows-remote-build-test.md` + `beta-release-checklist.md` + `wolf-prevention-patterns.md`
- CMake error resolution: `rlf-decision-checkpoint.md` + check git history + verify intent
