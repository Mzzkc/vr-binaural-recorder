# Win VR Setup→ASMRtist
VR Binaural Recorder install+testing

Quick start:Transform any mic→spatialized audio based on VR position | Time:15-20min(w/diagnostics)

## 🔧DIAGNOSTIC-FIRST SETUP

**CRITICAL:**Test BEFORE recording sessions!

**Phase1:Pre-Install(5min)**
```batch
REM check_conflicts.bat
@echo off
echo [1]VR runtimes...
if exist"C:\Program Files (x86)\Steam\steamapps\common\SteamVR"echo FOUND:SteamVR
if exist"C:\Program Files\Oculus"echo FOUND:Oculus
if exist"C:\Windows\System32\MixedRealityRuntime.dll"echo FOUND:WMR
echo [2]Audio locks...
tasklist|findstr/i"discord obs spotify chrome"
echo [3]VR USB...
powershell"Get-PnPDevice|Where-Object{$_.FriendlyName -like'*VR*'-or$_.FriendlyName -like'*Oculus*'-or$_.FriendlyName -like'*Vive*'}|Select FriendlyName,Status"
echo Next:Install,then run audio_diagnostic.bat
pause
```

**Phase2:Install(5min)**
1. Download:`VRBinauralRecorder-Windows-x64-Setup.exe`
2. Run as Admin:auto includes OpenVR DLLs,creates shortcuts,sets up HRTF data,configures Win audio perms
3. Verify:
```batch
cd"C:\Program Files\VR Binaural Recorder"
dir openvr_api.dll
dir deps\hrtf\
VRBinauralRecorder.exe --version
```

**Phase3:Audio Path Valid(5min)→MOST CRITICAL**
```batch
REM audio_diagnostic.bat
@echo off
echo Test TRUE binaural spatial(not just stereo routed)
echo [1]Diagnostic mode...
"C:\Program Files\VR Binaural Recorder\VRBinauralRecorder.exe"--diagnostic-mode
echo [2]Listen:tone circling 360°,"Left ear only"(silent right),"Right ear only"(silent left),footsteps front→back
echo [3]If NORMAL STEREO→audio routed incorrectly(see TROUBLESHOOTING)
pause
```

## 🎧HEADSET SETUP

**Quest 2/3/Pro(common ASMRtist)**
Issue:complex audio routing via Oculus
Steps:
1. Install Oculus Desktop:`https://www.oculus.com/setup/`
2. Enable Dev Mode:Oculus→Settings→General→Unknown Sources:ON(allows audio routing access)
3. Config Audio:Oculus→Devices→Quest→Audio→Output:"Oculus Virtual Audio",Input:"Oculus Virtual Audio"
4. Test:`VRBinauralRecorder.exe --test-quest-audio-routing`

Common:
- ❌"Audio device not found"→✅Restart Oculus service:`Services.msc→Oculus VR Runtime Service→Restart`
- ❌Getting stereo not binaural→✅Disable Quest spatial audio:`Oculus→Devices→Audio→Spatial Audio:OFF`

**Vive/Index(professional)**
Issue:Native SteamVR,version-sensitive
Steps:
1. Install SteamVR:`Steam→Library→Tools→SteamVR`
2. Config:`SteamVR→Settings→Audio→Playback:"Vive Audio",Recording:"Vive Mic",Mirror:"recording interface"`
3. Test:`VRBinauralRecorder.exe --test-steamvr-audio-routing`

Common:
- ❌"SteamVR not detected"→✅Launch SteamVR first,then app
- ❌Audio stutter on head move→✅Reduce tracking:`SteamVR→Settings→Developer→Refresh:72Hz`

**WMR(budget)**
Issue:MS OpenVR wrapper conflicts
Steps:
1. Install WMR Portal:`MS Store→Windows Mixed Reality`
2. Enable SteamVR compat:`WMR→Settings→Startup→Use for SteamVR:ON`
3. Config:`WMR→Settings→Audio→Input/Output:"Default"`

Common:❌"OpenVR init failed"→✅Start WMR Portal,wait"Ready",then launch app

## 🔊AUDIO TROUBLESHOOTING

**Problem1:"Normal stereo not spatial"**
Diagnosis:bypass HRTF
Solutions:
1. Check Win exclusive:`Sound Settings→App volume→VR Binaural Recorder using intended output`
2. Disable others:
```batch
taskkill/f/im discord.exe
taskkill/f/im obs64.exe
taskkill/f/im spotify.exe
taskkill/f/im chrome.exe
```
3. Reset Win audio:
```batch
REM Admin
net stop audiosrv
net start audiosrv
```

**Problem2:"Audio device not found"/"Init failed"**
Diagnosis:exclusive conflict/driver
Solutions:
1. Check exclusive:`powershell"Get-Process|Where-Object{$_.ProcessName -like'*audio*'}|Select ProcessName,Id"`
2. VR runtime conflict:
```batch
net stop"SteamVR Server"
net stop"Oculus VR Runtime Service"
net start"Oculus VR Runtime Service"
net start"SteamVR Server"
```
3. Driver reset:`pnputil/enum-devices/class AudioEndpoint`(disable+re-enable device in Device Mgr)

