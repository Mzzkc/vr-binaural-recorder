# GIT ACCOUNTABILITY PROCEDURES
**VR Binaural Recorder Project - Version Control Standards**

**Date:** September 24, 2025
**Authority:** Git Guru (Claude Code)
**Status:** MANDATORY - ALL ENGINEERS MUST COMPLY

---

## PURPOSE AND SCOPE

This document establishes comprehensive git tracking and accountability procedures to ensure:
- **Complete engineer attribution** for all work performed
- **Transparent development history** showing who did what work
- **Proper documentation** of all code changes and decisions
- **Accountability enforcement** preventing unauthorized or untracked changes

**Client Requirement:** No more building without proper version control practices and engineer accountability.

---

## MANDATORY COMMIT MESSAGE FORMAT

### Standard Commit Message Structure
```
[ENGINEER]: Brief description of change (50 chars max)

Detailed description of what was implemented/changed.
Explain the why, not just the what.

Engineer: [Full Name/Role]
Files Modified: [List of key files changed]
Documentation Updated: [Yes/No and which files]
Testing: [What testing was performed]
Review Required: [Yes/No - specify if peer review needed]

🤖 Generated with [Claude Code](https://claude.ai/code)
Co-Authored-By: Claude <noreply@anthropic.com>
```

### Examples of Proper Attribution

**Veteran Engineer Commit:**
```
[VETERAN]: Implement real-time HRTF convolution processing

Replace stub implementations with industry-standard FFTW-based
convolution for spatial audio processing. Achieves <10ms latency
target with zero memory allocations in audio callback.

Engineer: Veteran Software Engineer
Files Modified: modules/audio/hrtf_processor.cpp, modules/audio/convolution_engine.cpp
Documentation Updated: Yes - API_DOCUMENTATION.md
Testing: Performance benchmarks, real-time processing validation
Review Required: Yes - Solution Architect approval for architecture

🤖 Generated with [Claude Code](https://claude.ai/code)
Co-Authored-By: Claude <noreply@anthropic.com>
```

**Solution Architect Commit:**
```
[ARCHITECT]: Correct fundamental architecture from recording to routing

Implement virtual audio device architecture to route spatial audio
to external applications instead of recording to files. Aligns
with original requirements for real-time audio transformation.

Engineer: Solution Architect
Files Modified: core/main.cpp, modules/audio/virtual_device.cpp
Documentation Updated: Yes - CORRECTED_ARCHITECTURE_DESIGN.md
Testing: External application integration tests
Review Required: Yes - Team architectural review

🤖 Generated with [Claude Code](https://claude.ai/code)
Co-Authored-By: Claude <noreply@anthropic.com>
```

**Directory Cleanup Specialist Commit:**
```
[CLEANUP]: Eliminate recording functionality and simplify structure

Remove 47 recording-related files and reduce configuration from
166 to 20 parameters. Focus codebase on audio routing requirements.

Engineer: Directory Cleanup Specialist
Files Modified: [Removed] recording/, [Modified] config/
Documentation Updated: Yes - Updated README.md
Testing: Build system validation, core functionality verification
Review Required: No - Cleanup following approved simplification plan

🤖 Generated with [Claude Code](https://claude.ai/code)
Co-Authored-By: Claude <noreply@anthropic.com>
```

**Project Manager Commit:**
```
[PM]: Implement project realignment and governance procedures

Establish development governance to prevent architectural drift.
Create team coordination procedures and success metrics tracking.

Engineer: Project Manager
Files Modified: DEVELOPMENT_GOVERNANCE_PROCEDURES.md, PROJECT_VISION_REALIGNMENT.md
Documentation Updated: Yes - Multiple governance documents
Testing: Process compliance validation
Review Required: Yes - Team approval of governance procedures

🤖 Generated with [Claude Code](https://claude.ai/code)
Co-Authored-By: Claude <noreply@anthropic.com>
```

---

## ENGINEER IDENTIFICATION SYSTEM

### Primary Engineers and Responsibilities

**Veteran Software Engineer:**
- All technical implementation and code quality work
- Performance optimization and real-time processing
- Code review and technical debt resolution
- Industry standards compliance

**Solution Architect:**
- All architectural decisions and design work
- System design and technology selection
- Architecture compliance and governance
- Integration patterns and best practices

**Directory Cleanup Specialist:**
- All organization and cleanup work
- File structure optimization
- Documentation reduction and organization
- Configuration simplification

