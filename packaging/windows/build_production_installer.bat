@echo off
REM VR Binaural Recorder Production Windows Installer Build Script
REM Mike Rodriguez - Production-ready installer creation for ASMRtist testers
REM Requirements: NSIS, Visual Studio 2019/2022, Windows 10+ SDK

setlocal enabledelayedexpansion

echo ========================================
echo VR Binaural Recorder Production Build
echo ========================================
echo.

REM Check prerequisites
echo [INFO] Checking build prerequisites...

REM Check Visual Studio
where cl.exe >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] Visual Studio compiler not found in PATH
    echo Please run this script from a Visual Studio Developer Command Prompt
    exit /b 1
)

REM Check NSIS
where makensis.exe >nul 2>&1
if %errorlevel% neq 0 (
    echo [WARN] NSIS not found in PATH, checking standard locations...
    if exist "C:\Program Files (x86)\NSIS\makensis.exe" (
        set "PATH=%PATH%;C:\Program Files (x86)\NSIS"
        echo [INFO] Found NSIS at C:\Program Files ^(x86^)\NSIS
    ) else if exist "C:\Program Files\NSIS\makensis.exe" (
        set "PATH=%PATH%;C:\Program Files\NSIS"
        echo [INFO] Found NSIS at C:\Program Files\NSIS
    ) else (
        echo [ERROR] NSIS not found. Please install from https://nsis.sourceforge.io/
        echo Or use chocolatey: choco install nsis
        exit /b 1
    )
)

REM Check CMake
where cmake.exe >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] CMake not found in PATH
    echo Please install CMake from https://cmake.org/download/
    exit /b 1
)

echo [INFO] All prerequisites found
echo.

REM Set production build parameters
set BUILD_TYPE=Release
set BUILD_DIR=build-windows-release
set SOURCE_DIR=%~dp0
set CMAKE_GENERATOR="Visual Studio 17 2022"
set ARCHITECTURE=x64

REM Override generator if VS2019 is being used
cl.exe 2>&1 | findstr /C:"19.2" >nul && set CMAKE_GENERATOR="Visual Studio 16 2019"

echo [INFO] Configuration:
echo   Build Type: %BUILD_TYPE%
echo   Build Directory: %BUILD_DIR%
echo   Generator: %CMAKE_GENERATOR%
echo   Architecture: %ARCHITECTURE%
echo.

REM Clean previous build if requested
if "%1"=="clean" (
    echo [INFO] Cleaning previous build directory...
    if exist "%BUILD_DIR%" (
        rmdir /s /q "%BUILD_DIR%"
    )
)

REM Create build directory
if not exist "%BUILD_DIR%" (
    echo [INFO] Creating build directory: %BUILD_DIR%
    mkdir "%BUILD_DIR%"
)

cd /d "%BUILD_DIR%"

echo [INFO] Configuring CMake with production optimizations...
cmake ^
    -G %CMAKE_GENERATOR% ^
    -A %ARCHITECTURE% ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DCMAKE_C_FLAGS_RELEASE="/O2 /GL /DNDEBUG /arch:AVX2" ^
    -DCMAKE_CXX_FLAGS_RELEASE="/O2 /GL /DNDEBUG /arch:AVX2 /std:c++17" ^
    -DCMAKE_EXE_LINKER_FLAGS_RELEASE="/LTCG /OPT:REF /OPT:ICF" ^
    -DCMAKE_INSTALL_PREFIX="install" ^
    -DENABLE_PRODUCTION_BUILD=ON ^
    -DENABLE_AVX2_OPTIMIZATION=ON ^
    -DENABLE_OPENGL_ACCELERATION=ON ^
    -DWINDOWS_RELEASE_BUILD=ON ^
    "%SOURCE_DIR%"

if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration failed
    exit /b 1
)

echo [INFO] Building release binaries with maximum optimizations...
cmake --build . --config %BUILD_TYPE% --parallel %NUMBER_OF_PROCESSORS%