**Problem3:"VR overlay not visible"/"Black screen"**
Diagnosis:OpenGL render/VR overlay conflict
Solutions:
1. Graphics driver update:NVIDIA(GeForce Exp),AMD(Radeon SW)
2. VR overlay conflict:
```batch
taskkill/f/im"SteamVR.exe"
taskkill/f/im"vrserver.exe"
taskkill/f/im"OculusClient.exe"
VRBinauralRecorder.exe --exclusive-vr-mode
```

## 🎙️ASMRTIST WORKFLOW

**Pre-Recording Checklist(5min every session)**
```batch
REM pre_recording_check.bat
@echo off
echo [1]Audio Path(30s)
VRBinauralRecorder.exe --test-binaural-output --duration 30
echo [2]VR Tracking(15s)
VRBinauralRecorder.exe --test-head-tracking --duration 15
echo [3]Performance(30s)
VRBinauralRecorder.exe --performance-test --duration 30
echo [4]Full Workflow(2min test)
VRBinauralRecorder.exe --test-recording --duration 120 --output test_recording.wav
echo [5]Results
if exist test_recording.wav(
  echo ✅Test created
  dir test_recording.wav|findstr test_recording.wav
  echo ✅Ready for ASMR session!
)else(
  echo ❌Failed→DO NOT proceed
  echo Check:%APPDATA%\VRBinauralRecorder\logs\
)
pause
```

**Mic Positioning**
1. Primary:30cm front,mouth height seated/standing,pointing toward typical head position
2. VR calibrate:put on headset,stand at recording position,launch app,"Set Origin",walk around→test spatial
3. Spatial valid:walk circle around mic,expect:mic audio pans opposite movement(move left→mic sounds right)

**Session Optimization(hour-long)**
1. Resources:
```batch
wmic process where name="VRBinauralRecorder.exe"CALL setpriority"high priority"
powercfg/change standby-timeout-ac 0
powercfg/change hibernate-timeout-ac 0
```
2. Perf monitor:`VRBinauralRecorder.exe --enable-performance-overlay`(frame drops<1%,audio latency<20ms,CPU<70%)

## 🐛ADVANCED DIAGNOSTICS

**System Info Collection**
```batch
REM system_info.bat→generates system_report.txt
@echo off
echo===VR Binaural System Diagnostic===>system_report.txt
echo Generated:%date%%time%>>system_report.txt
echo[SYSTEM]>>system_report.txt
systeminfo|findstr/i"OS Name OS Version System Manufacturer System Model Total Physical Memory">>system_report.txt
echo[GRAPHICS]>>system_report.txt
dxdiag/t dxdiag_temp.txt
findstr/i"Card name Driver Date Driver Version"dxdiag_temp.txt>>system_report.txt
del dxdiag_temp.txt
echo[AUDIO]>>system_report.txt
powershell"Get-PnPDevice -Class AudioEndpoint|Select FriendlyName,Status">>system_report.txt
echo[VR HARDWARE]>>system_report.txt
powershell"Get-PnPDevice|Where-Object{$_.FriendlyName -like'*VR*'-or$_.FriendlyName -like'*Oculus*'-or$_.FriendlyName -like'*Vive*'}|Select FriendlyName,Status">>system_report.txt
echo[RUNNING]>>system_report.txt
tasklist|findstr/i"VR Oculus Steam discord obs spotify">>system_report.txt
echo Report saved:system_report.txt(include when reporting issues!)
```

**Log Analysis**
```batch
findstr/i"error failed exception""%APPDATA%\VRBinauralRecorder\logs\*.log"
findstr/i"audio device exclusive access""%APPDATA%\VRBinauralRecorder\logs\*.log"
findstr/i"openvr steamvr oculus tracking""%APPDATA%\VRBinauralRecorder\logs\*.log"
```

## 🚀SUCCESS VALIDATION

**Audio Test:**Record 30s walking around mic
- ✅Expect:mic position changes w/VR head,clear binaural stereo imaging(distinct L/R)
- ❌Fail:sounds normal stereo regardless head movement

**VR Test:**Move head watching VR overlay
- ✅Expect:smooth tracking no jitter,UI overlay stable in VR space
- ❌Fail:tracking lag,stuttering,overlay disappears

**Perf Test:**Record 10min continuous
- ✅Expect:no audio dropouts/frame drops,file size grows continuously
- ❌Fail:stuttering audio/perf warnings

## 📞HELP

**90% issues:**
1. Audio routing(50%)→audio=stereo not spatial
2. VR runtime conflicts(25%)→multiple VR systems fighting
3. Win perms(15%)→exclusive audio denied

**Before support:**Run diagnostics,check system_report.txt,review logs for errors

**Contact if:**Diagnostics"PASS"but recording fails,system meets reqs but perf poor,troubleshooting makes worse

**ASMRtist Priority:**asmr-support@vr-binaural-recorder.com(include system_report.txt+sample),response<24hr for blocking issues

Ready to create amazing spatial ASMR!🎧🥽🎤

Last:2025-09-27|v1.0|Win10/11 x64
