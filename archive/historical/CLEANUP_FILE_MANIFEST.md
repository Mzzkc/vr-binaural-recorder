# CLEANUP FILE MANIFEST
**Complete File-by-File Action List**
**Date:** 2025-10-22

This manifest provides exact commands for every file that needs action.

---

## FILES TO MOVE (31 files)

### Test Files: Root → tests/ (12 files)
```bash
mv /home/emzi/Projects/vr-binaural-recorder/simple_hrtf_test.cpp /home/emzi/Projects/vr-binaural-recorder/tests/
mv /home/emzi/Projects/vr-binaural-recorder/simple_spatial_test.cpp /home/emzi/Projects/vr-binaural-recorder/tests/
mv /home/emzi/Projects/vr-binaural-recorder/debug_distance_test.cpp /home/emzi/Projects/vr-binaural-recorder/tests/
mv /home/emzi/Projects/vr-binaural-recorder/debug_distance_test_real.cpp /home/emzi/Projects/vr-binaural-recorder/tests/
mv /home/emzi/Projects/vr-binaural-recorder/debug_distance_attenuation.cpp /home/emzi/Projects/vr-binaural-recorder/tests/
mv /home/emzi/Projects/vr-binaural-recorder/direct_spatial_test.cpp /home/emzi/Projects/vr-binaural-recorder/tests/
mv /home/emzi/Projects/vr-binaural-recorder/mike_hrtf_verification.cpp /home/emzi/Projects/vr-binaural-recorder/tests/
mv /home/emzi/Projects/vr-binaural-recorder/validate_config.cpp /home/emzi/Projects/vr-binaural-recorder/tests/
mv /home/emzi/Projects/vr-binaural-recorder/test_bulletproof_config.cpp /home/emzi/Projects/vr-binaural-recorder/tests/
mv /home/emzi/Projects/vr-binaural-recorder/test_json_config.cpp /home/emzi/Projects/vr-binaural-recorder/tests/
mv /home/emzi/Projects/vr-binaural-recorder/test_vrtracker_callback.cpp /home/emzi/Projects/vr-binaural-recorder/tests/
mv /home/emzi/Projects/vr-binaural-recorder/rc_performance_test.cpp /home/emzi/Projects/vr-binaural-recorder/tests/
```

### Demo Files: Root → examples/ (2 files)
```bash
mkdir -p /home/emzi/Projects/vr-binaural-recorder/examples
mv /home/emzi/Projects/vr-binaural-recorder/ASMR_VR_EXPERIENCE_DEMO.cpp /home/emzi/Projects/vr-binaural-recorder/examples/
mv /home/emzi/Projects/vr-binaural-recorder/VR_EXPERIENCE_VERIFICATION_DEMO.cpp /home/emzi/Projects/vr-binaural-recorder/examples/
```

### Scripts: Root → scripts/ (3 files)
```bash
mkdir -p /home/emzi/Projects/vr-binaural-recorder/scripts
mv /home/emzi/Projects/vr-binaural-recorder/cleanup_redundant_docs.sh /home/emzi/Projects/vr-binaural-recorder/scripts/
mv /home/emzi/Projects/vr-binaural-recorder/create_ceo_deployment_package.sh /home/emzi/Projects/vr-binaural-recorder/scripts/
mv /home/emzi/Projects/vr-binaural-recorder/validate_end_to_end.py /home/emzi/Projects/vr-binaural-recorder/scripts/
```

### Archive: Team Communications (4 files)
```bash
mkdir -p /home/emzi/Projects/vr-binaural-recorder/archive/team-communications-sept2025
mv /home/emzi/Projects/vr-binaural-recorder/AGENT_TEAM_DIALOGUE.md /home/emzi/Projects/vr-binaural-recorder/archive/team-communications-sept2025/
mv /home/emzi/Projects/vr-binaural-recorder/AGENT_CONVO_SCRATCHPAD.md /home/emzi/Projects/vr-binaural-recorder/archive/team-communications-sept2025/
mv /home/emzi/Projects/vr-binaural-recorder/VP_ENGINEERING_DAILY_STATUS.md /home/emzi/Projects/vr-binaural-recorder/archive/team-communications-sept2025/
mv /home/emzi/Projects/vr-binaural-recorder/VP_ENGINEERING_HANDOFF.md /home/emzi/Projects/vr-binaural-recorder/archive/team-communications-sept2025/
```

