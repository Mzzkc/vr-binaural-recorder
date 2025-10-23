# Windows packaging CMakeLists.txt - Windows installer and distribution

if(NOT WIN32)
    message(WARNING "Windows packaging is only available on Windows platforms")
    return()
endif()

# Find NSIS for installer creation
find_program(NSIS_MAKENSIS
    NAMES makensis
    PATHS
        "C:/Program Files (x86)/NSIS"
        "C:/Program Files/NSIS"
        "$ENV{ProgramFiles}/NSIS"
    DOC "NSIS makensis compiler"
)

# Find Windows SDK for code signing
find_program(SIGNTOOL_EXECUTABLE
    NAMES signtool
    PATHS
        "C:/Program Files (x86)/Windows Kits/10/bin/x64"
        "C:/Program Files (x86)/Windows Kits/10/bin/10.0.*/x64"
        "C:/Program Files/Windows Kits/10/bin/x64"
        "$ENV{WindowsSdkBinPath}/x64"
    DOC "Windows SignTool for code signing"
)

# Variables for installer
set(INSTALLER_SOURCE_DIR "${CMAKE_SOURCE_DIR}")
set(INSTALLER_BUILD_DIR "${CMAKE_BINARY_DIR}")
set(INSTALLER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/installer")

# Create installer output directory
file(MAKE_DIRECTORY "${INSTALLER_OUTPUT_DIR}")

# Validate critical dependencies exist before packaging (warnings only during configure)
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/third_party/openvr/bin/win64/openvr_api.dll")
    message(WARNING "OpenVR DLL not found at: ${CMAKE_SOURCE_DIR}/third_party/openvr/bin/win64/openvr_api.dll - installer may fail")
else()
    message(STATUS "OpenVR DLL found: ${CMAKE_SOURCE_DIR}/third_party/openvr/bin/win64/openvr_api.dll")
endif()

# Check for OpenVR headers (include is a symlink, headers is the real directory)
if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/openvr/include/openvr.h")
    message(STATUS "OpenVR header found: ${CMAKE_SOURCE_DIR}/third_party/openvr/include/openvr.h")
elseif(EXISTS "${CMAKE_SOURCE_DIR}/third_party/openvr/headers/openvr.h")
    message(STATUS "OpenVR header found: ${CMAKE_SOURCE_DIR}/third_party/openvr/headers/openvr.h")
else()
    message(WARNING "OpenVR header not found - build may fail")
endif()

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/hrtf_data")
    message(WARNING "HRTF data directory not found at: ${CMAKE_SOURCE_DIR}/hrtf_data - runtime may fail")
else()
    message(STATUS "HRTF data found: ${CMAKE_SOURCE_DIR}/hrtf_data")
endif()

# Configure installer script
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/installer.nsi"
    "${INSTALLER_OUTPUT_DIR}/installer.nsi"
    @ONLY
)

# Function to sign executable
function(sign_executable TARGET_FILE)
    if(SIGNTOOL_EXECUTABLE AND WINDOWS_CODE_SIGN_CERT)
        if(WINDOWS_CODE_SIGN_PASSWORD)
            set(SIGN_PASSWORD_ARG "/p" "${WINDOWS_CODE_SIGN_PASSWORD}")
        else()
            set(SIGN_PASSWORD_ARG "")
        endif()

        add_custom_command(
            TARGET ${TARGET_FILE} POST_BUILD
            COMMAND ${SIGNTOOL_EXECUTABLE}
                sign
                /f "${WINDOWS_CODE_SIGN_CERT}"
                ${SIGN_PASSWORD_ARG}
                /t "http://timestamp.verisign.com/scripts/timstamp.dll"
                /d "VR Binaural Recorder"
                /du "https://github.com/vrbaudio/vr-binaural-recorder"
                "$<TARGET_FILE:${TARGET_FILE}>"
            COMMENT "Code signing ${TARGET_FILE}"
            VERBATIM
        )
    endif()
endfunction()

# Sign main executable if certificate is available
if(TARGET vr_binaural_recorder)
    sign_executable(vr_binaural_recorder)
endif()