**Project Manager:**
- All coordination and process work
- Team communication and alignment
- Success metrics and milestone tracking
- Governance procedure implementation

### Attribution Requirements

**MANDATORY FIELDS in ALL commits:**
- `Engineer:` - Full role/name of the engineer
- `Files Modified:` - Key files changed (not exhaustive list)
- `Documentation Updated:` - Yes/No and which documentation
- `Testing:` - What testing was performed
- `Review Required:` - Whether peer review is needed

---

## BRANCH AND WORKFLOW STANDARDS

### Branch Naming Convention
```
[engineer-type]/[feature-description]
```

**Examples:**
- `veteran/hrtf-convolution-implementation`
- `architect/virtual-device-design`
- `cleanup/recording-functionality-removal`
- `pm/governance-procedures-implementation`

### Mandatory Workflow Steps

**Before Any Code Changes:**
1. Create properly named feature branch
2. Document planned changes in commit message draft
3. Coordinate with affected team members
4. Ensure understanding of architectural impact

**During Development:**
1. Make frequent, atomic commits with proper attribution
2. Update documentation with each significant change
3. Run tests before each commit
4. Maintain build system functionality

**Before Merging:**
1. Complete peer review (if marked as required)
2. Validate all tests pass
3. Confirm documentation is current
4. Verify architectural compliance

### Merge Requirements
- All commits must have proper engineer attribution
- No merge without documented testing
- No merge of broken builds
- No merge without required peer review

---

## ACCOUNTABILITY ENFORCEMENT

### Automated Validation (Git Hooks)

**Pre-commit Hook Requirements:**
```bash
#!/bin/bash
# Validate commit message format
if ! grep -q "Engineer:" "$1"; then
    echo "ERROR: Commit missing 'Engineer:' attribution"
    exit 1
fi

if ! grep -q "Files Modified:" "$1"; then
    echo "ERROR: Commit missing 'Files Modified:' field"
    exit 1
fi

if ! grep -q "Testing:" "$1"; then
    echo "ERROR: Commit missing 'Testing:' field"
    exit 1
fi
```

**Pre-push Hook Requirements:**
```bash
#!/bin/bash
# Validate branch naming
branch=$(git rev-parse --abbrev-ref HEAD)
if [[ ! "$branch" =~ ^(veteran|architect|cleanup|pm)/ ]]; then
    echo "ERROR: Branch must start with engineer type (veteran/architect/cleanup/pm)"
    exit 1
fi
```

### Manual Review Requirements

**Daily Accountability Review:**
- All commits from previous day reviewed for proper attribution
- Missing attribution issues escalated immediately
- Documentation compliance verified
- Testing requirements validated

**Weekly Team Review:**
- Engineer contribution analysis
- Attribution accuracy assessment
- Process compliance evaluation
- Improvement recommendations

### Violation Response Procedures

**Missing Attribution (Level 1):**
- Immediate notification to committing engineer
- Required commit amendment with proper attribution
- Documentation of violation in team tracker

**Repeated Violations (Level 2):**
- Team meeting to review accountability procedures
- Additional training on git standards
- Enhanced review requirements for violating engineer

**Systematic Non-Compliance (Level 3):**
- Project manager escalation
- Client notification of accountability issues
- Potential access restriction pending compliance

---

## TOOLS AND AUTOMATION

### Required Git Configuration

**Global Git Settings:**
```bash
git config --global user.name "[Full Engineer Name/Role]"
git config --global user.email "[engineer]@vr-binaural-project.local"
git config --global commit.template .gitmessage
```

**Commit Message Template (.gitmessage):**
```
[ENGINEER]: Brief description (50 chars max)

Detailed description of changes and rationale.

Engineer: [Your Full Name/Role]
Files Modified:
Documentation Updated: [Yes/No]
Testing:
Review Required: [Yes/No]

🤖 Generated with [Claude Code](https://claude.ai/code)
Co-Authored-By: Claude <noreply@anthropic.com>
```

### Recommended Tools

**For Commit Message Validation:**
- `.githooks/` directory with validation scripts
- Pre-commit framework integration
- Automated attribution checking

**For Contribution Tracking:**
- `git log --author="[Engineer]"` for individual contributions
- `git shortlog -sn` for contribution summaries
- Custom scripts for attribution analysis

---

## DOCUMENTATION REQUIREMENTS

### Code Documentation Standards