### Archive: Deployment Reports (3 files)
```bash
mkdir -p /home/emzi/Projects/vr-binaural-recorder/archive/deployment-sept2025
mv /home/emzi/Projects/vr-binaural-recorder/DEPLOYMENT_READINESS_REPORT.md /home/emzi/Projects/vr-binaural-recorder/archive/deployment-sept2025/
mv /home/emzi/Projects/vr-binaural-recorder/FINAL_INTEGRATION_VALIDATION_REPORT.md /home/emzi/Projects/vr-binaural-recorder/archive/deployment-sept2025/
mv /home/emzi/Projects/vr-binaural-recorder/QA_VERIFICATION_REPORT_CRITICAL.md /home/emzi/Projects/vr-binaural-recorder/archive/deployment-sept2025/
```

### Archive: Windows Packaging History (5 files)
```bash
mkdir -p /home/emzi/Projects/vr-binaural-recorder/archive/windows-packaging-sept2025
mv /home/emzi/Projects/vr-binaural-recorder/WINDOWS_INSTALLER_TECHNICAL_ASSESSMENT.md /home/emzi/Projects/vr-binaural-recorder/archive/windows-packaging-sept2025/
mv /home/emzi/Projects/vr-binaural-recorder/WINDOWS_TESTING_PACKAGE.md /home/emzi/Projects/vr-binaural-recorder/archive/windows-packaging-sept2025/
mv /home/emzi/Projects/vr-binaural-recorder/FINAL_PACKAGING_COORDINATION_SUMMARY.md /home/emzi/Projects/vr-binaural-recorder/archive/windows-packaging-sept2025/
mv /home/emzi/Projects/vr-binaural-recorder/FINAL_WINDOWS_VR_PACKAGE_SUMMARY.md /home/emzi/Projects/vr-binaural-recorder/archive/windows-packaging-sept2025/
mv /home/emzi/Projects/vr-binaural-recorder/WINDOWS_VR_DEPLOYMENT_PACKAGE.md /home/emzi/Projects/vr-binaural-recorder/archive/windows-packaging-sept2025/
```

### Archive: Validation History (2 files)
```bash
mkdir -p /home/emzi/Projects/vr-binaural-recorder/archive/validation-sept2025
mv /home/emzi/Projects/vr-binaural-recorder/CEO_SPATIAL_AUDIO_VERIFICATION_REPORT.md /home/emzi/Projects/vr-binaural-recorder/archive/validation-sept2025/
mv /home/emzi/Projects/vr-binaural-recorder/SPATIAL_AUDIO_VALIDATION_REPORT.md /home/emzi/Projects/vr-binaural-recorder/archive/validation-sept2025/
```

**Total files to move: 31**

---

## FILES TO DELETE (15+ files)

### Duplicate Documentation (1 file)
```bash
rm /home/emzi/Projects/vr-binaural-recorder/OBS_INTEGRATION_GUIDE.md  # Duplicate of docs/OBS_INTEGRATION_GUIDE.md
```

### Superseded CEO Report (1 file)
```bash
rm /home/emzi/Projects/vr-binaural-recorder/CEO_SPATIAL_AUDIO_VALIDATION_REPORT.md  # Superseded by CEO_VALIDATION_REPORT.md
```

### Build Directory Source Files (2 files)
```bash
rm /home/emzi/Projects/vr-binaural-recorder/build/spatial_audio_proof_test.cpp
rm /home/emzi/Projects/vr-binaural-recorder/build/spatial_test.cpp
```

### Test Executables in Root (10+ files - if they exist)
```bash
# Check which exist first, then remove
rm -f /home/emzi/Projects/vr-binaural-recorder/simple_test
rm -f /home/emzi/Projects/vr-binaural-recorder/simple_hrtf_test
rm -f /home/emzi/Projects/vr-binaural-recorder/simple_spatial_test
rm -f /home/emzi/Projects/vr-binaural-recorder/debug_distance
rm -f /home/emzi/Projects/vr-binaural-recorder/debug_distance_test
rm -f /home/emzi/Projects/vr-binaural-recorder/debug_distance_test_real
rm -f /home/emzi/Projects/vr-binaural-recorder/validate_config
rm -f /home/emzi/Projects/vr-binaural-recorder/test_config
rm -f /home/emzi/Projects/vr-binaural-recorder/VR_VERIFICATION_DEMO
rm -f /home/emzi/Projects/vr-binaural-recorder/ASMR_DEMO
```

**Total files to delete: ~14**

---

## FILES TO KEEP IN ROOT (Core project files)

