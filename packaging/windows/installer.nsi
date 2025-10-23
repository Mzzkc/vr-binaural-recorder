; VR Binaural Recorder Windows Installer (NSIS)
; Professional Windows installer for VR Binaural Recorder

!define PRODUCT_NAME "VR Binaural Recorder"
!define PRODUCT_VERSION "1.0.0"
!define PRODUCT_PUBLISHER "VRB Audio"
!define PRODUCT_WEB_SITE "https://github.com/vrbaudio/vr-binaural-recorder"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\vr_binaural_recorder.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; Installer properties
Name "${PRODUCT_NAME}"
OutFile "VRBinauralRecorder-${PRODUCT_VERSION}-Setup.exe"
InstallDir "$PROGRAMFILES64\VR Binaural Recorder"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show
RequestExecutionLevel admin
Unicode True

; Modern UI
!include "MUI2.nsh"
!include "WinVer.nsh"
!include "x64.nsh"
!include "FileFunc.nsh"
!include "Sections.nsh"
!include "LogicLib.nsh"

; Additional includes for enhanced functionality
!insertmacro GetAvailableSpaceSize
!insertmacro VersionCompare

; Global variables
Var ROLLBACK_DIR

; Installer pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\LICENSE"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\vr_binaural_recorder.exe"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\README.md"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Languages
!insertmacro MUI_LANGUAGE "English"

; Version information
VIProductVersion "${PRODUCT_VERSION}.0"
VIAddVersionKey "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey "Comments" "Professional VR Spatial Audio Recording"
VIAddVersionKey "CompanyName" "${PRODUCT_PUBLISHER}"
VIAddVersionKey "LegalCopyright" "© 2024 ${PRODUCT_PUBLISHER}"
VIAddVersionKey "FileDescription" "${PRODUCT_NAME} Installer"
VIAddVersionKey "FileVersion" "${PRODUCT_VERSION}"
VIAddVersionKey "ProductVersion" "${PRODUCT_VERSION}"
VIAddVersionKey "InternalName" "VRBinauralRecorderSetup"
VIAddVersionKey "OriginalFilename" "VRBinauralRecorder-${PRODUCT_VERSION}-Setup.exe"

