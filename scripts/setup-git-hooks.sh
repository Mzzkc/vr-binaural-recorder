#!/bin/bash
# Setup git hooks for architectural governance
# Run this script to install mandatory governance hooks

echo "🔧 Setting up architectural governance git hooks..."

# Check if we're in a git repository
if [ ! -d ".git" ]; then
    echo "❌ Error: Not in a git repository"
    echo "Run this script from the project root directory"
    exit 1
fi

# Create git hooks directory if it doesn't exist
mkdir -p .git/hooks

# Install pre-commit hook
echo "Installing pre-commit governance hook..."
cp scripts/pre-commit-governance.sh .git/hooks/pre-commit
chmod +x .git/hooks/pre-commit

# Install commit-msg hook for commit message validation
echo "Installing commit message validation hook..."
cat > .git/hooks/commit-msg << 'EOF'
#!/bin/bash
# Commit message validation for architectural governance

commit_regex='^(feat|fix|docs|style|refactor|test|chore|build)(\([a-z0-9-]+\))?: .{1,50}'

error_msg="❌ INVALID COMMIT MESSAGE FORMAT

Commit message must follow conventional commits format:
<type>(<scope>): <description>

Types allowed:
- feat: New feature (audio routing/spatial processing only)
- fix: Bug fix
- docs: Documentation changes
- style: Code style changes
- refactor: Code refactoring
- test: Adding or updating tests
- chore: Maintenance tasks
- build: Build system changes

Examples:
✅ feat(audio): implement HRTF convolution processing
✅ fix(vr): correct pose tracking matrix conversion
✅ docs(api): update spatial audio interface documentation

❌ feat(recording): add recording session management (FORBIDDEN)
❌ fix audio bug (missing scope and colon)
❌ WIP: working on stuff (not descriptive)

For more details, see: DEVELOPMENT_GOVERNANCE_PROCEDURES.md"

# Check commit message format
if ! grep -qE "$commit_regex" "$1"; then
    echo "$error_msg"
    exit 1
fi

# Check for forbidden functionality in commit message
if grep -qiE "(record|recording|session|take|playback)" "$1"; then
    echo "❌ FORBIDDEN FUNCTIONALITY IN COMMIT MESSAGE"
    echo ""
    echo "Commit messages cannot reference recording functionality."
    echo "This project builds audio routing/transformation, not recording."
    echo ""
    echo "Use terms like: routing, spatial, processing, transformation"
    echo "Avoid terms like: recording, session, take, playback"
    exit 1
fi

echo "✅ Commit message format validated"
EOF

chmod +x .git/hooks/commit-msg

# Install pre-push hook for final validation
echo "Installing pre-push validation hook..."
cat > .git/hooks/pre-push << 'EOF'
#!/bin/bash
# Pre-push validation for architectural governance

echo "🔍 Running pre-push architectural validation..."

# Check if any recording functionality exists in the codebase
if grep -r "StartRecording\|StopRecording\|RecordingSession" src/ 2>/dev/null; then
    echo "❌ PUSH REJECTED: Recording functionality detected in codebase"
    echo ""
    echo "This violates core architectural principles."
    echo "Remove all recording functionality before pushing."
    echo ""
    exit 1
fi

# Check configuration complexity
if [ -f "config/vr_binaural_config.json" ]; then
    CONFIG_PARAMS=$(grep -o '"[^"]*"\s*:' config/vr_binaural_config.json | wc -l)
    if [ "$CONFIG_PARAMS" -gt 30 ]; then
        echo "❌ PUSH REJECTED: Configuration too complex ($CONFIG_PARAMS parameters)"
        echo ""
        echo "Maximum allowed: 30 parameters"
        echo "Simplify configuration before pushing."
        echo ""
        exit 1
    fi
fi

# Check for large components
LARGE_COMPONENTS=$(find src/ -name "*.cpp" -o -name "*.h" | xargs wc -l | awk '$1 > 1500 { print $2 " (" $1 " lines)" }' | head -5)
if [ -n "$LARGE_COMPONENTS" ]; then
    echo "⚠️  WARNING: Large components detected (>1500 lines):"
    echo "$LARGE_COMPONENTS"
    echo ""
    echo "Consider refactoring into smaller components."
    echo ""
fi

echo "✅ Pre-push validation passed"
EOF

chmod +x .git/hooks/pre-push

# Create backup of existing hooks if they exist
if [ -f ".git/hooks/pre-commit.backup" ]; then
    echo "ℹ️  Existing pre-commit hook backed up as pre-commit.backup"
fi

echo ""
echo "✅ Git hooks installation complete!"
echo ""
echo "Installed hooks:"
echo "  • pre-commit: Architectural compliance validation"
echo "  • commit-msg: Commit message format validation"
echo "  • pre-push: Final codebase validation"
echo ""
echo "These hooks are now MANDATORY for all commits and pushes."
echo "They will prevent architectural violations from entering the codebase."
echo ""
echo "To test the pre-commit hook:"
echo "  git add <file> && git commit -m 'test: verify governance hooks'"
echo ""