#!/usr/bin/env python3
"""
HRTF Dataset Download and Setup Tool
Downloads and configures HRTF datasets for VR Binaural Recorder
"""

import os
import sys
import json
import urllib.request
import urllib.error
import zipfile
import tarfile
import shutil
from pathlib import Path
import argparse

class HRTFDatasetDownloader:
    def __init__(self, project_root):
        self.project_root = Path(project_root)
        self.hrtf_data_dir = self.project_root / "hrtf_data"
        self.temp_dir = self.project_root / "temp_downloads"

        # Dataset configurations
        self.datasets = {
            "mit_kemar_compact": {
                "name": "MIT KEMAR HRTF Dataset (Compact)",
                "description": "Compact KEMAR dummy head measurements from MIT",
                "urls": [
                    "http://sound.media.mit.edu/resources/KEMAR/compact.zip"
                ],
                "extract_dir": "mit_kemar_compact",
                "file_format": "zip"
            },
            "mit_kemar_full": {
                "name": "MIT KEMAR HRTF Dataset (Full)",
                "description": "Full KEMAR dummy head measurements from MIT",
                "urls": [
                    "http://sound.media.mit.edu/resources/KEMAR/full.zip"
                ],
                "extract_dir": "mit_kemar_full",
                "file_format": "zip"
            },
            "cipic_github": {
                "name": "CIPIC HRTF Dataset (GitHub Mirror)",
                "description": "CIPIC database from GitHub mirror",
                "urls": [
                    "https://github.com/amini-allight/cipic-hrtf-database/archive/refs/heads/master.zip"
                ],
                "extract_dir": "cipic",
                "file_format": "zip"
            },
            "cipic_sofa": {
                "name": "CIPIC HRTF Dataset (SOFA Format)",
                "description": "CIPIC database in SOFA format",
                "urls": [
                    "https://raw.githubusercontent.com/TWOEARS/SOFA/master/HRTFs/CIPIC/subject_003.sofa",
                    "https://raw.githubusercontent.com/TWOEARS/SOFA/master/HRTFs/CIPIC/subject_008.sofa",
                    "https://raw.githubusercontent.com/TWOEARS/SOFA/master/HRTFs/CIPIC/subject_010.sofa"
                ],
                "extract_dir": "cipic_sofa",
                "file_format": "sofa"
            },
            "sofa_sample": {
                "name": "SOFA Sample Dataset",
                "description": "Sample SOFA format HRTF files",
                "urls": [
                    "https://www.sofaconventions.org/data/database/clubfritz/ClubFritz1_44K.sofa",
                    "https://www.sofaconventions.org/data/database/clubfritz/ClubFritz2_44K.sofa"
                ],
                "extract_dir": "sofa_samples",
                "file_format": "sofa"
            }
        }

    def setup_directories(self):
        """Create necessary directories"""
        self.hrtf_data_dir.mkdir(exist_ok=True)
        self.temp_dir.mkdir(exist_ok=True)
        print(f"Created directories: {self.hrtf_data_dir}, {self.temp_dir}")

    def download_file(self, url, destination):
        """Download a file with progress indication"""
        try:
            print(f"Downloading: {url}")
            with urllib.request.urlopen(url) as response:
                total_size = int(response.headers.get('Content-Length', 0))
                downloaded = 0

                with open(destination, 'wb') as f:
                    while True:
                        chunk = response.read(8192)
                        if not chunk:
                            break
                        f.write(chunk)
                        downloaded += len(chunk)

                        if total_size > 0:
                            percent = (downloaded / total_size) * 100
                            print(f"\rProgress: {percent:.1f}%", end='', flush=True)

                print(f"\nDownloaded: {destination}")
                return True

        except urllib.error.URLError as e:
            print(f"Failed to download {url}: {e}")
            return False
        except Exception as e:
            print(f"Error downloading {url}: {e}")
            return False

    def extract_archive(self, archive_path, extract_to):
        """Extract various archive formats"""
        try:
            if archive_path.suffix.lower() == '.zip':
                with zipfile.ZipFile(archive_path, 'r') as zip_ref:
                    zip_ref.extractall(extract_to)
            elif archive_path.name.endswith('.tar.Z'):
                # Handle .tar.Z files (need uncompress first)
                import subprocess
                uncompressed_path = archive_path.with_suffix('')
                subprocess.run(['uncompress', str(archive_path)], check=True)
                with tarfile.open(uncompressed_path, 'r') as tar_ref:
                    tar_ref.extractall(extract_to)
            elif archive_path.suffix.lower() in ['.tar', '.gz']:
                with tarfile.open(archive_path, 'r:*') as tar_ref:
                    tar_ref.extractall(extract_to)
            else:
                # For .sofa files, just copy them directly
                shutil.copy2(archive_path, extract_to)

            print(f"Extracted: {archive_path} -> {extract_to}")
            return True

        except Exception as e:
            print(f"Failed to extract {archive_path}: {e}")
            return False

    def download_dataset(self, dataset_key):
        """Download and setup a specific dataset"""
        if dataset_key not in self.datasets:
            print(f"Unknown dataset: {dataset_key}")
            return False

        dataset = self.datasets[dataset_key]
        print(f"\n=== Downloading {dataset['name']} ===")
        print(f"Description: {dataset['description']}")

        dataset_dir = self.hrtf_data_dir / dataset['extract_dir']
        dataset_dir.mkdir(exist_ok=True)

        success_count = 0
        total_files = len(dataset['urls'])

        for i, url in enumerate(dataset['urls']):
            filename = Path(url).name
            temp_file = self.temp_dir / filename

            print(f"\n[{i+1}/{total_files}] Processing: {filename}")

            if self.download_file(url, temp_file):
                if self.extract_archive(temp_file, dataset_dir):
                    success_count += 1
                    # Clean up temp file after successful extraction
                    temp_file.unlink(missing_ok=True)
                else:
                    print(f"Failed to extract {filename}")
            else:
                print(f"Failed to download {filename}")

        print(f"\n{dataset['name']} download complete: {success_count}/{total_files} files successful")
        return success_count > 0

    def create_config_template(self):
        """Create a configuration template for the downloaded datasets"""
        config_template = {
            "hrtf": {
                "datasets": {
                    "mit_kemar": {
                        "path": "hrtf_data/mit_kemar",
                        "format": "raw",
                        "sample_rate": 44100,
                        "description": "MIT KEMAR dummy head measurements"
                    },
                    "cipic": {
                        "path": "hrtf_data/cipic",
                        "format": "mat",
                        "sample_rate": 44100,
                        "description": "CIPIC database with 45 subjects"
                    },
                    "sofa_samples": {
                        "path": "hrtf_data/sofa_samples",
                        "format": "sofa",
                        "sample_rate": 44100,
                        "description": "SOFA format sample datasets"
                    }
                },
                "default_dataset": "mit_kemar",
                "processing": {
                    "filter_length": 512,
                    "crossfade_samples": 64,
                    "interpolation": "linear"
                }
            }
        }

        config_path = self.project_root / "hrtf_datasets_config.json"
        with open(config_path, 'w') as f:
            json.dump(config_template, f, indent=2)

        print(f"\nCreated dataset configuration: {config_path}")

    def list_datasets(self):
        """List available datasets"""
        print("\nAvailable HRTF Datasets:")
        print("=" * 50)
        for key, dataset in self.datasets.items():
            print(f"Key: {key}")
            print(f"Name: {dataset['name']}")
            print(f"Description: {dataset['description']}")
            print(f"Files: {len(dataset['urls'])}")
            print("-" * 30)

    def cleanup(self):
        """Clean up temporary files"""
        if self.temp_dir.exists():
            shutil.rmtree(self.temp_dir)
        print("Cleaned up temporary files")

