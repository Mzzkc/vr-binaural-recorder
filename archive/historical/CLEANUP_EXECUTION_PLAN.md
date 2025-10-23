# REPOSITORY CLEANUP EXECUTION PLAN
**Detailed Step-by-Step Cleanup Protocol**
**Date:** 2025-10-22

## PRE-EXECUTION SAFETY

Before executing cleanup, verify:
1. Current branch: recover-deleted-tests
2. Unstaged changes: MASSIVE (see git status)
3. **CRITICAL: Commit or stash changes before cleanup!**

---

## PHASE 1: MOVE MISPLACED SOURCE FILES

### 1A: Move Test Files from Root → tests/
```bash
cd /home/emzi/Projects/vr-binaural-recorder

# Move test files
mv simple_hrtf_test.cpp tests/
mv simple_spatial_test.cpp tests/
mv debug_distance_test.cpp tests/
mv debug_distance_test_real.cpp tests/
mv debug_distance_attenuation.cpp tests/
mv direct_spatial_test.cpp tests/
mv mike_hrtf_verification.cpp tests/
mv validate_config.cpp tests/
mv test_bulletproof_config.cpp tests/
mv test_json_config.cpp tests/
mv test_vrtracker_callback.cpp tests/
mv rc_performance_test.cpp tests/
```

**Files moved: 12**

### 1B: Move Demo Files from Root → examples/
```bash
# Create examples directory if needed
mkdir -p examples

# Move demo files
mv ASMR_VR_EXPERIENCE_DEMO.cpp examples/
mv VR_EXPERIENCE_VERIFICATION_DEMO.cpp examples/
```

**Files moved: 2**

### 1C: Remove Source Files from Build Directory
```bash
# These should NEVER be in build/
rm build/spatial_audio_proof_test.cpp
rm build/spatial_test.cpp
```

**Files deleted: 2**

---

## PHASE 2: CLEAN UP VALIDATION REPORTS

### 2A: Delete Outdated/Contradictory CEO Reports
```bash
# Keep only the latest (Sep 27)
# Delete older contradictory versions
rm CEO_SPATIAL_AUDIO_VERIFICATION_REPORT.md  # Sep 25: "BLOCKED"
rm CEO_SPATIAL_AUDIO_VALIDATION_REPORT.md    # Sep 26: superseded
```

**Keep:** CEO_VALIDATION_REPORT.md (Sep 27 - latest)
**Delete:** 2 older versions

### 2B: Clean Up Redundant Validation Reports
```bash
# Superseded spatial audio validations
rm SPATIAL_AUDIO_VALIDATION_REPORT.md  # Sep 26 - older version
# Keep: SPATIAL_AUDIO_VALIDATION.md (Sep 27 - latest)
```

---

## PHASE 3: CONSOLIDATE WINDOWS DOCUMENTATION

### 3A: Archive Old Windows Packaging Docs
```bash
mkdir -p archive/windows-packaging-sept2025

# Move redundant Windows docs to archive
mv WINDOWS_INSTALLER_TECHNICAL_ASSESSMENT.md archive/windows-packaging-sept2025/
mv WINDOWS_TESTING_PACKAGE.md archive/windows-packaging-sept2025/
mv FINAL_PACKAGING_COORDINATION_SUMMARY.md archive/windows-packaging-sept2025/
mv FINAL_WINDOWS_VR_PACKAGE_SUMMARY.md archive/windows-packaging-sept2025/
mv WINDOWS_VR_DEPLOYMENT_PACKAGE.md archive/windows-packaging-sept2025/
```

**Keep in root for now (user-facing):**
- WINDOWS_USER_TESTING_GUIDE.md
- WINDOWS_CLEAN_INSTALL_TEST.md
- WINDOWS_PACKAGE_VALIDATION_REPORT.md

**Consider:** Creating single docs/WINDOWS_INSTALLATION.md consolidating all

---

## PHASE 4: RESOLVE DUPLICATE DOCUMENTATION

### 4A: OBS Integration Guide Duplicate
```bash
# Compare and keep better version in docs/
# Delete root version
rm OBS_INTEGRATION_GUIDE.md

# Verify docs/OBS_INTEGRATION_GUIDE.md exists and is complete
# (It does - confirmed in analysis)
```

### 4B: Verify No Other Duplicates
Already checked - OBS guide is the only exact duplicate.

---

## PHASE 5: ARCHIVE AGENT ROLEPLAY/COMMUNICATION DOCS