# Create Windows installer
if(NSIS_MAKENSIS)
    # Copy required files for installer
    add_custom_target(prepare_installer
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/hrtf_data"
            "${INSTALLER_OUTPUT_DIR}/hrtf_data"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:vr_binaural_recorder>"
            "${INSTALLER_OUTPUT_DIR}/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/config/vr_binaural_config.json.example"
            "${INSTALLER_OUTPUT_DIR}/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/README.md"
            "${INSTALLER_OUTPUT_DIR}/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/LICENSE"
            "${INSTALLER_OUTPUT_DIR}/"
        # Copy OpenVR runtime DLL - CRITICAL for VR functionality
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/third_party/openvr/bin/win64/openvr_api.dll"
            "${INSTALLER_OUTPUT_DIR}/"
        # Copy OpenVR headers and libs for development builds
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "${INSTALLER_OUTPUT_DIR}/third_party/openvr/include"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/third_party/openvr/include/openvr.h"
            "${INSTALLER_OUTPUT_DIR}/third_party/openvr/include/"
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "${INSTALLER_OUTPUT_DIR}/third_party/openvr/lib/win64"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/third_party/openvr/lib/win64/openvr_api.lib"
            "${INSTALLER_OUTPUT_DIR}/third_party/openvr/lib/win64/"
        # Copy testing framework
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/test-installer.ps1"
            "${INSTALLER_OUTPUT_DIR}/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/test-installer.bat"
            "${INSTALLER_OUTPUT_DIR}/"
        DEPENDS vr_binaural_recorder
        COMMENT "Preparing installer files and testing framework"
    )

    # Download Visual C++ Redistributable
    add_custom_target(download_vcredist
        COMMAND powershell -Command "
            if (!(Test-Path '${INSTALLER_OUTPUT_DIR}/vc_redist.x64.exe')) {
                Write-Host 'Downloading Visual C++ Redistributable...'
                Invoke-WebRequest -Uri 'https://aka.ms/vs/17/release/vc_redist.x64.exe' -OutFile '${INSTALLER_OUTPUT_DIR}/vc_redist.x64.exe'
            } else {
                Write-Host 'Visual C++ Redistributable already downloaded'
            }"
        COMMENT "Downloading Visual C++ Redistributable"
    )

    # Create NSIS installer
    add_custom_target(windows_installer
        COMMAND ${NSIS_MAKENSIS}
            /DSOURCE_DIR="${CMAKE_SOURCE_DIR}"
            /DBUILD_DIR="${CMAKE_BINARY_DIR}"
            /DINSTALLER_DIR="${INSTALLER_OUTPUT_DIR}"
            "${INSTALLER_OUTPUT_DIR}/installer.nsi"
        DEPENDS prepare_installer download_vcredist
        WORKING_DIRECTORY "${INSTALLER_OUTPUT_DIR}"
        COMMENT "Creating Windows installer with NSIS"
    )

    # Sign installer if certificate is available
    if(SIGNTOOL_EXECUTABLE AND WINDOWS_CODE_SIGN_CERT)
        add_custom_command(
            TARGET windows_installer POST_BUILD
            COMMAND ${SIGNTOOL_EXECUTABLE}
                sign
                /f "${WINDOWS_CODE_SIGN_CERT}"
                ${SIGN_PASSWORD_ARG}
                /t "http://timestamp.verisign.com/scripts/timstamp.dll"
                /d "VR Binaural Recorder Setup"
                /du "https://github.com/vrbaudio/vr-binaural-recorder"
                "${INSTALLER_OUTPUT_DIR}/VRBinauralRecorder-1.0.0-Setup.exe"
            COMMENT "Code signing installer"
            VERBATIM
        )
    endif()

    # Add installer testing target
    add_custom_target(test_installer
        COMMAND powershell -ExecutionPolicy Bypass -File "${INSTALLER_OUTPUT_DIR}/test-installer.ps1"
            -InstallerPath "${INSTALLER_OUTPUT_DIR}/VRBinauralRecorder-1.0.0-Setup.exe"
            -UninstallAfterTest:$$true
            -Verbose
            -ReportPath "${INSTALLER_OUTPUT_DIR}/installer-test-report.json"
        DEPENDS windows_installer
        WORKING_DIRECTORY "${INSTALLER_OUTPUT_DIR}"
        COMMENT "Running automated installer tests"
    )

    message(STATUS "Windows installer target 'windows_installer' available")
    message(STATUS "Installer testing target 'test_installer' available")
    message(STATUS "NSIS found: ${NSIS_MAKENSIS}")
else()
    message(WARNING "NSIS not found. Windows installer will not be available.")
    message(STATUS "To install NSIS:")
    message(STATUS "  Download from: https://nsis.sourceforge.io/Download")
    message(STATUS "  Or use chocolatey: choco install nsis")