; Installation sections
Section "Core Application" SEC01
  SectionIn RO

  ; Create backup directory for rollback
  CreateDirectory "$TEMP\VRBinauralRecorder_Backup_${PRODUCT_VERSION}"
  StrCpy $ROLLBACK_DIR "$TEMP\VRBinauralRecorder_Backup_${PRODUCT_VERSION}"

  SetOutPath "$INSTDIR"

  ; Main executable with validation
  DetailPrint "Installing main executable..."
  ClearErrors
  File "${BUILD_DIR}\vr_binaural_recorder.exe"
  IfErrors 0 exe_ok
    DetailPrint "ERROR: Failed to install main executable"
    Call RollbackInstallation
    MessageBox MB_ICONSTOP "Installation failed: Unable to copy main executable.$\n$\nThis may be due to:$\n• Insufficient permissions$\n• Antivirus interference$\n• Disk space shortage"
    Abort
  exe_ok:

  ; Verify executable integrity
  GetFileSize "$INSTDIR\vr_binaural_recorder.exe" $0
  IntCmp $0 1000000 size_ok size_error size_ok  ; Minimum 1MB expected
  size_error:
    DetailPrint "ERROR: Executable file appears corrupted (size: $0 bytes)"
    Call RollbackInstallation
    MessageBox MB_ICONSTOP "Installation failed: Executable file integrity check failed."
    Abort
  size_ok:

  ; Configuration files with error handling
  DetailPrint "Installing configuration files..."
  ClearErrors
  File "${SOURCE_DIR}\config\vr_binaural_config.json.example"
  IfErrors config_error 0
  ClearErrors
  Rename "$INSTDIR\vr_binaural_config.json.example" "$INSTDIR\vr_binaural_config.json"
  IfErrors config_error config_ok
  config_error:
    DetailPrint "ERROR: Failed to install configuration files"
    Call RollbackInstallation
    MessageBox MB_ICONSTOP "Installation failed: Unable to install configuration files."
    Abort
  config_ok:

  ; HRTF data with verification
  DetailPrint "Installing HRTF data..."
  SetOutPath "$INSTDIR\hrtf"
  ClearErrors
  File /r "${SOURCE_DIR}\deps\hrtf\*.*"
  IfErrors hrtf_error 0

  ; Verify HRTF data completeness
  IfFileExists "$INSTDIR\hrtf\*.wav" hrtf_ok hrtf_verify_error
  hrtf_verify_error:
  hrtf_error:
    DetailPrint "ERROR: Failed to install HRTF data"
    Call RollbackInstallation
    MessageBox MB_ICONSTOP "Installation failed: Unable to install HRTF data files.$\n$\nHRTF data is essential for spatial audio processing."
    Abort
  hrtf_ok:

  ; Required DLLs with dependency validation
  DetailPrint "Installing required libraries..."
  SetOutPath "$INSTDIR"
  ClearErrors
  File /nonfatal "${BUILD_DIR}\*.dll"
  IfErrors 0 dll_check
    DetailPrint "WARNING: Some DLL files could not be copied"
    ; Continue but warn user
    MessageBox MB_ICONEXCLAMATION "Warning: Some library files could not be installed.$\n$\nThe application may not function correctly."

  dll_check:
  ; Verify critical DLLs are present
  IfFileExists "$INSTDIR\*.dll" dll_ok dll_missing
  dll_missing:
    DetailPrint "ERROR: No DLL files found after installation"
    Call RollbackInstallation
    MessageBox MB_ICONSTOP "Installation failed: Critical library files are missing."
    Abort
  dll_ok:

  ; Documentation
  DetailPrint "Installing documentation..."
  ClearErrors
  File /nonfatal "${SOURCE_DIR}\README.md"
  File /nonfatal "${SOURCE_DIR}\LICENSE"
  ; Documentation is optional, don't fail on error

  ; Create shortcuts with error handling
  DetailPrint "Creating shortcuts..."
  ClearErrors
  CreateDirectory "$SMPROGRAMS\VR Binaural Recorder"
  IfErrors shortcut_error 0
  CreateShortCut "$SMPROGRAMS\VR Binaural Recorder\VR Binaural Recorder.lnk" "$INSTDIR\vr_binaural_recorder.exe"
  IfErrors shortcut_error 0
  CreateShortCut "$SMPROGRAMS\VR Binaural Recorder\Uninstall.lnk" "$INSTDIR\uninst.exe"
  IfErrors shortcut_error 0
  CreateShortCut "$DESKTOP\VR Binaural Recorder.lnk" "$INSTDIR\vr_binaural_recorder.exe"
  IfErrors shortcut_error shortcut_ok
  shortcut_error:
    DetailPrint "WARNING: Some shortcuts could not be created"
    MessageBox MB_ICONEXCLAMATION "Warning: Some desktop shortcuts could not be created.$\n$\nYou can manually run the application from: $INSTDIR"
  shortcut_ok:

  ; Clean up backup directory if everything succeeded
  RMDir /r "$ROLLBACK_DIR"

SectionEnd

Section "Visual C++ Redistributable" SEC02
  ; Install VC++ Redistributable if not present
  ReadRegStr $0 HKLM "SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64" "Version"
  ${If} $0 == ""
    DetailPrint "Installing Visual C++ Redistributable..."
    File "vc_redist.x64.exe"
    ExecWait "$INSTDIR\vc_redist.x64.exe /quiet /norestart" $1
    Delete "$INSTDIR\vc_redist.x64.exe"
    ${If} $1 != 0
      MessageBox MB_ICONEXCLAMATION "Warning: Visual C++ Redistributable installation failed (Code: $1). The application may not run correctly."
    ${EndIf}
  ${Else}
    DetailPrint "Visual C++ Redistributable already installed: $0"
  ${EndIf}
SectionEnd

