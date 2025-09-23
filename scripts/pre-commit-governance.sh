#!/bin/bash
# Pre-commit governance hook for architectural compliance
# MANDATORY: Prevents recording functionality and architectural violations

echo "🔍 Running architectural compliance checks..."

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Flag for overall success
CHECKS_PASSED=true

# Function to report violations
report_violation() {
    echo -e "${RED}❌ ARCHITECTURAL VIOLATION:${NC} $1"
    CHECKS_PASSED=false
}

# Function to report warnings
report_warning() {
    echo -e "${YELLOW}⚠️  WARNING:${NC} $1"
}

# Function to report success
report_success() {
    echo -e "${GREEN}✅${NC} $1"
}

echo ""
echo "=== ARCHITECTURAL COMPLIANCE CHECKS ==="

# Check 1: Recording functionality detection
echo "Checking for recording functionality..."
if git diff --cached --name-only | xargs grep -l "Recording\|RecordingSession\|StartRecord\|StopRecord\|PauseRecord" 2>/dev/null; then
    report_violation "Recording functionality detected in staged files"
    echo "  This violates core architectural principle: audio routing, not recording"
    echo "  Remove all recording code before committing"
else
    report_success "No recording functionality detected"
fi

# Check 2: File I/O in audio processing code
echo "Checking for file I/O in audio processing..."
if git diff --cached | grep -E "fopen|ofstream|WriteFile|iostream|std::ifstream|std::ofstream" -- "*/audio/*" "*audio*" 2>/dev/null; then
    report_violation "File I/O detected in audio processing code"
    echo "  This violates real-time safety requirements"
    echo "  Audio processing must be file I/O free"
else
    report_success "No file I/O in audio processing code"
fi

# Check 3: Memory allocation in real-time code
echo "Checking for memory allocation in audio callbacks..."
if git diff --cached | grep -E "new |malloc|calloc|make_unique|make_shared" -- "*callback*" "*process*" "*audio*" 2>/dev/null; then
    if git diff --cached | grep -E "new |malloc|calloc|make_unique|make_shared" -- "*callback*" "*process*" | grep -v "//.*new\|/\*.*new" 2>/dev/null; then
        report_violation "Memory allocation detected in real-time audio code"
        echo "  This violates real-time safety requirements"
        echo "  Use pre-allocated buffers or stack allocation"
    fi
fi

# Check 4: Configuration parameter count
echo "Checking configuration complexity..."
if [ -f "config/vr_binaural_config.json" ]; then
    CONFIG_PARAMS=$(grep -o '"[^"]*"\s*:' config/vr_binaural_config.json | wc -l)
    if [ "$CONFIG_PARAMS" -gt 25 ]; then
        report_violation "Too many configuration parameters ($CONFIG_PARAMS > 25)"
        echo "  Target: <20 parameters, Maximum: 25"
        echo "  Simplify configuration to essential parameters only"
    elif [ "$CONFIG_PARAMS" -gt 20 ]; then
        report_warning "Configuration parameter count approaching limit ($CONFIG_PARAMS/25)"
        echo "  Consider simplification before adding more parameters"
    else
        report_success "Configuration complexity within limits ($CONFIG_PARAMS/25)"
    fi
fi

# Check 5: Component size validation
echo "Checking component size..."
LARGE_FILES=()
while IFS= read -r -d '' file; do
    if [[ "$file" == *.cpp ]] || [[ "$file" == *.h ]] || [[ "$file" == *.hpp ]]; then
        LINES=$(wc -l < "$file" 2>/dev/null || echo 0)
        if [ "$LINES" -gt 1000 ]; then
            LARGE_FILES+=("$file ($LINES lines)")
        fi
    fi
done < <(git diff --cached --name-only -z)

if [ ${#LARGE_FILES[@]} -gt 0 ]; then
    report_warning "Large components detected (>1000 lines):"
    for file in "${LARGE_FILES[@]}"; do
        echo "    $file"
    done
    echo "  Consider breaking into smaller, focused components"
else
    report_success "All components within reasonable size limits"
fi

# Check 6: Forbidden terms in new code
echo "Checking for forbidden terminology..."
FORBIDDEN_TERMS=("recording" "recorder" "take" "session" "playback" "file_output")
FORBIDDEN_FOUND=false

for term in "${FORBIDDEN_TERMS[@]}"; do
    if git diff --cached | grep -i "$term" | grep -v "//.*$term\|/\*.*$term" 2>/dev/null | head -3; then
        if [ "$FORBIDDEN_FOUND" = false ]; then
            report_violation "Forbidden terminology detected in new code"
            FORBIDDEN_FOUND=true
        fi
        echo "  Found: $term (conflicts with audio routing focus)"
    fi
done

if [ "$FORBIDDEN_FOUND" = false ]; then
    report_success "No forbidden terminology detected"
fi

# Check 7: Industry standard API usage
echo "Checking for custom implementations of platform APIs..."
CUSTOM_IMPL_WARNING=false

if git diff --cached | grep -E "class.*AudioDevice|class.*VRDevice|class.*AudioDriver" 2>/dev/null; then
    report_warning "Custom device implementations detected"
    echo "  Prefer using platform-provided APIs (WASAPI, PulseAudio, Core Audio, OpenVR)"
    CUSTOM_IMPL_WARNING=true
fi

if [ "$CUSTOM_IMPL_WARNING" = false ]; then
    report_success "Using standard platform APIs"
fi

echo ""
echo "=== ARCHITECTURAL COMPLIANCE SUMMARY ==="

if [ "$CHECKS_PASSED" = true ]; then
    echo -e "${GREEN}✅ ALL CHECKS PASSED${NC}"
    echo "Commit approved for architectural compliance"
    echo ""
    exit 0
else
    echo -e "${RED}❌ COMPLIANCE VIOLATIONS DETECTED${NC}"
    echo ""
    echo "COMMIT REJECTED - Fix violations before committing:"
    echo "1. Remove any recording functionality"
    echo "2. Eliminate file I/O from audio processing code"
    echo "3. Use pre-allocated memory in real-time code"
    echo "4. Simplify configuration if parameter count exceeded"
    echo "5. Use appropriate terminology (routing, not recording)"
    echo ""
    echo "For help, see: DEVELOPMENT_GOVERNANCE_PROCEDURES.md"
    echo ""
    exit 1
fi