endif()

# Create portable package
add_custom_target(windows_portable
    COMMAND ${CMAKE_COMMAND} -E make_directory "${INSTALLER_OUTPUT_DIR}/VRBinauralRecorder-Portable"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:vr_binaural_recorder>"
        "${INSTALLER_OUTPUT_DIR}/VRBinauralRecorder-Portable/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/hrtf_data"
        "${INSTALLER_OUTPUT_DIR}/VRBinauralRecorder-Portable/hrtf_data"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/config/vr_binaural_config.json.example"
        "${INSTALLER_OUTPUT_DIR}/VRBinauralRecorder-Portable/vr_binaural_config.json"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/README.md"
        "${INSTALLER_OUTPUT_DIR}/VRBinauralRecorder-Portable/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/LICENSE"
        "${INSTALLER_OUTPUT_DIR}/VRBinauralRecorder-Portable/"
    # Copy OpenVR DLL for portable package
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/third_party/openvr/bin/win64/openvr_api.dll"
        "${INSTALLER_OUTPUT_DIR}/VRBinauralRecorder-Portable/"
    DEPENDS vr_binaural_recorder
    COMMENT "Creating portable Windows package with OpenVR runtime"
)

# Create ZIP archive of portable package
find_program(POWERSHELL_EXECUTABLE
    NAMES powershell
    DOC "PowerShell executable"
)

if(POWERSHELL_EXECUTABLE)
    add_custom_target(windows_zip
        COMMAND ${POWERSHELL_EXECUTABLE} -Command "
            Compress-Archive -Path '${INSTALLER_OUTPUT_DIR}/VRBinauralRecorder-Portable/*'
            -DestinationPath '${INSTALLER_OUTPUT_DIR}/VRBinauralRecorder-1.0.0-Portable.zip'
            -Force"
        DEPENDS windows_portable
        COMMENT "Creating ZIP archive of portable package"
    )
else()
    message(WARNING "PowerShell not found. ZIP archive creation not available.")
endif()

# MSI installer using WiX (optional)
find_program(WIX_CANDLE
    NAMES candle
    PATHS
        "C:/Program Files (x86)/WiX Toolset v3.11/bin"
        "$ENV{WIX}/bin"
    DOC "WiX candle compiler"
)

find_program(WIX_LIGHT
    NAMES light
    PATHS
        "C:/Program Files (x86)/WiX Toolset v3.11/bin"
        "$ENV{WIX}/bin"
    DOC "WiX light linker"
)

if(WIX_CANDLE AND WIX_LIGHT)
    # Configure WiX source file
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/installer.wxs"
        "${INSTALLER_OUTPUT_DIR}/installer.wxs"
        @ONLY
    )

    add_custom_target(windows_msi
        COMMAND ${WIX_CANDLE} -o "${INSTALLER_OUTPUT_DIR}/installer.wixobj" "${INSTALLER_OUTPUT_DIR}/installer.wxs"
        COMMAND ${WIX_LIGHT} -o "${INSTALLER_OUTPUT_DIR}/VRBinauralRecorder-1.0.0.msi" "${INSTALLER_OUTPUT_DIR}/installer.wixobj"
        DEPENDS prepare_installer
        COMMENT "Creating MSI installer with WiX"
    )

    message(STATUS "WiX MSI installer target 'windows_msi' available")
    message(STATUS "WiX found: ${WIX_CANDLE}, ${WIX_LIGHT}")
endif()

# Create Windows build info file
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/build-info.json.in")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/build-info.json.in"
        "${INSTALLER_OUTPUT_DIR}/build-info.json"
        @ONLY
    )
endif()

# Summary
message(STATUS "Windows packaging configured:")
message(STATUS "  Output directory: ${INSTALLER_OUTPUT_DIR}")
message(STATUS "  Available targets:")
message(STATUS "    - windows_portable: Portable package")
if(POWERSHELL_EXECUTABLE)
    message(STATUS "    - windows_zip: ZIP archive")
endif()
if(NSIS_MAKENSIS)
    message(STATUS "    - windows_installer: NSIS installer")
endif()
if(WIX_CANDLE AND WIX_LIGHT)
    message(STATUS "    - windows_msi: MSI installer")
endif()
if(SIGNTOOL_EXECUTABLE AND WINDOWS_CODE_SIGN_CERT)
    message(STATUS "  Code signing: Enabled")
else()
    message(STATUS "  Code signing: Disabled (certificate not configured)")
endif()