Section "OpenVR Runtime" SEC06
  ; Check if OpenVR/SteamVR is installed
  ReadRegStr $0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Steam App 250820" "InstallLocation"
  ReadRegStr $1 HKLM "SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\Steam App 250820" "InstallLocation"
  ReadRegStr $2 HKCU "SOFTWARE\Valve\Steam" "SteamPath"

  ; Check if SteamVR is found
  ${If} $0 != ""
  ${OrIf} $1 != ""
    DetailPrint "SteamVR installation found"
    ; Validate OpenVR binaries
    ${If} $0 != ""
      StrCpy $3 "$0\bin\win64\openvr_api.dll"
    ${Else}
      StrCpy $3 "$1\bin\win64\openvr_api.dll"
    ${EndIf}

    ${If} ${FileExists} "$3"
      DetailPrint "OpenVR API found: $3"
      WriteRegStr HKLM "SOFTWARE\VRBinauralRecorder" "OpenVRPath" "$3"
    ${Else}
      MessageBox MB_ICONEXCLAMATION "Warning: SteamVR found but OpenVR API missing. VR functionality may be limited."
    ${EndIf}
  ${ElseIf} $2 != ""
    ; Check Steam installation for SteamVR
    StrCpy $3 "$2\steamapps\common\SteamVR"
    ${If} ${FileExists} "$3"
      DetailPrint "SteamVR found in Steam directory: $3"
      WriteRegStr HKLM "SOFTWARE\VRBinauralRecorder" "SteamVRPath" "$3"
    ${Else}
      ; Offer to install SteamVR
      MessageBox MB_ICONQUESTION|MB_YESNO "SteamVR not found. VR functionality requires SteamVR.$\n$\nWould you like to download and install SteamVR now?" IDNO skipsteamvr

      DetailPrint "Launching Steam to install SteamVR..."
      ${If} ${FileExists} "$2\Steam.exe"
        ExecShell "open" "steam://install/250820" "" SW_SHOWNORMAL
        MessageBox MB_ICONINFORMATION "Steam will now open to install SteamVR. Please complete the installation and restart this installer if needed."
      ${Else}
        MessageBox MB_ICONEXCLAMATION "Steam not found. Please install Steam and SteamVR manually for VR functionality."
      ${EndIf}

      skipsteamvr:
    ${EndIf}
  ${Else}
    MessageBox MB_ICONEXCLAMATION "Steam/SteamVR not found. VR functionality will be limited to fallback mode.$\n$\nFor full VR support, please install Steam and SteamVR."
  ${EndIf}
SectionEnd

Section "Windows Audio Virtual Device Driver" SEC03
  ; Install virtual audio device driver
  DetailPrint "Installing virtual audio device driver..."
  SetOutPath "$INSTDIR\driver"
  File /r "${SOURCE_DIR}\packaging\windows\driver\*.*"

  ; Register virtual audio device
  nsExec::ExecToLog '"$INSTDIR\driver\install_driver.bat"'
  Pop $0
  ${If} $0 != 0
    MessageBox MB_ICONEXCLAMATION "Warning: Virtual audio device driver installation failed. Some features may not work correctly."
  ${EndIf}
SectionEnd

Section "ASIO Driver Support" SEC04
  ; Optional ASIO driver support
  SetOutPath "$INSTDIR\asio"
  File /nonfatal "${SOURCE_DIR}\third_party\asiosdk\*.*"
SectionEnd

Section "Start Menu Integration" SEC05
  ; Additional start menu entries
  CreateShortCut "$SMPROGRAMS\VR Binaural Recorder\Configuration.lnk" "$INSTDIR\vr_binaural_config.json"
  CreateShortCut "$SMPROGRAMS\VR Binaural Recorder\Documentation.lnk" "$INSTDIR\README.md"
  CreateShortCut "$SMPROGRAMS\VR Binaural Recorder\Website.lnk" "${PRODUCT_WEB_SITE}"
SectionEnd

