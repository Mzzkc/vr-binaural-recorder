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
  SetOutPath "$INSTDIR"

  ; Main executable
  File "${BUILD_DIR}\vr_binaural_recorder.exe"

  ; Configuration files
  File "${SOURCE_DIR}\config\vr_binaural_config.json.example"
  Rename "$INSTDIR\vr_binaural_config.json.example" "$INSTDIR\vr_binaural_config.json"

  ; HRTF data
  SetOutPath "$INSTDIR\hrtf"
  File /r "${SOURCE_DIR}\deps\hrtf\*.*"

  ; Required DLLs
  SetOutPath "$INSTDIR"
  File "${BUILD_DIR}\*.dll"

  ; Documentation
  File "${SOURCE_DIR}\README.md"
  File "${SOURCE_DIR}\LICENSE"

  ; Create shortcuts
  CreateDirectory "$SMPROGRAMS\VR Binaural Recorder"
  CreateShortCut "$SMPROGRAMS\VR Binaural Recorder\VR Binaural Recorder.lnk" "$INSTDIR\vr_binaural_recorder.exe"
  CreateShortCut "$SMPROGRAMS\VR Binaural Recorder\Uninstall.lnk" "$INSTDIR\uninst.exe"
  CreateShortCut "$DESKTOP\VR Binaural Recorder.lnk" "$INSTDIR\vr_binaural_recorder.exe"
SectionEnd

Section "Visual C++ Redistributable" SEC02
  ; Install VC++ Redistributable if not present
  ReadRegStr $0 HKLM "SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64" "Version"
  ${If} $0 == ""
    DetailPrint "Installing Visual C++ Redistributable..."
    File "vc_redist.x64.exe"
    ExecWait "$INSTDIR\vc_redist.x64.exe /quiet /norestart"
    Delete "$INSTDIR\vc_redist.x64.exe"
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
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; Installation events
Function .onInit
  ; Check Windows version (minimum Windows 10)
  ${IfNot} ${AtLeastWin10}
    MessageBox MB_ICONSTOP "This application requires Windows 10 or later."
    Abort
  ${EndIf}

  ; Check architecture (x64 only)
  ${IfNot} ${RunningX64}
    MessageBox MB_ICONSTOP "This application requires 64-bit Windows."
    Abort
  ${EndIf}

  ; Check if already installed
  ReadRegStr $R0 ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString"
  StrCmp $R0 "" done

  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "VR Binaural Recorder is already installed. Do you want to uninstall the previous version first?" IDNO done

  ; Uninstall previous version
  ExecWait '$R0 /S _?=$INSTDIR'

  done:
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
  DeleteRegKey HKCR ".vrb"
  DeleteRegKey HKCR "VRBinauralRecording"

  ; Refresh shell
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'

  SetAutoClose true
SectionEnd