# ARCHITECTURAL GOVERNANCE PROCEDURES

## Purpose

This document establishes procedures to prevent future architectural drift and ensure all development aligns with the core use case of **audio routing and transformation**, not recording functionality.

## Core Architectural Principles

### 1. Use Case Validation Principle
**RULE**: Every component must directly serve the audio routing/transformation use case.

**Validation Questions**:
- Does this component help route audio between applications?
- Does this component apply real-time spatial transformations?
- Does this component provide VR control over audio processing?

**REJECT if the component**:
- Manages recording sessions
- Handles file I/O in the audio path
- Provides recording quality/format options
- Implements take management or playback

### 2. Real-Time Processing Principle
**RULE**: Audio processing must be real-time safe with no blocking operations.

**Requirements**:
- Zero memory allocations in audio callback
- No file I/O in audio processing thread
- Lock-free data structures for shared state
- <10ms total system latency

**REJECT**:
- File writing during audio processing
- Memory allocation in real-time code
- Blocking synchronization in audio path
- Complex UI updates during audio processing

### 3. Simplicity Principle
**RULE**: Choose the simplest solution that meets requirements.

**Guidelines**:
- Use existing APIs instead of custom implementations
- Prefer composition over inheritance
- Minimize abstraction layers
- Target <5,000 lines total codebase

**REJECT**:
- Custom implementations of platform-provided functionality
- Over-engineered capability detection systems
- Multiple redundant approaches to the same problem
- Complex configuration systems

## Architectural Review Process

### Pre-Development Review

#### 1. Component Design Review
**REQUIRED BEFORE CODING**:

**Template**: `COMPONENT_DESIGN_REVIEW.md`
```markdown
# Component Design Review: [ComponentName]

## Use Case Validation
- [ ] Directly serves audio routing/transformation
- [ ] No recording functionality
- [ ] Real-time processing capable
- [ ] Integrates with virtual audio device

## Technical Requirements
- [ ] <500 lines of code
- [ ] Uses existing APIs where possible
- [ ] Lock-free for real-time components
- [ ] Platform-specific implementation available

## Architecture Compliance
- [ ] Follows layer separation
- [ ] No coupling to recording features
- [ ] Industry standard patterns used
- [ ] Simple configuration

## Approval
- [ ] Tech Lead Review
- [ ] Architect Review
- [ ] Use Case Validation
```

#### 2. API Design Review
**REQUIRED FOR ALL PUBLIC INTERFACES**:

```cpp
// GOOD: Audio routing interface
class SpatialAudioEngine {
public:
    void ProcessAudioBlock(const float* input, float* output, size_t frames);
    void UpdateSpatialParameters(float mix, bool enableHRTF);
};

// BAD: Recording interface (REJECT)
class AudioEngine {
public:
    bool StartRecording(const std::string& filename);
    void PauseRecording();
    // ARCHITECTURAL VIOLATION: Recording functionality
};
```

### Development Review Process

#### 1. Daily Architecture Check
**DAILY STANDUP QUESTIONS**:
1. Does your work support audio routing or transformation?
2. Are you adding any recording functionality? (If yes, STOP)
3. Are you using existing APIs or building custom solutions?
4. Will your changes affect real-time audio performance?

#### 2. Code Review Checklist

**MANDATORY CHECKS**:
```markdown
## Architectural Compliance Checklist

### Use Case Alignment
- [ ] No recording session management code
- [ ] No file format selection logic
- [ ] No recording quality presets
- [ ] No take management functionality

### Real-Time Safety
- [ ] No memory allocation in audio callback
- [ ] No file I/O in audio processing thread
- [ ] No blocking operations in real-time code
- [ ] Lock-free shared state access

### Simplicity Validation
- [ ] Uses platform APIs where available
- [ ] Minimal abstraction layers
- [ ] Simple configuration options
- [ ] Clear single responsibility

### Industry Standards
- [ ] Follows WASAPI/JACK/Core Audio patterns
- [ ] Uses OpenVR APIs directly
- [ ] Standard audio buffer management
- [ ] Platform-specific implementations
```

#### 3. Implementation Validation

**AUTOMATED CHECKS** (Add to CI/CD):
```bash
# Check for recording functionality
if grep -r "StartRecording\|StopRecording\|RecordingSession" src/; then
    echo "ERROR: Recording functionality detected - architectural violation"
    exit 1
fi

# Check for file I/O in audio code
if grep -r "fopen\|ofstream\|WriteFile" src/audio/; then
    echo "ERROR: File I/O in audio code - real-time violation"
    exit 1
fi

# Check for memory allocation in audio callback
if grep -r "new\|malloc\|make_unique" src/audio/*callback*; then
    echo "ERROR: Memory allocation in audio callback - real-time violation"
    exit 1
fi
```

## Technology Decision Framework

### 1. Virtual Audio Device Implementation

**DECISION MATRIX**:
| Platform | APPROVED Solution | REJECTED Solutions | Rationale |
|----------|------------------|-------------------|-----------|
| Windows | WASAPI Loopback | Custom audio driver | Use platform APIs |
| Linux | PulseAudio null-sink | ALSA custom device | Standard ecosystem solution |
| macOS | Core Audio aggregate | Custom Audio Unit | Native platform support |

### 2. VR Integration Decisions

**APPROVED**:
- Direct OpenVR API usage
- Minimal overlay UI
- Real-time pose updates
- Simple controller input

**REJECTED**:
- Custom VR tracking implementations
- Complex hardware detection
- Multi-vendor VR support
- VR testing frameworks

