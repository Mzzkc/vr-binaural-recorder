#!/usr/bin/env python3
"""
Test HRTF Dataset Generator
Creates synthetic HRTF data for testing the VR Binaural Recorder
"""

import numpy as np
import scipy.io.wavfile as wavfile
import json
import os
from pathlib import Path

def create_synthetic_hrtf(duration_ms=5.0, sample_rate=44100):
    """Create a synthetic HRTF impulse response"""
    samples = int((duration_ms / 1000.0) * sample_rate)

    # Create a simple synthetic HRTF with:
    # - Initial impulse (direct path)
    # - Exponential decay (room acoustics simulation)
    # - Random phase variations (head/ear diffraction simulation)

    t = np.linspace(0, duration_ms/1000, samples)

    # Direct path impulse
    impulse = np.zeros(samples)
    impulse[0] = 1.0

    # Exponential decay envelope
    decay = np.exp(-t * 800)  # 800 Hz decay rate

    # Add some resonant frequencies (ear canal, head diffraction)
    freq_components = (
        0.3 * np.sin(2 * np.pi * 1000 * t) +  # 1 kHz
        0.2 * np.sin(2 * np.pi * 3000 * t) +  # 3 kHz (ear canal)
        0.1 * np.sin(2 * np.pi * 6000 * t) +  # 6 kHz (pinna)
        0.05 * np.sin(2 * np.pi * 8000 * t)   # 8 kHz (head diffraction)
    )

    # Apply frequency content and decay
    hrtf = (impulse + 0.1 * freq_components) * decay

    # Normalize and convert to 16-bit
    hrtf = hrtf / np.max(np.abs(hrtf)) * 0.8
    return (hrtf * 32767).astype(np.int16)

def create_test_dataset(output_dir):
    """Create a complete test HRTF dataset"""
    output_path = Path(output_dir)
    output_path.mkdir(exist_ok=True)

    elevations = [-40, -30, -20, -10, 0, 10, 20, 30, 40, 50, 60, 70, 80, 90]
    azimuths_per_elevation = {
        -40: range(-80, 85, 10),
        -30: range(-80, 85, 10),
        -20: range(-80, 85, 10),
        -10: range(-90, 95, 5),
        0: range(-90, 95, 5),
        10: range(-90, 95, 5),
        20: range(-90, 95, 5),
        30: range(-80, 85, 10),
        40: range(-80, 85, 10),
        50: range(-80, 85, 10),
        60: range(-60, 65, 10),
        70: range(-60, 65, 10),
        80: range(-60, 65, 10),
        90: [0]  # Only one azimuth at the top
    }

    sample_rate = 44100
    dataset_info = {
        "name": "Synthetic Test HRTF Dataset",
        "description": "Generated synthetic HRTF data for testing",
        "sample_rate": sample_rate,
        "format": "wav",
        "measurements": []
    }

    total_files = 0

    for elevation in elevations:
        elev_dir = output_path / f"elev{elevation:+d}"
        elev_dir.mkdir(exist_ok=True)

        for azimuth in azimuths_per_elevation[elevation]:
            # Create left and right ear responses
            for ear in ['L', 'R']:
                # Add slight differences between ears for realistic stereo effect
                if ear == 'L':
                    phase_shift = 0
                    amplitude_factor = 1.0
                else:
                    # Right ear gets slightly different timing and amplitude
                    phase_shift = np.sin(np.radians(azimuth)) * 0.0001  # Small ITD
                    amplitude_factor = 0.95 + 0.1 * np.cos(np.radians(azimuth))

                # Generate HRTF
                hrtf = create_synthetic_hrtf()

                # Apply ear-specific modifications
                if phase_shift != 0:
                    # Simple phase shift simulation
                    shift_samples = int(phase_shift * sample_rate)
                    if shift_samples > 0:
                        hrtf = np.concatenate([np.zeros(shift_samples), hrtf[:-shift_samples]])
                    elif shift_samples < 0:
                        hrtf = np.concatenate([hrtf[-shift_samples:], np.zeros(-shift_samples)])

                hrtf = (hrtf * amplitude_factor).astype(np.int16)

                # Save as WAV file
                filename = f"H{azimuth:+04d}e{elevation:+03d}{ear.lower()}.wav"
                filepath = elev_dir / filename
                wavfile.write(filepath, sample_rate, hrtf)

                # Add to dataset info
                dataset_info["measurements"].append({
                    "elevation": elevation,
                    "azimuth": azimuth,
                    "ear": ear,
                    "filename": str(filepath.relative_to(output_path))
                })

                total_files += 1

    # Save dataset metadata
    with open(output_path / "dataset_info.json", 'w') as f:
        json.dump(dataset_info, f, indent=2)

    print(f"Created synthetic HRTF dataset with {total_files} files")
    print(f"Saved to: {output_path}")
    print(f"Elevations: {elevations}")
    print(f"Total measurements: {len(dataset_info['measurements']) // 2} (stereo pairs)")

def main():
    import argparse
    parser = argparse.ArgumentParser(description="Create synthetic HRTF test dataset")
    parser.add_argument("--output", default="hrtf_data/synthetic_test",
                       help="Output directory for test dataset")

    args = parser.parse_args()
    create_test_dataset(args.output)

if __name__ == "__main__":
    main()