; Component descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} "Core VR Binaural Recorder application and required files"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC02} "Microsoft Visual C++ Redistributable (required for application to run)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC03} "Windows virtual audio device driver for system-wide audio capture"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC04} "ASIO driver support for professional audio interfaces"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC05} "Additional Start Menu shortcuts and integration"
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC06} "OpenVR/SteamVR runtime detection and installation assistance"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; Installation events
Function .onInit
  ; Check Windows version (minimum Windows 10)
  ${IfNot} ${AtLeastWin10}
    MessageBox MB_ICONSTOP "This application requires Windows 10 or later.$\n$\nSupported versions:$\n• Windows 10 (1903 or later)$\n• Windows 11"
    Abort
  ${EndIf}

  ; Check architecture (x64 only)
  ${IfNot} ${RunningX64}
    MessageBox MB_ICONSTOP "This application requires 64-bit Windows.$\n$\nYour system: $${RunningX64}"
    Abort
  ${EndIf}

  ; Check available disk space (minimum 500MB)
  ${GetAvailableSpaceSize} "$PROGRAMFILES64" $R0
  ${If} $R0 < 500
    MessageBox MB_ICONSTOP "Insufficient disk space. Minimum 500MB required, $R0 MB available."
    Abort
  ${EndIf}

  ; Check if already installed and compare versions
  ReadRegStr $R0 ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString"
  ReadRegStr $R1 ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion"

  ${If} $R0 != ""
    ${If} $R1 == "${PRODUCT_VERSION}"
      MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "VR Binaural Recorder ${PRODUCT_VERSION} is already installed.$\n$\nWould you like to repair the installation?" IDYES repair IDNO cancel
      repair:
        ; Continue with installation to repair
        Goto version_check_done
      cancel:
        Quit
    ${Else}
      ; Different version installed
      ${VersionCompare} "$R1" "${PRODUCT_VERSION}" $R2
      ${If} $R2 == 1
        ; Newer version installed
        MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "A newer version ($R1) is already installed.$\n$\nDo you want to downgrade to ${PRODUCT_VERSION}?" IDNO cancel
      ${ElseIf} $R2 == 0
        ; Same version (should not happen due to check above)
        MessageBox MB_ICONINFORMATION "Version ${PRODUCT_VERSION} is already installed."
        Quit
      ${Else}
        ; Older version installed
        MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON1 "An older version ($R1) is installed.$\n$\nDo you want to update to ${PRODUCT_VERSION}?" IDNO cancel
      ${EndIf}

      ; Uninstall previous version with validation
      DetailPrint "Uninstalling previous version..."
      ClearErrors
      ExecWait '$R0 /S _?=$INSTDIR' $R3
      ${If} ${Errors}
        MessageBox MB_ICONSTOP "Failed to uninstall previous version. Installation cannot continue."
        Abort
      ${EndIf}
      ${If} $R3 != 0
        MessageBox MB_ICONEXCLAMATION "Previous version uninstall completed with warnings (Code: $R3)."
      ${EndIf}
    ${EndIf}
  ${EndIf}

  version_check_done:

  ; Validate critical system dependencies
  Call ValidateSystemDependencies

  ; Continue if validation passes
  Goto done

  cancel:
    Quit

  done:
FunctionEnd

; Function to rollback partial installation
Function RollbackInstallation
  DetailPrint "Rolling back partial installation..."

  ; Remove partially copied files
  Delete "$INSTDIR\vr_binaural_recorder.exe"
  Delete "$INSTDIR\*.dll"
  Delete "$INSTDIR\vr_binaural_config.json*"
  Delete "$INSTDIR\README.md"
  Delete "$INSTDIR\LICENSE"

  ; Remove directories
  RMDir /r "$INSTDIR\hrtf"

  ; Remove shortcuts
  Delete "$SMPROGRAMS\VR Binaural Recorder\VR Binaural Recorder.lnk"
  Delete "$SMPROGRAMS\VR Binaural Recorder\Uninstall.lnk"
  Delete "$DESKTOP\VR Binaural Recorder.lnk"
  RMDir "$SMPROGRAMS\VR Binaural Recorder"

  ; Clean install directory if empty
  RMDir "$INSTDIR"

  ; Clean backup directory
  RMDir /r "$ROLLBACK_DIR"

  DetailPrint "Rollback completed"
FunctionEnd

