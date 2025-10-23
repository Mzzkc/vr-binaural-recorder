@echo off
REM VR Binaural Recorder Installer Testing Batch Script
REM Wrapper for PowerShell installer testing framework

setlocal EnableDelayedExpansion

echo ==========================================
echo VR Binaural Recorder Installer Tester
echo ==========================================
echo.

REM Check for PowerShell availability
powershell -Command "Write-Host 'PowerShell available'" >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: PowerShell is not available or not in PATH
    echo Please ensure PowerShell is installed and accessible
    pause
    exit /b 1
)

REM Get installer path from command line or prompt
if "%1"=="" (
    echo Please provide the path to the installer executable:
    set /p INSTALLER_PATH="Installer Path: "
) else (
    set INSTALLER_PATH=%1
)

REM Check if installer exists
if not exist "!INSTALLER_PATH!" (
    echo ERROR: Installer file not found: !INSTALLER_PATH!
    pause
    exit /b 1
)

echo Found installer: !INSTALLER_PATH!
echo.

REM Ask for uninstall preference
set /p UNINSTALL_CHOICE="Uninstall after testing? (Y/N) [Y]: "
if "!UNINSTALL_CHOICE!"=="" set UNINSTALL_CHOICE=Y
if /i "!UNINSTALL_CHOICE!"=="N" (
    set UNINSTALL_PARAM="-UninstallAfterTest:$false"
) else (
    set UNINSTALL_PARAM=""
)

REM Ask for verbose output
set /p VERBOSE_CHOICE="Enable verbose logging? (Y/N) [N]: "
if /i "!VERBOSE_CHOICE!"=="Y" (
    set VERBOSE_PARAM="-Verbose"
) else (
    set VERBOSE_PARAM=""
)

echo.
echo Starting installer testing...
echo ==========================================

REM Execute PowerShell testing script
powershell -ExecutionPolicy Bypass -File "%~dp0test-installer.ps1" -InstallerPath "!INSTALLER_PATH!" !UNINSTALL_PARAM! !VERBOSE_PARAM! -ReportPath "%~dp0installer-test-report.json"

set TEST_RESULT=%ERRORLEVEL%

echo.
echo ==========================================
if %TEST_RESULT% EQU 0 (
    echo TESTING COMPLETED SUCCESSFULLY
    echo Report: %~dp0installer-test-report.json
) else (
    echo TESTING FAILED - CHECK LOGS FOR DETAILS
    echo Report: %~dp0installer-test-report.json
    if exist "%~dp0installer-test.log" (
        echo Log: %~dp0installer-test.log
    )
)

echo ==========================================

pause
exit /b %TEST_RESULT%