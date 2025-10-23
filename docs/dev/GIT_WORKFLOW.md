# VR Binaural Recorder - Git Workflow and Branching Strategy

## Repository Overview

This repository follows a structured branching strategy designed for professional VR audio software development with emphasis on stability, testing, and collaborative development.

## Branch Structure

### Main Branches

#### `master` (Production)
- **Purpose**: Production-ready code only
- **Protection**: Require pull request reviews, status checks must pass
- **Deployment**: Automatically triggers release builds
- **Merge Policy**: Only from `develop` branch via pull request
- **Naming Convention**: `master`

#### `develop` (Integration)
- **Purpose**: Integration branch for ongoing development
- **Protection**: Require pull request reviews from feature branches
- **Testing**: All automated tests must pass before merge
- **Merge Policy**: Feature branches merge here first
- **Naming Convention**: `develop`

### Supporting Branches

#### Feature Branches
- **Purpose**: New features and enhancements
- **Naming Convention**: `feature/component-description`
- **Examples**:
  - `feature/audio-engine-optimization`
  - `feature/hrtf-dataset-loader`
  - `feature/vr-hand-tracking`
  - `feature/realtime-visualization`
- **Lifecycle**: Branch from `develop`, merge back to `develop`
- **Testing**: Must pass all existing tests, add new tests for new functionality

#### Bug Fix Branches
- **Purpose**: Non-critical bug fixes
- **Naming Convention**: `fix/issue-description`
- **Examples**:
  - `fix/audio-dropout-recovery`
  - `fix/vr-tracking-interpolation`
  - `fix/memory-leak-ring-buffer`
- **Lifecycle**: Branch from `develop`, merge back to `develop`

#### Hotfix Branches
- **Purpose**: Critical production fixes
- **Naming Convention**: `hotfix/critical-issue`
- **Examples**:
  - `hotfix/audio-crash-recovery`
  - `hotfix/vr-headset-detection`
- **Lifecycle**: Branch from `master`, merge to both `master` and `develop`
- **Priority**: Highest priority, immediate review required

#### Release Branches
- **Purpose**: Prepare releases, final testing and bug fixes
- **Naming Convention**: `release/v{major}.{minor}.{patch}`
- **Examples**:
  - `release/v1.0.0`
  - `release/v1.1.0`
- **Lifecycle**: Branch from `develop`, merge to `master` and `develop`
- **Activities**: Version bumps, final testing, documentation updates

#### Experimental Branches
- **Purpose**: Research, prototypes, experimental features
- **Naming Convention**: `experiment/research-topic`
- **Examples**:
  - `experiment/gpu-hrtf-processing`
  - `experiment/neural-audio-enhancement`
  - `experiment/blockchain-audio-nft`
- **Lifecycle**: May be long-lived, periodically sync with `develop`
- **Merge Policy**: Requires extensive review and testing

## Commit Message Standards

### Format
```
<type>(<scope>): <subject>

<body>

<footer>
```

### Types
- **feat**: New feature implementation
- **fix**: Bug fix
- **docs**: Documentation changes
- **style**: Code formatting, no functional changes
- **refactor**: Code restructuring without changing functionality
- **perf**: Performance improvements
- **test**: Adding or modifying tests
- **chore**: Build process, auxiliary tools, libraries
- **build**: Build system changes (CMake, dependencies)
- **ci**: Continuous integration changes

### Scopes (Component-based)
- **audio**: Audio engine, PortAudio integration
- **hrtf**: HRTF processing, spatial audio algorithms
- **vr**: VR tracking, OpenVR integration
- **ui**: User interface, overlay system
- **config**: Configuration management
- **build**: Build system, CMake, dependencies
- **test**: Testing infrastructure
- **docs**: Documentation

### Examples
```
feat(audio): add SIMD-optimized ring buffer with AVX2 support

Implement lock-free ring buffer with SIMD optimizations for
real-time audio processing. Reduces CPU usage by 15% in
high-throughput scenarios.

- AVX2 vectorized memory operations
- Cache-line aligned memory allocation
- Adaptive buffer sizing for different workloads
- Comprehensive unit tests with edge cases

Closes #123
```

```
fix(vr): resolve tracking interpolation jitter at low framerates

VR pose interpolation was causing jitter when headset framerate
dropped below 60fps. Improved prediction algorithm with better
temporal smoothing.

- Enhanced Kalman filter implementation
- Configurable smoothing parameters
- Fallback to linear interpolation for extreme cases

Fixes #456
```

## Development Workflow

### 1. Starting New Work

```bash
# Create feature branch from develop
git checkout develop
git pull origin develop
git checkout -b feature/audio-latency-optimization

# Work on your feature
# Make atomic commits with clear messages
git add <files>
git commit -m "feat(audio): reduce buffer latency with predictive sizing"
```

### 2. Regular Development

```bash
# Keep feature branch updated with develop
git checkout develop
git pull origin develop
git checkout feature/audio-latency-optimization
git merge develop

# Or use rebase for cleaner history
git rebase develop
```