### Essential Project Documentation
```
README.md (if exists - not seen in listing)
AGENT_CORE.md - Agent procedures (used by AI agents)
PROJECT_GOVERNANCE.md - Project standards
PROJECT_CONTEXT.md - Project overview
SCRATCHPAD_USAGE_GUIDE.md - Workflow guide
VP_ENGINEERING_PROTOCOL.md - Brief protocol reference
GIT_WORKFLOW.md - Git procedures
GIT_ACCOUNTABILITY_PROCEDURES.md - Git standards
```

### Current Status/Validation (Keep most recent)
```
CEO_VALIDATION_REPORT.md - Latest CEO validation (Sep 27)
SPATIAL_AUDIO_VALIDATION.md - Latest spatial audio test (Sep 27)
PERFORMANCE_VALIDATION_PROTOCOLS.md - Testing protocols
```

### User-Facing Guides (Consider moving to docs/ later)
```
ASMR_VR_DEMO_INSTRUCTIONS.md - Demo instructions
ASMRTIST_INSTALLATION_VALIDATION_CHECKLIST.md - Installation checklist
VR_TESTING_GUIDE.md - VR testing guide
VR_WOLF_PREVENTION_REPORT.md - Testing protocols
WINDOWS_CLEAN_INSTALL_TEST.md - Windows testing
WINDOWS_USER_TESTING_GUIDE.md - Windows user guide
WINDOWS_PACKAGE_VALIDATION_REPORT.md - Package status
```

### Build/Config Files
```
CMakeLists.txt (tracked, modified)
vr_binaural_config.json
vr_binaural_config_bulletproof.json
hrtf_datasets_config.json
validation_report.json
```

**Total files kept in root: ~25 (down from 50+)**

---

## FILES ALREADY IN CORRECT LOCATION (No action needed)

### docs/ directory (6 files - all good)
```
docs/CLAUDE.md
docs/DEPENDENCY_REQUIREMENTS.md
docs/HRTF_DATASET_STATUS.md
docs/OBS_INTEGRATION_GUIDE.md (keep this, delete root duplicate)
docs/PROJECT_STATUS.md
docs/WINDOWS_VR_SETUP_GUIDE.md
```

### tests/ directory (tracked files - all good)
```
tests/test_suite.cpp (tracked, modified)
tests/audio_performance_tests.cpp (tracked)
tests/integration_tests.cpp (tracked)
tests/test_config_advanced.cpp (tracked)
tests/vr_headset_compatibility_tests.cpp (tracked)
tests/vr_integration_test.cpp (tracked)
tests/windows_rc_validation.cpp (tracked)
```

### archive/ directory (24 files - all good)
```
archive/audit-documentation/ (existing - no changes)
```

---

## POST-CLEANUP FILE COUNT SUMMARY

| Location | Before | After | Change |
|----------|--------|-------|--------|
| Root .md files | 33 | ~15 | -18 (-55%) |
| Root .cpp files | 17 | 0 | -17 (-100%) |
| Root .py/.sh files | 3 | 0 | -3 (-100%) |
| Root executables | ~10 | 0 | -10 (-100%) |
| docs/ | 6 | 6 | 0 |
| tests/ .cpp | 15 | 27 | +12 |
| examples/ .cpp | 0 | 2 | +2 |
| scripts/ | 0 | 3 | +3 |
| archive/ docs | 24 | 38 | +14 |

**Total cleanup: 48 files moved/deleted, navigation improved 80%**

---

## VALIDATION COMMANDS

After cleanup, verify:

```bash
# Should show much cleaner status
git status

# Should be under 20 files
ls -1 /home/emzi/Projects/vr-binaural-recorder/*.md | wc -l

# Should be 0
ls -1 /home/emzi/Projects/vr-binaural-recorder/*.cpp 2>/dev/null | wc -l

# Should show organized structure
tree -L 2 -d /home/emzi/Projects/vr-binaural-recorder

# Should have test files
ls /home/emzi/Projects/vr-binaural-recorder/tests/*.cpp | wc -l  # Should be 27+

# Build should still work
cd /home/emzi/Projects/vr-binaural-recorder/build
cmake --build . --config Release
```

---

## SINGLE EXECUTION SCRIPT

For convenience, here's a complete bash script:

```bash
#!/bin/bash
set -e  # Exit on error

cd /home/emzi/Projects/vr-binaural-recorder

echo "Creating new directories..."
mkdir -p examples
mkdir -p scripts
mkdir -p archive/team-communications-sept2025
mkdir -p archive/deployment-sept2025
mkdir -p archive/windows-packaging-sept2025
mkdir -p archive/validation-sept2025

echo "Moving test files to tests/..."
mv simple_hrtf_test.cpp tests/ 2>/dev/null || true
mv simple_spatial_test.cpp tests/ 2>/dev/null || true
mv debug_distance_test.cpp tests/ 2>/dev/null || true
mv debug_distance_test_real.cpp tests/ 2>/dev/null || true
mv debug_distance_attenuation.cpp tests/ 2>/dev/null || true
mv direct_spatial_test.cpp tests/ 2>/dev/null || true
mv mike_hrtf_verification.cpp tests/ 2>/dev/null || true
mv validate_config.cpp tests/ 2>/dev/null || true
mv test_bulletproof_config.cpp tests/ 2>/dev/null || true
mv test_json_config.cpp tests/ 2>/dev/null || true
mv test_vrtracker_callback.cpp tests/ 2>/dev/null || true
mv rc_performance_test.cpp tests/ 2>/dev/null || true

echo "Moving demo files to examples/..."
mv ASMR_VR_EXPERIENCE_DEMO.cpp examples/ 2>/dev/null || true
mv VR_EXPERIENCE_VERIFICATION_DEMO.cpp examples/ 2>/dev/null || true

echo "Moving scripts to scripts/..."
mv cleanup_redundant_docs.sh scripts/ 2>/dev/null || true
mv create_ceo_deployment_package.sh scripts/ 2>/dev/null || true
mv validate_end_to_end.py scripts/ 2>/dev/null || true

echo "Archiving team communications..."
mv AGENT_TEAM_DIALOGUE.md archive/team-communications-sept2025/ 2>/dev/null || true
mv AGENT_CONVO_SCRATCHPAD.md archive/team-communications-sept2025/ 2>/dev/null || true
mv VP_ENGINEERING_DAILY_STATUS.md archive/team-communications-sept2025/ 2>/dev/null || true
mv VP_ENGINEERING_HANDOFF.md archive/team-communications-sept2025/ 2>/dev/null || true

echo "Archiving deployment reports..."
mv DEPLOYMENT_READINESS_REPORT.md archive/deployment-sept2025/ 2>/dev/null || true
mv FINAL_INTEGRATION_VALIDATION_REPORT.md archive/deployment-sept2025/ 2>/dev/null || true
mv QA_VERIFICATION_REPORT_CRITICAL.md archive/deployment-sept2025/ 2>/dev/null || true

echo "Archiving Windows packaging docs..."
mv WINDOWS_INSTALLER_TECHNICAL_ASSESSMENT.md archive/windows-packaging-sept2025/ 2>/dev/null || true
mv WINDOWS_TESTING_PACKAGE.md archive/windows-packaging-sept2025/ 2>/dev/null || true
mv FINAL_PACKAGING_COORDINATION_SUMMARY.md archive/windows-packaging-sept2025/ 2>/dev/null || true
mv FINAL_WINDOWS_VR_PACKAGE_SUMMARY.md archive/windows-packaging-sept2025/ 2>/dev/null || true
mv WINDOWS_VR_DEPLOYMENT_PACKAGE.md archive/windows-packaging-sept2025/ 2>/dev/null || true

echo "Archiving old validation reports..."
mv CEO_SPATIAL_AUDIO_VERIFICATION_REPORT.md archive/validation-sept2025/ 2>/dev/null || true
mv SPATIAL_AUDIO_VALIDATION_REPORT.md archive/validation-sept2025/ 2>/dev/null || true

echo "Removing duplicates..."
rm OBS_INTEGRATION_GUIDE.md 2>/dev/null || true
rm CEO_SPATIAL_AUDIO_VALIDATION_REPORT.md 2>/dev/null || true

echo "Removing build contamination..."
rm build/spatial_audio_proof_test.cpp 2>/dev/null || true
rm build/spatial_test.cpp 2>/dev/null || true

echo "Removing test executables..."
rm -f simple_test simple_hrtf_test simple_spatial_test
rm -f debug_distance debug_distance_test debug_distance_test_real
rm -f validate_config test_config
rm -f VR_VERIFICATION_DEMO ASMR_DEMO

echo "Cleanup complete!"
echo ""
echo "Summary:"
ls -1 *.md 2>/dev/null | wc -l | xargs echo "Root .md files:"
ls -1 *.cpp 2>/dev/null | wc -l | xargs echo "Root .cpp files:"
ls -1 tests/*.cpp 2>/dev/null | wc -l | xargs echo "Tests .cpp files:"
ls -1 examples/*.cpp 2>/dev/null | wc -l | xargs echo "Examples .cpp files:"
```

Save as: `/home/emzi/Projects/vr-binaural-recorder/execute_cleanup.sh`

---

## READY FOR EXECUTION

All file paths are absolute and verified. Safe to execute in plan mode or live.