### 3. Audio Processing Decisions

**APPROVED**:
- Industry-standard HRTF libraries
- Platform-optimized audio APIs
- Lock-free circular buffers
- SIMD optimization where beneficial

**REJECTED**:
- Custom HRTF implementations
- File-based audio processing
- Complex audio effect chains
- Recording format conversion

## Configuration Management

### 1. Configuration Validation

**APPROVED CONFIGURATION CATEGORIES**:
```json
{
  "audioRouting": {    // ✅ Core functionality
    "virtualDeviceName": "string",
    "sampleRate": "number",
    "bufferSize": "number"
  },
  "spatialAudio": {    // ✅ Core functionality
    "enableHRTF": "boolean",
    "spatialMix": "number"
  },
  "vrIntegration": {   // ✅ Core functionality
    "overlayScale": "number",
    "trackingEnabled": "boolean"
  }
}
```

**REJECTED CONFIGURATION CATEGORIES**:
```json
{
  "recording": {       // ❌ ARCHITECTURAL VIOLATION
    "recordingPath": "any",
    "fileFormat": "any",
    "qualityPresets": "any"
  },
  "sessionManagement": { // ❌ ARCHITECTURAL VIOLATION
    "takeManagement": "any",
    "autoIncrement": "any"
  }
}
```

### 2. Configuration Review Process

**BEFORE ADDING NEW CONFIG OPTIONS**:
1. Validate against use case (audio routing/transformation only)
2. Ensure real-time safety (no blocking file operations)
3. Check for existing platform-standard alternatives
4. Document necessity and impact

## Error Prevention Strategies

### 1. Naming Conventions

**USE THESE TERMS**:
- AudioRouting, AudioTransformation, SpatialProcessing
- VirtualDevice, AudioPipeline, RealTimeProcessor
- OverlayControls, SpatialMix, HRTFProcessor

**NEVER USE THESE TERMS**:
- Recording, Recorder, RecordingSession
- TakeManager, FileFormat, AudioFile
- QualityPreset, RecordingPreset

### 2. Code Structure Guards

**DIRECTORY STRUCTURE VALIDATION**:
```
APPROVED:
src/
├── spatial_audio/    ✅ Core functionality
├── virtual_device/   ✅ Core functionality
├── vr_integration/   ✅ Core functionality

REJECTED:
src/
├── recording/        ❌ ARCHITECTURAL VIOLATION
├── session_mgmt/     ❌ ARCHITECTURAL VIOLATION
├── file_management/  ❌ ARCHITECTURAL VIOLATION
```

### 3. Dependency Management

**APPROVED DEPENDENCIES**:
- OpenVR (VR integration)
- PortAudio (cross-platform audio)
- Platform audio APIs (WASAPI, Core Audio, PulseAudio)
- HRTF libraries (MIT KEMAR dataset)

**REJECTED DEPENDENCIES**:
- Audio file format libraries (libflac, libogg, etc.)
- Recording frameworks
- Complex UI frameworks beyond minimal overlay
- Audio editing libraries

## Escalation Procedures

### 1. Architectural Violation Response

**IMMEDIATE RESPONSE**:
1. Stop development on violating component
2. Notify team lead and architect
3. Schedule architectural review meeting
4. Document violation and corrective action

**SEVERITY LEVELS**:
- **Critical**: Recording functionality added
- **High**: File I/O in audio processing
- **Medium**: Over-engineering simple functionality
- **Low**: Non-standard naming or structure

### 2. Architecture Decision Escalation

**WHEN TO ESCALATE**:
- Proposed solution requires >1,000 lines of code
- Platform API doesn't exist for required functionality
- Real-time requirements cannot be met
- Industry standard approach is unclear

**ESCALATION PROCESS**:
1. Document technical requirements and constraints
2. Research industry standard approaches
3. Present alternatives with trade-off analysis
4. Get approval before implementation

## Governance Tool Integration

### 1. Git Hooks

**PRE-COMMIT CHECKS**:
```bash
#!/bin/bash
# Pre-commit architectural validation

# Check for architectural violations
./scripts/validate-architecture.sh

# Check for recording functionality
if git diff --cached --name-only | xargs grep -l "Recording\|Record" 2>/dev/null; then
    echo "ERROR: Recording functionality detected in commit"
    echo "This violates architectural principles"
    exit 1
fi
```

### 2. CI/CD Integration

**BUILD PIPELINE CHECKS**:
- Run architectural violation detection
- Validate configuration schema
- Check real-time safety requirements
- Verify industry standard compliance

### 3. Documentation Requirements

**MANDATORY DOCUMENTATION**:
- Component purpose and use case validation
- API documentation with examples
- Real-time safety guarantees
- Platform-specific implementation notes

## Review and Update Process

### 1. Quarterly Architecture Review

**REVIEW AGENDA**:
1. Assess adherence to governance procedures
2. Review architectural violations and responses
3. Update procedures based on lessons learned
4. Validate architecture against evolving requirements

### 2. Procedure Updates

**UPDATE TRIGGERS**:
- New platform support requirements
- Industry standard changes
- Performance requirement changes
- Architectural violation patterns

### 3. Team Training

**ONGOING TRAINING REQUIREMENTS**:
- Monthly architectural principle review
- Platform API training for team members
- Real-time programming best practices
- Industry standard pattern workshops

---

**ENFORCEMENT**: These procedures are mandatory for all development. Violations will trigger immediate code review and corrective action. The goal is to prevent future architectural drift and maintain focus on the core audio routing/transformation use case.