### 3. Code Review Process

```bash
# Push feature branch
git push origin feature/audio-latency-optimization

# Create pull request to develop branch
# Required reviewers: At least 1 senior developer
# Required checks: All automated tests, build verification
```

### 4. Release Process

```bash
# Create release branch from develop
git checkout develop
git checkout -b release/v1.1.0

# Final testing, version updates, documentation
git commit -m "chore(release): bump version to 1.1.0"

# Merge to master
git checkout master
git merge release/v1.1.0
git tag v1.1.0

# Merge back to develop
git checkout develop
git merge release/v1.1.0
```

## Quality Gates

### Automated Checks
- **Build Verification**: All platforms (Windows, Linux, macOS)
- **Unit Tests**: All tests must pass (>90% coverage)
- **Integration Tests**: VR and audio system integration
- **Performance Tests**: Audio latency under acceptable thresholds
- **Code Quality**: No critical issues in static analysis
- **Security Scan**: No known vulnerabilities

### Manual Reviews
- **Code Review**: At least one senior developer approval
- **Architecture Review**: For changes affecting core systems
- **Performance Review**: For audio/VR critical path changes
- **Hardware Testing**: VR headset testing when applicable

## Branch Protection Rules

### Master Branch
- Require pull request reviews (2 reviewers)
- Dismiss stale reviews when new commits pushed
- Require status checks to pass
- Require branches to be up to date before merge
- Restrict pushes to administrators only
- Require signed commits

### Develop Branch
- Require pull request reviews (1 reviewer)
- Require status checks to pass
- Allow administrators to bypass restrictions
- Require linear history (no merge commits)

## Emergency Procedures

### Critical Production Issues

1. **Immediate Response**
   ```bash
   # Create hotfix branch from master
   git checkout master
   git checkout -b hotfix/critical-audio-crash

   # Make minimal fix
   git commit -m "hotfix(audio): prevent crash in HRTF processor"

   # Fast-track review and merge
   ```

2. **Communication**
   - Notify all team members immediately
   - Update issue tracker with emergency status
   - Document root cause and prevention measures

### Development Blockers

1. **Build System Issues**
   - Create `fix/build-system` branch
   - Prioritize resolution
   - May require emergency merge to develop

2. **Dependency Problems**
   - Update dependency management
   - Test across all supported platforms
   - Update documentation

## Git Configuration

### Repository Settings
```bash
# Enable automatic cleanup
git config core.autocrlf input
git config core.safecrlf true

# Set up signing
git config commit.gpgsign true
git config user.signingkey YOUR_GPG_KEY

# Performance settings
git config core.preloadindex true
git config core.fscache true
```

### Recommended Aliases
```bash
git config alias.co checkout
git config alias.br branch
git config alias.ci commit
git config alias.st status
git config alias.unstage 'reset HEAD --'
git config alias.last 'log -1 HEAD'
git config alias.visual '!gitk'
git config alias.graph 'log --graph --oneline --decorate --all'
```

## Tool Integration

### IDE/Editor Integration
- **Git hooks**: Automatic code formatting, commit validation
- **Pre-commit checks**: Build verification, test execution
- **Continuous Integration**: GitHub Actions, Jenkins, or similar

### Project Management
- **Issue Linking**: Link commits to issues automatically
- **Progress Tracking**: Update project boards on branch events
- **Release Notes**: Auto-generate from commit messages

## Best Practices

### Commit Practices
- **Atomic Commits**: One logical change per commit
- **Clear Messages**: Descriptive commit messages with context
- **Frequent Commits**: Commit early and often
- **Test Before Commit**: Ensure tests pass before committing

### Branch Management
- **Short-lived Branches**: Keep feature branches under 2 weeks
- **Regular Syncing**: Sync with develop branch frequently
- **Clean History**: Use interactive rebase to clean up history

### Code Review
- **Constructive Feedback**: Focus on code quality and learning
- **Performance Considerations**: Special attention to audio/VR paths
- **Documentation**: Ensure adequate documentation for new features

## Troubleshooting

### Common Issues

#### Merge Conflicts
```bash
# Resolve conflicts manually
git mergetool

# Or abort merge
git merge --abort
```

#### Lost Commits
```bash
# Find lost commits
git reflog

# Recover commit
git cherry-pick <commit-hash>
```

#### Wrong Branch
```bash
# Move commits to correct branch
git cherry-pick <commit-hash>
git reset --hard HEAD~1  # Remove from current branch
```

## Monitoring and Metrics

### Repository Health
- **Branch Age**: Alert if feature branches > 3 weeks old
- **Review Time**: Track time from PR creation to merge
- **Build Success Rate**: Monitor CI/CD pipeline health
- **Test Coverage**: Maintain >90% coverage requirement

### Development Velocity
- **Commit Frequency**: Track team productivity
- **Feature Completion**: Time from branch creation to merge
- **Bug Resolution Time**: Track fix turnaround time
- **Release Frequency**: Target monthly releases

This workflow ensures high code quality, stability, and efficient collaboration for the VR Binaural Recorder project.