; Function to validate system dependencies before installation
Function ValidateSystemDependencies
  Push $0
  Push $1

  ; Check Windows Audio Service
  nsExec::ExecToStack 'sc query AudioSrv'
  Pop $0
  Pop $1
  ${If} $0 != 0
    MessageBox MB_ICONEXCLAMATION "Warning: Windows Audio Service not available. Audio functionality may be limited."
  ${EndIf}

  ; Check for Windows Defender real-time protection interference
  ReadRegDWord HKLM "SOFTWARE\Microsoft\Windows Defender\Real-Time Protection" "DisableRealtimeMonitoring" $0
  ${If} $0 != 1
    ; Real-time protection is enabled, warn about potential audio latency
    MessageBox MB_ICONINFORMATION "Windows Defender real-time protection is active.$\n$\nFor optimal audio performance, consider adding VR Binaural Recorder to Windows Defender exclusions after installation."
  ${EndIf}

  ; Check for sufficient memory (minimum 4GB recommended)
  System::Int64Op 4 * 1024 * 1024 * 1024  ; 4GB in bytes
  Pop $0
  System::Call "kernel32::GlobalMemoryStatusEx(*i.r1) i.s"
  System::Call "*$1(i64.s, i64.s, i64.r2, i64.s, i64.s, i64.s, i64.s, i64.s)"
  System::Free $1

  ${If} $2 L< $0
    MessageBox MB_ICONEXCLAMATION "Warning: Less than 4GB RAM available. Application performance may be limited.$\n$\nRecommended: 8GB+ RAM for optimal VR audio processing."
  ${EndIf}

  Pop $1
  Pop $0
FunctionEnd

Function .onInstSuccess
  ; Register application
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\vr_binaural_recorder.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\vr_binaural_recorder.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegDWord ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoModify" 1
  WriteRegDWord ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoRepair" 1

  ; Calculate installed size
  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  IntFmt $0 "0x%08X" $0
  WriteRegDWord ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "EstimatedSize" "$0"

  ; Create uninstaller
  WriteUninstaller "$INSTDIR\uninst.exe"

  ; Register file associations
  WriteRegStr HKCR ".vrb" "" "VRBinauralRecording"
  WriteRegStr HKCR "VRBinauralRecording" "" "VR Binaural Recording"
  WriteRegStr HKCR "VRBinauralRecording\DefaultIcon" "" "$INSTDIR\vr_binaural_recorder.exe,0"
  WriteRegStr HKCR "VRBinauralRecording\shell\open\command" "" '"$INSTDIR\vr_binaural_recorder.exe" "%1"'

  ; Refresh shell
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'
FunctionEnd

; Uninstaller
Section Uninstall
  ; Stop application if running
  KillProcDLL::KillProc "vr_binaural_recorder.exe"
  Sleep 2000

  ; Remove virtual audio device driver
  nsExec::ExecToLog '"$INSTDIR\driver\uninstall_driver.bat"'

  ; Remove files
  Delete "$INSTDIR\vr_binaural_recorder.exe"
  Delete "$INSTDIR\*.dll"
  Delete "$INSTDIR\vr_binaural_config.json"
  Delete "$INSTDIR\README.md"
  Delete "$INSTDIR\LICENSE"
  Delete "$INSTDIR\uninst.exe"

  ; Remove directories
  RMDir /r "$INSTDIR\hrtf"
  RMDir /r "$INSTDIR\driver"
  RMDir /r "$INSTDIR\asio"
  RMDir "$INSTDIR"

  ; Remove shortcuts
  Delete "$SMPROGRAMS\VR Binaural Recorder\VR Binaural Recorder.lnk"
  Delete "$SMPROGRAMS\VR Binaural Recorder\Configuration.lnk"
  Delete "$SMPROGRAMS\VR Binaural Recorder\Documentation.lnk"
  Delete "$SMPROGRAMS\VR Binaural Recorder\Website.lnk"
  Delete "$SMPROGRAMS\VR Binaural Recorder\Uninstall.lnk"
  RMDir "$SMPROGRAMS\VR Binaural Recorder"
  Delete "$DESKTOP\VR Binaural Recorder.lnk"

  ; Remove registry entries
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  DeleteRegKey HKLM "SOFTWARE\VRBinauralRecorder"
  DeleteRegKey HKCR ".vrb"
  DeleteRegKey HKCR "VRBinauralRecording"

  ; Refresh shell
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'

  SetAutoClose true
SectionEnd