if %errorlevel% neq 0 (
    echo [ERROR] Build failed
    exit /b 1
)

echo.
echo [INFO] Verifying main executable was built...
if not exist "%BUILD_TYPE%\vr_binaural_recorder.exe" (
    echo [ERROR] Main executable not found at %BUILD_TYPE%\vr_binaural_recorder.exe
    exit /b 1
)

REM Get file size for verification
for %%A in ("%BUILD_TYPE%\vr_binaural_recorder.exe") do set EXEC_SIZE=%%~zA
echo [INFO] Main executable size: %EXEC_SIZE% bytes

if %EXEC_SIZE% lss 1000000 (
    echo [WARN] Executable size is less than 1MB - this may indicate a problem
)

echo.
echo [INFO] Building Windows installer package...
cmake --build . --target windows_installer --config %BUILD_TYPE%

if %errorlevel% neq 0 (
    echo [ERROR] Windows installer creation failed
    exit /b 1
)

echo [INFO] Building portable ZIP package...
cmake --build . --target windows_zip --config %BUILD_TYPE%

if %errorlevel% neq 0 (
    echo [WARN] Portable ZIP package creation failed, but continuing...
)

echo.
echo [INFO] Production build completed successfully!
echo.

REM Verify installer was created
set INSTALLER_PATH=installer\VRBinauralRecorder-1.0.0-Setup.exe
if exist "%INSTALLER_PATH%" (
    for %%A in ("%INSTALLER_PATH%") do set INSTALLER_SIZE=%%~zA
    echo [SUCCESS] Installer created: %INSTALLER_PATH%
    echo [INFO] Installer size: !INSTALLER_SIZE! bytes
) else (
    echo [ERROR] Installer not found at expected location: %INSTALLER_PATH%
    exit /b 1
)

REM Verify portable package
set PORTABLE_PATH=installer\VRBinauralRecorder-1.0.0-Portable.zip
if exist "%PORTABLE_PATH%" (
    for %%A in ("%PORTABLE_PATH%") do set PORTABLE_SIZE=%%~zA
    echo [SUCCESS] Portable package created: %PORTABLE_PATH%
    echo [INFO] Portable package size: !PORTABLE_SIZE! bytes
) else (
    echo [WARN] Portable package not found (optional)
)

echo.
echo ========================================
echo PRODUCTION BUILD SUMMARY
echo ========================================
echo Main executable: %BUILD_TYPE%\vr_binaural_recorder.exe (^!EXEC_SIZE^! bytes)
echo Installer package: %INSTALLER_PATH% (^!INSTALLER_SIZE^! bytes)
if exist "%PORTABLE_PATH%" (
    echo Portable package: %PORTABLE_PATH% (^!PORTABLE_SIZE^! bytes)
)
echo.
echo [INFO] Ready for testing with: packaging\windows\test-installer.ps1
echo [COMMAND] powershell -ExecutionPolicy Bypass -File "..\packaging\windows\test-installer.ps1" -InstallerPath "%INSTALLER_PATH%" -Verbose
echo.

if "%1" neq "no-test" (
    echo [INFO] Running automated installer validation...
    echo.
    powershell -ExecutionPolicy Bypass -File "%SOURCE_DIR%packaging\windows\test-installer.ps1" -InstallerPath "%INSTALLER_PATH%" -UninstallAfterTest:$true -Verbose -ReportPath "installer-test-report.json"

    if !errorlevel! equ 0 (
        echo.
        echo [SUCCESS] Installer validation PASSED! Ready for ASMRtist testing.
    ) else (
        echo.
        echo [WARNING] Installer validation had issues. Check installer-test-report.json
    )
)

echo.
echo [COMPLETE] Production Windows installer build finished.
echo [NEXT STEP] Test installer on clean Windows system before distributing to ASMRtists.

endlocal