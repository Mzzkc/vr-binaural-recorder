#!/usr/bin/env python3
"""
VR Binaural Recorder - System Validation and Setup
Validates system requirements and configures optimal settings
"""

import os
import sys
import json
import platform
import subprocess
import ctypes
import shutil
import argparse
from pathlib import Path
from typing import Dict, List, Tuple, Optional
import logging

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format='[%(levelname)s] %(message)s'
)
logger = logging.getLogger(__name__)

class SystemValidator:
    """Validates system requirements for VR Binaural Recorder"""
    
    def __init__(self):
        self.platform = platform.system()
        self.arch = platform.machine()
        self.results = {}
        self.warnings = []
        self.errors = []
        
    def validate_all(self) -> bool:
        """Run all validation checks"""
        logger.info("Starting system validation...")
        
        checks = [
            self.check_os,
            self.check_architecture,
            self.check_memory,
            self.check_cpu,
            self.check_gpu,
            self.check_audio,
            self.check_vr_runtime,
            self.check_dependencies,
            self.check_permissions,
            self.check_network
        ]
        
        for check in checks:
            try:
                check()
            except Exception as e:
                logger.error(f"Check failed: {e}")
                self.errors.append(str(e))
        
        return len(self.errors) == 0
    
    def check_os(self):
        """Check operating system compatibility"""
        logger.info("Checking OS...")
        
        if self.platform == 'Windows':
            version = platform.version()
            major = int(version.split('.')[0])
            if major < 10:
                self.errors.append("Windows 10 or later required")
            else:
                self.results['os'] = f"Windows {version}"
                logger.info(f"  ✓ Windows {version}")
                
        elif self.platform == 'Linux':
            self.results['os'] = platform.platform()
            logger.info(f"  ✓ {platform.platform()}")
            
        else:
            self.errors.append(f"Unsupported OS: {self.platform}")
    
    def check_architecture(self):
        """Check CPU architecture"""
        logger.info("Checking architecture...")
        
        if 'x86_64' in self.arch or 'AMD64' in self.arch:
            self.results['arch'] = self.arch
            logger.info(f"  ✓ {self.arch}")
        else:
            self.errors.append(f"x86_64 architecture required, found: {self.arch}")
    
    def check_memory(self):
        """Check available memory"""
        logger.info("Checking memory...")
        
        if self.platform == 'Windows':
            kernel32 = ctypes.windll.kernel32
            c_ulong = ctypes.c_ulong
            
            class MEMORYSTATUS(ctypes.Structure):
                _fields_ = [
                    ('dwLength', c_ulong),
                    ('dwMemoryLoad', c_ulong),
                    ('dwTotalPhys', c_ulong),
                    ('dwAvailPhys', c_ulong),
                    ('dwTotalPageFile', c_ulong),
                    ('dwAvailPageFile', c_ulong),
                    ('dwTotalVirtual', c_ulong),
                    ('dwAvailVirtual', c_ulong),
                ]
            
            memoryStatus = MEMORYSTATUS()
            memoryStatus.dwLength = ctypes.sizeof(MEMORYSTATUS)
            kernel32.GlobalMemoryStatus(ctypes.byref(memoryStatus))
            
            total_gb = memoryStatus.dwTotalPhys / (1024**3)
            avail_gb = memoryStatus.dwAvailPhys / (1024**3)
            
        else:  # Linux/Unix
            import resource
            import psutil
            
            memory = psutil.virtual_memory()
            total_gb = memory.total / (1024**3)
            avail_gb = memory.available / (1024**3)
        
        self.results['memory'] = {
            'total': f"{total_gb:.1f} GB",
            'available': f"{avail_gb:.1f} GB"
        }
        
        if total_gb < 8:
            self.warnings.append(f"Low memory: {total_gb:.1f} GB (8 GB recommended)")
        
        logger.info(f"  ✓ Total: {total_gb:.1f} GB, Available: {avail_gb:.1f} GB")
    
    def check_cpu(self):
        """Check CPU capabilities"""
        logger.info("Checking CPU...")
        
        try:
            import cpuinfo
            info = cpuinfo.get_cpu_info()
            
            self.results['cpu'] = {
                'brand': info['brand_raw'],
                'cores': info['count'],
                'features': []
            }
            
            # Check for required SIMD features
            required_features = ['sse2', 'sse3', 'ssse3', 'sse4_1', 'sse4_2']
            recommended_features = ['avx', 'avx2']
            
            for feature in required_features:
                if feature in info['flags']:
                    self.results['cpu']['features'].append(feature.upper())
                else:
                    self.errors.append(f"CPU missing required feature: {feature.upper()}")
            
            for feature in recommended_features:
                if feature in info['flags']:
                    self.results['cpu']['features'].append(feature.upper())
                else:
                    self.warnings.append(f"CPU missing recommended feature: {feature.upper()}")
            
            logger.info(f"  ✓ {info['brand_raw']}")
            logger.info(f"    Cores: {info['count']}")
            logger.info(f"    SIMD: {', '.join(self.results['cpu']['features'])}")
            
        except ImportError:
            self.warnings.append("cpuinfo module not available - install with: pip install py-cpuinfo")
            
            # Fallback to basic check
            import multiprocessing
            cores = multiprocessing.cpu_count()
            self.results['cpu'] = {'cores': cores}
            logger.info(f"  ✓ CPU cores: {cores}")
    
    def check_gpu(self):
        """Check GPU and VR capability"""
        logger.info("Checking GPU...")
        
        if self.platform == 'Windows':
            try:
                # Use WMI to get GPU info
                import wmi
                c = wmi.WMI()
                
                gpus = []
                for gpu in c.Win32_VideoController():
                    gpus.append({
                        'name': gpu.Name,
                        'driver': gpu.DriverVersion,
                        'memory': f"{int(gpu.AdapterRAM or 0) / (1024**3):.1f} GB" if gpu.AdapterRAM else 'Unknown'
                    })
                
                if gpus:
                    self.results['gpu'] = gpus
                    for gpu in gpus:
                        logger.info(f"  ✓ {gpu['name']}")
                        logger.info(f"    Driver: {gpu['driver']}")
                        logger.info(f"    Memory: {gpu['memory']}")
                else:
                    self.warnings.append("No GPU detected")
                    
            except ImportError:
                # Fallback to nvidia-smi if available
                if shutil.which('nvidia-smi'):
                    result = subprocess.run(['nvidia-smi', '--query-gpu=name,driver_version,memory.total',
                                           '--format=csv,noheader'], capture_output=True, text=True)
                    if result.returncode == 0:
                        gpu_info = result.stdout.strip().split(', ')
                        self.results['gpu'] = [{
                            'name': gpu_info[0],
                            'driver': gpu_info[1],
                            'memory': gpu_info[2]
                        }]
                        logger.info(f"  ✓ {gpu_info[0]}")
                else:
                    self.warnings.append("GPU detection not available")
        else:
            # Linux GPU detection
            if shutil.which('lspci'):
                result = subprocess.run(['lspci', '-nn'], capture_output=True, text=True)
                vga_lines = [line for line in result.stdout.split('\n') if 'VGA' in line or '3D' in line]
                if vga_lines:
                    self.results['gpu'] = vga_lines
                    for line in vga_lines:
                        logger.info(f"  ✓ {line}")
            else:
                self.warnings.append("GPU detection not available (lspci not found)")
    
    def check_audio(self):
        """Check audio subsystem"""
        logger.info("Checking audio...")
        
        audio_systems = []
        
        if self.platform == 'Windows':
            # Check for WASAPI
            audio_systems.append('WASAPI')
            
            # Check for ASIO
            asio_path = Path("C:/Program Files/Common Files/Steinberg/ASIO")
            if asio_path.exists():
                audio_systems.append('ASIO')
            
            # Check for virtual cable
            import winreg
            try:
                key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,
                                    r"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall")
                for i in range(winreg.QueryInfoKey(key)[0]):
                    subkey_name = winreg.EnumKey(key, i)
                    subkey = winreg.OpenKey(key, subkey_name)
                    try:
                        name = winreg.QueryValueEx(subkey, "DisplayName")[0]
                        if 'Virtual Cable' in name or 'VB-Cable' in name:
                            audio_systems.append('Virtual Cable')
                            break
                    except:
                        pass
                    winreg.CloseKey(subkey)
                winreg.CloseKey(key)
            except:
                pass
                
        else:  # Linux
            # Check for ALSA
            if shutil.which('aplay'):
                audio_systems.append('ALSA')
            
            # Check for JACK
            if shutil.which('jack_control'):
                audio_systems.append('JACK')
            
            # Check for PulseAudio
            if shutil.which('pactl'):
                audio_systems.append('PulseAudio')
        
        if audio_systems:
            self.results['audio'] = audio_systems
            logger.info(f"  ✓ Available: {', '.join(audio_systems)}")
        else:
            self.errors.append("No audio system detected")
        
        if 'Virtual Cable' not in audio_systems and 'JACK' not in audio_systems:
            self.warnings.append("Virtual audio cable not detected - install VB-Cable for best results")
    
    def check_vr_runtime(self):
        """Check for VR runtime (SteamVR/OpenXR)"""
        logger.info("Checking VR runtime...")
        
        vr_found = False
        
        # Check for SteamVR
        if self.platform == 'Windows':
            steam_paths = [
                Path(os.environ.get('PROGRAMFILES(X86)', '')) / 'Steam',
                Path(os.environ.get('PROGRAMFILES', '')) / 'Steam',
                Path('C:/Program Files (x86)/Steam'),
                Path('C:/Program Files/Steam')
            ]
            
            for steam_path in steam_paths:
                steamvr = steam_path / 'steamapps/common/SteamVR'
                if steamvr.exists():
                    self.results['vr'] = {'steamvr': str(steamvr)}
                    logger.info(f"  ✓ SteamVR found: {steamvr}")
                    vr_found = True
                    break
                    
        else:  # Linux
            home = Path.home()
            steam_paths = [
                home / '.steam/steam/steamapps/common/SteamVR',
                home / '.local/share/Steam/steamapps/common/SteamVR'
            ]
            
            for steamvr in steam_paths:
                if steamvr.exists():
                    self.results['vr'] = {'steamvr': str(steamvr)}
                    logger.info(f"  ✓ SteamVR found: {steamvr}")
                    vr_found = True
                    break
        
        # Check for OpenXR
        if self.platform == 'Windows':
            openxr_path = Path(os.environ.get('SystemRoot', 'C:/Windows')) / 'System32/openxr_loader.dll'
            if openxr_path.exists():
                if 'vr' not in self.results:
                    self.results['vr'] = {}
                self.results['vr']['openxr'] = str(openxr_path)
                logger.info(f"  ✓ OpenXR found")
                vr_found = True
        
        if not vr_found:
            self.errors.append("No VR runtime found - install SteamVR")
    
    def check_dependencies(self):
        """Check runtime dependencies"""
        logger.info("Checking dependencies...")
        
        deps_ok = True
        
        if self.platform == 'Windows':
            # Check for Visual C++ Redistributables
            vcredist_keys = [
                r"SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64",
                r"SOFTWARE\WOW6432Node\Microsoft\VisualStudio\14.0\VC\Runtimes\x64"
            ]
            
            vcredist_found = False
            import winreg
            
            for key_path in vcredist_keys:
                try:
                    key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, key_path)
                    installed = winreg.QueryValueEx(key, "Installed")[0]
                    if installed == 1:
                        vcredist_found = True
                        version = winreg.QueryValueEx(key, "Version")[0]
                        logger.info(f"  ✓ Visual C++ Redistributable: {version}")
                        break
                    winreg.CloseKey(key)
                except:
                    pass
            
            if not vcredist_found:
                self.errors.append("Visual C++ Redistributables not installed")
                deps_ok = False
        
        # Check for OpenVR API
        openvr_dll = Path('./openvr_api.dll' if self.platform == 'Windows' else './libopenvr_api.so')
        if openvr_dll.exists():
            logger.info(f"  ✓ OpenVR API: {openvr_dll}")
        else:
            self.warnings.append(f"OpenVR API not found: {openvr_dll}")
        
        return deps_ok
    
    def check_permissions(self):
        """Check file system permissions"""
        logger.info("Checking permissions...")
        
        # Check log directory
        log_dir = Path('./logs')
        try:
            log_dir.mkdir(exist_ok=True)
            test_file = log_dir / 'test.txt'
            test_file.write_text('test')
            test_file.unlink()
            logger.info(f"  ✓ Write permission: {log_dir}")
        except Exception as e:
            self.errors.append(f"No write permission for logs: {e}")
        
        # Check config file
        config_file = Path('./vr_binaural_config.json')
        if config_file.exists():
            if os.access(config_file, os.R_OK):
                logger.info(f"  ✓ Config readable: {config_file}")
            else:
                self.errors.append(f"Cannot read config: {config_file}")
    
    def check_network(self):
        """Check network settings (for updates/telemetry)"""
        logger.info("Checking network...")
        
        # Check firewall status
        if self.platform == 'Windows':
            result = subprocess.run(['netsh', 'advfirewall', 'show', 'currentprofile'],
                                  capture_output=True, text=True)
            if 'State                                 ON' in result.stdout:
                self.warnings.append("Windows Firewall is ON - may need exception for VR communication")
        
        logger.info("  ✓ Network check complete")
    
    def generate_report(self) -> str:
        """Generate validation report"""
        report = []
        report.append("=" * 60)
        report.append("VR BINAURAL RECORDER - SYSTEM VALIDATION REPORT")
        report.append("=" * 60)
        report.append("")
        
        # System Info
        report.append("SYSTEM INFORMATION:")
        report.append("-" * 40)
        for key, value in self.results.items():
            if isinstance(value, dict):
                report.append(f"{key.upper()}:")
                for k, v in value.items():
                    report.append(f"  {k}: {v}")
            elif isinstance(value, list):
                report.append(f"{key.upper()}: {', '.join(map(str, value))}")
            else:
                report.append(f"{key.upper()}: {value}")
        report.append("")
        
        # Warnings
        if self.warnings:
            report.append("WARNINGS:")
            report.append("-" * 40)
            for warning in self.warnings:
                report.append(f"⚠ {warning}")
            report.append("")
        
        # Errors
        if self.errors:
            report.append("ERRORS:")
            report.append("-" * 40)
            for error in self.errors:
                report.append(f"✗ {error}")
            report.append("")
        
        # Status
        report.append("VALIDATION STATUS:")
        report.append("-" * 40)
        if self.errors:
            report.append("✗ FAILED - Resolve errors before running")
        elif self.warnings:
            report.append("⚠ PASSED WITH WARNINGS - May experience reduced functionality")
        else:
            report.append("✓ PASSED - System ready for VR Binaural Recorder")
        
        report.append("=" * 60)
        
        return "\n".join(report)
    
    def generate_config_recommendations(self) -> Dict:
        """Generate recommended configuration based on system"""
        config = {}
        
        # Audio recommendations
        if 'ASIO' in self.results.get('audio', []):
            config['audio'] = {
                'api': 'asio',
                'bufferSize': 64
            }
        elif 'WASAPI' in self.results.get('audio', []):
            config['audio'] = {
                'api': 'wasapi',
                'exclusive': True,
                'bufferSize': 128
            }
        else:
            config['audio'] = {
                'api': 'auto',
                'bufferSize': 256
            }
        
        # Performance recommendations
        cpu_info = self.results.get('cpu', {})
        if 'AVX2' in cpu_info.get('features', []):
            config['performance'] = {
                'simdLevel': 'avx2'
            }
        elif 'AVX' in cpu_info.get('features', []):
            config['performance'] = {
                'simdLevel': 'avx'
            }
        else:
            config['performance'] = {
                'simdLevel': 'sse2'
            }
        
        # VR recommendations
        if 'vr' in self.results:
            config['vr'] = {
                'trackingRate': 90
            }
        
        return config


def main():
    parser = argparse.ArgumentParser(description='VR Binaural Recorder System Validator')
    parser.add_argument('--json', action='store_true', help='Output results as JSON')
    parser.add_argument('--config', action='store_true', help='Generate recommended config')
    parser.add_argument('--fix', action='store_true', help='Attempt to fix issues')
    parser.add_argument('--output', type=str, help='Output file for report')
    
    args = parser.parse_args()
    
    validator = SystemValidator()
    success = validator.validate_all()
    
    if args.json:
        output = {
            'success': success,
            'results': validator.results,
            'warnings': validator.warnings,
            'errors': validator.errors
        }
        print(json.dumps(output, indent=2))
    else:
        report = validator.generate_report()
        print(report)
        
        if args.output:
            Path(args.output).write_text(report)
            print(f"\nReport saved to: {args.output}")
    
    if args.config:
        config = validator.generate_config_recommendations()
        config_path = Path('recommended_config.json')
        config_path.write_text(json.dumps(config, indent=2))
        print(f"\nRecommended configuration saved to: {config_path}")
    
    if args.fix and validator.errors:
        print("\nAttempting to fix issues...")
        # Add fix logic here
    
    return 0 if success else 1


if __name__ == '__main__':
    sys.exit(main())