**File Headers:**
```cpp
/**
 * @file filename.cpp
 * @brief Brief description of file purpose
 * @author [Engineer Name/Role]
 * @date [Date of creation/major modification]
 * @version [Version if applicable]
 *
 * Detailed description of file contents and purpose.
 * Include any special considerations or dependencies.
 */
```

**Function Documentation:**
```cpp
/**
 * @brief Brief function description
 * @param paramName Description of parameter
 * @return Description of return value
 * @author [Engineer Name] - [Date]
 *
 * Detailed function description including algorithm approach,
 * performance characteristics, and usage examples.
 */
```

### Commit Documentation Requirements

**For New Features:**
- Detailed explanation of feature purpose and implementation
- Performance impact analysis
- Integration points with existing systems
- Testing methodology and results

**For Bug Fixes:**
- Root cause analysis of the bug
- Description of fix approach
- Validation that fix resolves issue
- Regression testing performed

**For Refactoring:**
- Reason for refactoring (performance, maintainability, etc.)
- Changes made to code structure
- Verification that functionality preserved
- Performance impact (positive/negative/neutral)

---

## QUALITY ASSURANCE

### Pre-Release Accountability Audit

**Before Any Release:**
1. Complete git log review for proper attribution
2. Verify all commits have required fields
3. Confirm documentation is current
4. Validate testing coverage

**Attribution Coverage Requirements:**
- 100% of commits have engineer attribution
- 100% of commits document testing performed
- 100% of file modifications tracked
- 100% of documentation updates recorded

### Continuous Compliance Monitoring

**Daily Automated Checks:**
- Scan recent commits for missing attribution
- Verify commit message format compliance
- Check branch naming convention adherence
- Validate required review completion

**Weekly Compliance Reports:**
```
Weekly Git Accountability Report - Week [Date]

COMPLIANCE METRICS:
├── Commits with proper attribution: [%]
├── Commits with testing documentation: [%]
├── Branches following naming convention: [%]
├── Required reviews completed: [%]
└── Documentation currency: [%]

ENGINEER CONTRIBUTIONS:
├── Veteran Engineer: [# commits] [# files modified]
├── Solution Architect: [# commits] [# files modified]
├── Directory Cleanup Specialist: [# commits] [# files modified]
└── Project Manager: [# commits] [# files modified]

COMPLIANCE ISSUES:
├── Missing attribution: [Count]
├── Missing testing documentation: [Count]
├── Incomplete reviews: [Count]
└── Actions taken: [Description]
```

---

## SUCCESS METRICS

### Key Performance Indicators

**Attribution Compliance:**
- Target: 100% commits with proper engineer attribution
- Measurement: Weekly automated audit
- Escalation: Any week below 95% compliance

**Documentation Currency:**
- Target: All code changes have corresponding documentation updates
- Measurement: Manual review of commits vs. documentation changes
- Escalation: Documentation lag exceeding 1 week

**Review Effectiveness:**
- Target: All required reviews completed within 2 business days
- Measurement: Time from review request to completion
- Escalation: Reviews pending beyond target time

**Process Adherence:**
- Target: 100% adherence to branch naming and workflow
- Measurement: Automated validation and manual audit
- Escalation: Pattern of non-compliance

### Continuous Improvement

**Monthly Process Review:**
- Evaluate effectiveness of accountability procedures
- Collect team feedback on process burden vs. benefit
- Identify areas for automation improvement
- Update procedures based on lessons learned

**Quarterly Client Reporting:**
- Comprehensive accountability implementation status
- Engineer contribution analysis and recognition
- Process compliance metrics and trends
- Recommendations for continued improvement

---

## CONCLUSION

These git accountability procedures ensure:
- **Complete transparency** in who contributes what work
- **Proper attribution** for all engineering efforts
- **Comprehensive tracking** of all code and documentation changes
- **Quality assurance** through required testing documentation
- **Continuous improvement** through regular compliance review

**ALL ENGINEERS ARE REQUIRED TO FOLLOW THESE PROCEDURES**

Non-compliance will result in escalation and potential access restrictions until compliance is achieved.

**Client satisfaction depends on proper version control accountability. These procedures ensure we deliver on that requirement.**

---

**Document Owner:** Git Guru (Claude Code)
**Review Schedule:** Monthly
**Approval Required:** Project Manager and Team Lead
**Implementation Date:** September 24, 2025
**Next Review:** October 24, 2025