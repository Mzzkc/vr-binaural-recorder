#!/usr/bin/env python3
"""
🚀 Creative Cleanup Validation Tool
Quick and dirty validator to test our directory cleanup before committing!
"""

import os
import sys
import subprocess
import json
from pathlib import Path

class CleanupValidator:
    def __init__(self, project_root):
        self.root = Path(project_root)
        self.issues = []
        self.suggestions = []

    def validate_build_system(self):
        """Check if CMakeLists.txt still works after moves"""
        print("🔧 Testing build system...")

        cmake_file = self.root / "CMakeLists.txt"
        if not cmake_file.exists():
            self.issues.append("CMakeLists.txt missing!")
            return False

        # Quick syntax check - look for include paths
        content = cmake_file.read_text()
        if "include/" in content and not (self.root / "include").exists():
            self.issues.append("CMakeLists.txt references missing include/ directory")

        return True

    def check_config_consolidation(self):
        """Validate configuration file organization"""
        print("⚙️ Checking config consolidation...")

        config_files = list(self.root.glob("*config*.json"))
        if len(config_files) > 2:  # Allow main + example
            self.suggestions.append(f"Found {len(config_files)} config files - consider consolidating")

        return True

    def validate_documentation_flow(self):
        """Check if docs create a logical learning path"""
        print("📚 Validating documentation flow...")

        doc_files = list(self.root.glob("*.md"))
        critical_docs = ["README.md", "GETTING_STARTED.md", "API.md"]

        missing_docs = [doc for doc in critical_docs if not (self.root / doc).exists()]
        if missing_docs:
            self.suggestions.append(f"Consider creating: {', '.join(missing_docs)}")

        return True

    def test_import_paths(self):
        """Rapid test of include paths in source files"""
        print("🔍 Testing include paths...")

        src_files = list(self.root.glob("src/*.cpp")) + list(self.root.glob("include/*.h"))
        broken_includes = []

        for src_file in src_files:
            content = src_file.read_text()
            # Look for local includes
            for line in content.split('\n'):
                if '#include "' in line:
                    include_path = line.split('"')[1]
                    full_path = self.root / include_path
                    if not full_path.exists():
                        broken_includes.append(f"{src_file.name}: {include_path}")

        if broken_includes:
            self.issues.append(f"Broken includes found: {broken_includes}")

        return len(broken_includes) == 0

    def run_validation(self):
        """Run the full validation suite"""
        print("🚀 Running Creative Cleanup Validation...")
        print("=" * 50)

        tests = [
            self.validate_build_system,
            self.check_config_consolidation,
            self.validate_documentation_flow,
            self.test_import_paths
        ]

        passed = 0
        for test in tests:
            try:
                if test():
                    passed += 1
                    print("✅ PASS")
                else:
                    print("❌ FAIL")
            except Exception as e:
                print(f"⚠️ ERROR: {e}")

        print("\n" + "=" * 50)
        print(f"🎯 Results: {passed}/{len(tests)} tests passed")

        if self.issues:
            print("\n🚨 CRITICAL ISSUES:")
            for issue in self.issues:
                print(f"  - {issue}")

        if self.suggestions:
            print("\n💡 CREATIVE SUGGESTIONS:")
            for suggestion in self.suggestions:
                print(f"  - {suggestion}")

        return len(self.issues) == 0

if __name__ == "__main__":
    project_root = sys.argv[1] if len(sys.argv) > 1 else "."
    validator = CleanupValidator(project_root)

    success = validator.run_validation()
    sys.exit(0 if success else 1)