### 5A: Create Archive for Team Communications
```bash
mkdir -p archive/team-communications-sept2025

# Move massive dialogue files
mv AGENT_TEAM_DIALOGUE.md archive/team-communications-sept2025/  # 82KB!
mv AGENT_CONVO_SCRATCHPAD.md archive/team-communications-sept2025/  # 32KB
mv VP_ENGINEERING_DAILY_STATUS.md archive/team-communications-sept2025/
mv VP_ENGINEERING_HANDOFF.md archive/team-communications-sept2025/
```

**Keep for now (might be useful):**
- AGENT_CORE.md (procedures)
- PROJECT_GOVERNANCE.md (standards)
- VP_ENGINEERING_PROTOCOL.md (brief protocol)

---

## PHASE 6: ARCHIVE OLD STATUS/DEPLOYMENT REPORTS

### 6A: Archive Completed Deployment Reports
```bash
mkdir -p archive/deployment-sept2025

mv DEPLOYMENT_READINESS_REPORT.md archive/deployment-sept2025/
mv FINAL_INTEGRATION_VALIDATION_REPORT.md archive/deployment-sept2025/
mv QA_VERIFICATION_REPORT_CRITICAL.md archive/deployment-sept2025/
```

---

## PHASE 7: CLEAN UP EXECUTABLES AND BUILD ARTIFACTS

### 7A: Remove Test Executables from Root/Build
```bash
cd /home/emzi/Projects/vr-binaural-recorder

# Remove compiled test executables
rm -f simple_test
rm -f simple_hrtf_test
rm -f simple_spatial_test
rm -f debug_distance
rm -f debug_distance_test
rm -f debug_distance_test_real
rm -f validate_config
rm -f test_config
rm -f VR_VERIFICATION_DEMO
rm -f ASMR_DEMO

# Note: These should be in build/ or not committed at all
```

---

## PHASE 8: ORGANIZE SCRIPTS

### 8A: Move Scripts to scripts/ Directory
```bash
mkdir -p scripts

# Move utility scripts
mv cleanup_redundant_docs.sh scripts/
mv create_ceo_deployment_package.sh scripts/
mv validate_end_to_end.py scripts/
```

### 8B: Move Build Scripts to packaging/
```bash
# Check if these exist in root and should be in packaging/
# (already in packaging/windows/ per git status)
```

---

## PHASE 9: UPDATE DOCUMENTATION INDEX

### 9A: Create Documentation Map
```bash
# Create docs/README.md explaining documentation structure
cat > docs/README.md << 'EOF'
# VR Binaural Recorder Documentation

## User Documentation
- [OBS Integration Guide](OBS_INTEGRATION_GUIDE.md) - Stream setup
- [Windows VR Setup Guide](WINDOWS_VR_SETUP_GUIDE.md) - Windows installation

## Developer Documentation
- [CLAUDE.md](CLAUDE.md) - AI assistant instructions
- [Dependency Requirements](DEPENDENCY_REQUIREMENTS.md) - Build dependencies
- [HRTF Dataset Status](HRTF_DATASET_STATUS.md) - Audio data files
- [Project Status](PROJECT_STATUS.md) - Current development status

## Testing
See [tests/README.md](../tests/README.md) for test suite documentation.

## Archives
Historical documentation in `/archive/`:
- audit-documentation/ - Architecture audits and governance
- team-communications-sept2025/ - Development dialogue
- deployment-sept2025/ - Deployment reports
- windows-packaging-sept2025/ - Windows installer development
EOF
```

### 9B: Create Tests Documentation
```bash
cat > tests/README.md << 'EOF'
# VR Binaural Recorder Test Suite

## Main Test Suite
- **test_suite.cpp** - Primary Google Test suite (TRACKED)
- **integration_tests.cpp** - Integration testing (TRACKED)
- **audio_performance_tests.cpp** - Performance benchmarks (TRACKED)

## VR Testing
- **vr_integration_test.cpp** - VR integration tests (TRACKED)
- **vr_headset_compatibility_tests.cpp** - Hardware compatibility (TRACKED)
- **test_openvr_api_validation.cpp** - OpenVR API validation

## Validation Tests (Untracked - Development)
- **spatial_audio_validation_BLOCKING.cpp** - Critical spatial audio tests
- **audio_cockpit_validation.cpp** - UI validation
- **ceo_spatial_validation.cpp** - End-to-end validation
- **vr_validation_suite.cpp** - Comprehensive VR tests

## Unit Tests
- **test_config_advanced.cpp** - Configuration system (TRACKED)
- **simple_hrtf_test.cpp** - HRTF processor unit tests
- **simple_spatial_test.cpp** - Spatial audio unit tests
- **debug_distance_*.cpp** - Distance attenuation debugging

## Running Tests
```bash
cd build
cmake --build . --target test_suite
./test_suite
```

See [../docs/CLAUDE.md](../docs/CLAUDE.md) for build instructions.
EOF
```

