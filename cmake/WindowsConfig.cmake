# WindowsConfig.cmake - Windows-specific build configuration for VR Binaural Recorder
# Professional Windows build system for production deployment

if(NOT WIN32)
    message(FATAL_ERROR "WindowsConfig.cmake should only be included on Windows platforms")
endif()

# Windows version targeting - Windows 10 1903 and later for WASAPI improvements
set(CMAKE_SYSTEM_VERSION "10.0.18362.0")
add_compile_definitions(
    WINVER=0x0A00
    _WIN32_WINNT=0x0A00
    NTDDI_VERSION=0x0A000006
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

# MSVC-specific optimizations
if(MSVC)
    # Enable all CPU optimizations
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /Oi /Ot /GL /arch:AVX2")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG /OPT:REF /OPT:ICF")

    # Enable whole program optimization
    set_property(GLOBAL PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

    # Multi-processor compilation
    add_compile_options(/MP)

    # Enhanced debugging info for production
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Zi /O2 /Ob1")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /DEBUG /OPT:REF")

    # Static runtime linking for deployment
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    # Warning level and treating warnings as errors
    add_compile_options(/W4 /WX)
    add_compile_options(/wd4251) # Disable DLL interface warning
    add_compile_options(/wd4275) # Disable non-DLL interface base class warning

    # Note: WIN32_EXECUTABLE property will be set in configure_windows_target() function

elseif(MINGW)
    # MinGW-specific optimizations
    add_compile_options(-march=native -mtune=native)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto -ffast-math")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto -s")

    # Static linking for deployment
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()

# Windows-specific compile definitions
add_compile_definitions(
    VRB_PLATFORM_WINDOWS=1
    VRB_ENABLE_WASAPI=1
    VRB_ENABLE_ASIO=1
    VRB_ENABLE_DIRECTSOUND=1
    _CRT_SECURE_NO_WARNINGS
    _UNICODE
    UNICODE
    _USE_MATH_DEFINES  # Define M_PI constant for <cmath>
)

# Windows Audio APIs
set(WINDOWS_AUDIO_LIBS
    winmm
    dsound
    ole32
    oleaut32
    advapi32
    shell32
    user32
    gdi32
    dwmapi
    avrt
    ws2_32
    setupapi
    cfgmgr32
)

# Windows Graphics APIs
set(WINDOWS_GRAPHICS_LIBS
    opengl32
    glu32
    gdi32
    comctl32
)

# Windows System APIs
set(WINDOWS_SYSTEM_LIBS
    kernel32
    ntdll
    psapi
    version
    shlwapi
)

# Combine all Windows libraries
set(WINDOWS_PLATFORM_LIBS
    ${WINDOWS_AUDIO_LIBS}
    ${WINDOWS_GRAPHICS_LIBS}
    ${WINDOWS_SYSTEM_LIBS}
)

# Function to configure Windows-specific target properties
function(configure_windows_target target_name)
    # Set Windows-specific properties
    set_target_properties(${target_name} PROPERTIES
        WIN32_EXECUTABLE TRUE
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )

    # Link Windows platform libraries
    target_link_libraries(${target_name} PRIVATE ${WINDOWS_PLATFORM_LIBS})

    # Set application manifest for DPI awareness and Windows compatibility
    if(MSVC)
        set_target_properties(${target_name} PROPERTIES
            VS_USER_PROPS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/windows/app.manifest"
        )
    endif()

    # Set application icon
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/app.rc")
        target_sources(${target_name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/app.rc")
    endif()
endfunction()

# Windows-specific dependency paths
set(WINDOWS_DEPENDENCY_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/windows")

# GLEW for Windows
if(NOT TARGET GLEW::GLEW)
    set(GLEW_ROOT "${WINDOWS_DEPENDENCY_ROOT}/glew")
    if(EXISTS "${GLEW_ROOT}")
        add_library(GLEW::GLEW STATIC IMPORTED)
        set_target_properties(GLEW::GLEW PROPERTIES
            IMPORTED_LOCATION "${GLEW_ROOT}/lib/Release/x64/glew32s.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${GLEW_ROOT}/include"
            INTERFACE_COMPILE_DEFINITIONS "GLEW_STATIC"
        )
    endif()
endif()

# ASIO SDK integration
set(ASIO_SDK_ROOT "${WINDOWS_DEPENDENCY_ROOT}/asiosdk")
if(EXISTS "${ASIO_SDK_ROOT}")
    add_compile_definitions(PA_USE_ASIO=1)
    add_library(asio_sdk INTERFACE)
    target_include_directories(asio_sdk INTERFACE
        "${ASIO_SDK_ROOT}/common"
        "${ASIO_SDK_ROOT}/host"
        "${ASIO_SDK_ROOT}/host/pc"
    )
endif()

# Windows Runtime (WinRT) for modern Windows APIs
if(MSVC AND CMAKE_SYSTEM_VERSION VERSION_GREATER_EQUAL "10.0")
    add_compile_definitions(VRB_ENABLE_WINRT=1)
    list(APPEND WINDOWS_PLATFORM_LIBS windowsapp)
endif()

# Windows Performance Toolkit integration
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_compile_definitions(VRB_ENABLE_ETW=1)
endif()

# Code signing setup (for production)
set(WINDOWS_CODE_SIGN_CERT "" CACHE STRING "Path to code signing certificate")
set(WINDOWS_CODE_SIGN_PASSWORD "" CACHE STRING "Code signing certificate password")

if(WINDOWS_CODE_SIGN_CERT AND EXISTS "${WINDOWS_CODE_SIGN_CERT}")
    message(STATUS "Code signing certificate found: ${WINDOWS_CODE_SIGN_CERT}")
    set(VRB_ENABLE_CODE_SIGNING TRUE)
endif()

# Windows installer configuration
set(WINDOWS_INSTALLER_TYPE "NSIS" CACHE STRING "Windows installer type (NSIS or WIX)")
set_property(CACHE WINDOWS_INSTALLER_TYPE PROPERTY STRINGS "NSIS" "WIX")

message(STATUS "Windows build configuration loaded:")
message(STATUS "  - Target Windows version: ${CMAKE_SYSTEM_VERSION}")
message(STATUS "  - Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  - Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  - ASIO SDK: ${ASIO_SDK_ROOT}")
message(STATUS "  - Code signing: ${VRB_ENABLE_CODE_SIGNING}")
message(STATUS "  - Installer type: ${WINDOWS_INSTALLER_TYPE}")