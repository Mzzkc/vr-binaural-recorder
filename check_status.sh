#!/bin/bash
# Quick status check for project

echo "=== VR Binaural Recorder Status ==="
echo ""

# Check current step
echo "Current Step:"
grep "Active Step:" PROJECT_STATUS.md | sed 's/.*: /  /'
echo ""

# Check completed steps
echo "Completed Steps:"
grep "COMPLETED" docs/completed_steps.log | tail -5 | sed 's/^/  /'
echo ""

# Check for blockers
echo "Blockers:"
if grep -q "None" PROJECT_STATUS.md; then
    echo "  None"
else
    grep "BLOCKED:" *.md 2>/dev/null | sed 's/^/  /' || echo "  None"
fi
echo ""

# Check build status
echo "Build Status:"
if [ -d "build" ] && [ -f "build/Makefile" ]; then
    echo "  ✓ Build system configured"
else
    echo "  ✗ Build system not configured"
fi

if [ -f "build/vr_binaural_recorder" ] || [ -f "build/vr_binaural_recorder.exe" ]; then
    echo "  ✓ Executable built"
else  
    echo "  ✗ Executable not built"
fi