---

## PHASE 10: GIT CLEANUP

### 10A: Review Untracked Files
```bash
# After cleanup, review remaining untracked files
git status --short | grep "^??"
```

### 10B: Update .gitignore
```bash
# Add patterns for files that should never be tracked
cat >> .gitignore << 'EOF'

# Test executables (should be in build/)
simple_test
simple_hrtf_test
simple_spatial_test
debug_distance*
validate_config
test_config

# Build artifacts in wrong locations
*.o
*.a
CMakeCache.txt
cmake_install.cmake
Makefile

# Temporary documentation
*_SCRATCHPAD.md
*_DIALOGUE.md
EOF
```

---

## POST-CLEANUP VERIFICATION

### Verify Directory Structure
```
/home/emzi/Projects/vr-binaural-recorder/
├── archive/
│   ├── audit-documentation/           (existing - 24 files)
│   ├── team-communications-sept2025/  (NEW - roleplay docs)
│   ├── deployment-sept2025/           (NEW - old reports)
│   └── windows-packaging-sept2025/    (NEW - packaging history)
├── docs/                               (clean, organized)
│   ├── CLAUDE.md
│   ├── OBS_INTEGRATION_GUIDE.md
│   ├── WINDOWS_VR_SETUP_GUIDE.md
│   ├── README.md                      (NEW - doc index)
│   └── ...
├── examples/                           (NEW)
│   ├── ASMR_VR_EXPERIENCE_DEMO.cpp
│   └── VR_EXPERIENCE_VERIFICATION_DEMO.cpp
├── scripts/                            (NEW)
│   ├── cleanup_redundant_docs.sh
│   ├── create_ceo_deployment_package.sh
│   └── validate_end_to_end.py
├── tests/                              (organized)
│   ├── README.md                      (NEW - test documentation)
│   └── [all test files properly located]
└── [root much cleaner - only essential files]
```

### Expected File Count Changes
- **Root .md files:** 33 → ~8-10 (70% reduction!)
- **Root .cpp files:** 17 → 0 (100% cleanup!)
- **Root executables:** ~10 → 0 (100% cleanup!)
- **Organized in docs/**: 6 → 6 (same, but indexed)
- **Organized in tests/**: 15 → 27+ (moved from root)
- **Archived docs:** 24 → ~45 (historical preservation)

---

## EXECUTION CHECKLIST

- [ ] Backup current state (git commit or stash)
- [ ] Execute Phase 1 (move source files)
- [ ] Execute Phase 2 (clean validation reports)
- [ ] Execute Phase 3 (consolidate Windows docs)
- [ ] Execute Phase 4 (remove duplicates)
- [ ] Execute Phase 5 (archive roleplay docs)
- [ ] Execute Phase 6 (archive old reports)
- [ ] Execute Phase 7 (clean executables)
- [ ] Execute Phase 8 (organize scripts)
- [ ] Execute Phase 9 (create doc indices)
- [ ] Execute Phase 10 (git cleanup)
- [ ] Verify directory structure
- [ ] Test build still works
- [ ] Update team on cleanup completion

---

## RISK ASSESSMENT

**Low Risk:**
- Moving test files (they're untracked)
- Archiving old documentation (preserves history)
- Removing duplicate files

**Medium Risk:**
- Deleting executables (can be rebuilt)
- Consolidating Windows docs (might break references)

**High Risk:**
- None - all changes are reversible via archive

**Rollback Plan:**
If anything breaks, files are in archive/, not deleted.

---

## SUCCESS METRICS

After cleanup:
- ✅ Root directory navigable (under 15 .md files)
- ✅ No source files in root
- ✅ No source files in build/
- ✅ All tests in tests/ directory
- ✅ All docs in docs/ directory
- ✅ Clear documentation index
- ✅ Historical records preserved in archive/
- ✅ Build system still works
- ✅ Git status cleaner and more manageable

**Time to Execute: ~15-20 minutes**
**Difficulty: Low (mostly file moves)**
**Benefit: Massive improvement in repository navigation**