def main():
    parser = argparse.ArgumentParser(description="Download and setup HRTF datasets")
    parser.add_argument("--dataset", choices=['mit_kemar_compact', 'mit_kemar_full', 'cipic_github', 'cipic_sofa', 'sofa_sample', 'all'],
                       help="Dataset to download")
    parser.add_argument("--list", action="store_true", help="List available datasets")
    parser.add_argument("--project-root", default=".", help="Project root directory")

    args = parser.parse_args()

    downloader = HRTFDatasetDownloader(args.project_root)

    if args.list:
        downloader.list_datasets()
        return

    if not args.dataset:
        print("Please specify a dataset to download or use --list to see available options")
        return

    try:
        downloader.setup_directories()

        if args.dataset == 'all':
            for dataset_key in downloader.datasets.keys():
                downloader.download_dataset(dataset_key)
        else:
            downloader.download_dataset(args.dataset)

        downloader.create_config_template()
        print("\n=== HRTF Dataset Setup Complete ===")
        print("Next steps:")
        print("1. Review hrtf_datasets_config.json")
        print("2. Update your main config to reference the datasets")
        print("3. Test the HRTF processor with the new data")

    except KeyboardInterrupt:
        print("\nDownload interrupted by user")
    except Exception as e:
        print(f"Error during setup: {e}")
    finally:
        downloader.cleanup()

if __name__ == "__main__":
    main()