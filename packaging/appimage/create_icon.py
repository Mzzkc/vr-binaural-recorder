#!/usr/bin/env python3
"""
Simple script to create a basic icon for VR Binaural Recorder
Since we don't have a graphics designer, this creates a minimal but professional icon
"""

try:
    from PIL import Image, ImageDraw, ImageFont
    PIL_AVAILABLE = True
except ImportError:
    PIL_AVAILABLE = False

import os

def create_svg_icon():
    """Create a simple SVG icon"""
    svg_content = '''<?xml version="1.0" encoding="UTF-8"?>
<svg width="128" height="128" viewBox="0 0 128 128" xmlns="http://www.w3.org/2000/svg">
  <defs>
    <linearGradient id="bgGrad" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" style="stop-color:#2196F3"/>
      <stop offset="100%" style="stop-color:#1976D2"/>
    </linearGradient>
    <linearGradient id="fgGrad" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" style="stop-color:#FFF"/>
      <stop offset="100%" style="stop-color:#E3F2FD"/>
    </linearGradient>
  </defs>

  <!-- Background circle -->
  <circle cx="64" cy="64" r="60" fill="url(#bgGrad)" stroke="#0D47A1" stroke-width="2"/>

  <!-- VR Headset outline -->
  <path d="M24 54 Q24 42 36 42 L92 42 Q104 42 104 54 L104 74 Q104 86 92 86 L36 86 Q24 86 24 74 Z"
        fill="none" stroke="url(#fgGrad)" stroke-width="3"/>

  <!-- Lenses -->
  <circle cx="45" cy="64" r="12" fill="none" stroke="url(#fgGrad)" stroke-width="2"/>
  <circle cx="83" cy="64" r="12" fill="none" stroke="url(#fgGrad)" stroke-width="2"/>

  <!-- Audio waves -->
  <path d="M16 64 Q20 60 24 64 Q20 68 16 64" fill="none" stroke="#FF9800" stroke-width="2"/>
  <path d="M8 64 Q14 56 20 64 Q14 72 8 64" fill="none" stroke="#FF9800" stroke-width="2"/>
  <path d="M112 64 Q108 60 104 64 Q108 68 112 64" fill="none" stroke="#FF9800" stroke-width="2"/>
  <path d="M120 64 Q114 56 108 64 Q114 72 120 64" fill="none" stroke="#FF9800" stroke-width="2"/>

  <!-- Recording indicator -->
  <circle cx="98" cy="38" r="6" fill="#F44336"/>
  <circle cx="98" cy="38" r="3" fill="#FFEBEE"/>
</svg>'''

    return svg_content

def main():
    icon_dir = os.path.dirname(os.path.abspath(__file__))

    # Create SVG icon
    svg_path = os.path.join(icon_dir, "vr_binaural_recorder.svg")
    with open(svg_path, 'w') as f:
        f.write(create_svg_icon())
    print(f"Created SVG icon: {svg_path}")

    # Create PNG icon if PIL is available
    if PIL_AVAILABLE:
        try:
            # Convert SVG to PNG using system tools if available
            import subprocess
            png_path = os.path.join(icon_dir, "vr_binaural_recorder.png")

            # Try inkscape first, then other converters
            converters = [
                ["inkscape", "--export-png", png_path, "--export-width", "128", "--export-height", "128", svg_path],
                ["rsvg-convert", "-w", "128", "-h", "128", "-o", png_path, svg_path],
                ["convert", svg_path, "-resize", "128x128", png_path]
            ]

            for cmd in converters:
                try:
                    subprocess.run(cmd, check=True, capture_output=True)
                    print(f"Created PNG icon: {png_path}")
                    break
                except (subprocess.CalledProcessError, FileNotFoundError):
                    continue
            else:
                print("Could not convert SVG to PNG - no suitable converter found")
        except Exception as e:
            print(f"PNG conversion failed: {e}")
    else:
        print("PIL not available - only SVG icon created")

if __name__ == "__main__":
    main()