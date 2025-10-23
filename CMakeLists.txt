# CMakeLists.txt - Build configuration for VR Binaural Recorder
cmake_minimum_required(VERSION 3.16)
project(VRBinauralRecorder VERSION 1.0.0 LANGUAGES C CXX)

# Windows-specific configuration
if(WIN32)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/WindowsConfig.cmake")
endif()

# vcpkg integration
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/vcpkg-toolchain.cmake")
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/vcpkg-toolchain.cmake")
endif()

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /WX)
    add_compile_options(/MP)  # Multi-processor compilation
    add_compile_options(/arch:AVX2)  # Enable AVX2 for SIMD
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/O2 /GL)
        add_link_options(/LTCG)
    endif()
else()
    add_compile_options(-Wall -Wextra)
    # Note: Removed -Wpedantic and -Werror to avoid issues with third-party dependencies
    add_compile_options(-march=native)  # Optimize for current CPU
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -flto)
    endif()
endif()

# Find packages
find_package(Threads REQUIRED)

# External dependencies
include(FetchContent)

# PortAudio with ASIO support on Windows
FetchContent_Declare(
    portaudio
    GIT_REPOSITORY https://github.com/PortAudio/portaudio.git
    GIT_TAG v19.7.0
)
set(PA_BUILD_SHARED OFF CACHE BOOL "Build shared library" FORCE)
if(WIN32)
    # Use only WMME (Windows Multimedia Extensions) - simplest, most compatible
    set(PA_USE_ASIO OFF CACHE BOOL "Enable ASIO" FORCE)
    set(PA_USE_WASAPI OFF CACHE BOOL "Disable WASAPI" FORCE)
    set(PA_USE_DS OFF CACHE BOOL "Disable DirectSound" FORCE)
    set(PA_USE_WDMKS OFF CACHE BOOL "Disable WDM-KS" FORCE)
    set(PA_USE_WMME ON CACHE BOOL "Enable WMME" FORCE)
endif()

# Workaround for PortAudio's old CMake version requirement
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "Minimum CMake policy version" FORCE)

# spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.11.0
)

# jsoncpp - Use system package to avoid FetchContent in-source build check issues
find_package(PkgConfig QUIET)
set(JSONCPP_USE_SYSTEM OFF)
if(PkgConfig_FOUND)
    pkg_check_modules(JSONCPP jsoncpp)
    if(JSONCPP_FOUND)
        set(JSONCPP_USE_SYSTEM ON)
        message(STATUS "Using system jsoncpp ${JSONCPP_VERSION}")
        # Create INTERFACE library for uniform linking
        add_library(jsoncpp_interface INTERFACE)
        target_link_libraries(jsoncpp_interface INTERFACE ${JSONCPP_LIBRARIES})
        target_include_directories(jsoncpp_interface INTERFACE ${JSONCPP_INCLUDE_DIRS})
        target_compile_options(jsoncpp_interface INTERFACE ${JSONCPP_CFLAGS_OTHER})
    endif()
endif()

if(NOT JSONCPP_USE_SYSTEM)
    message(STATUS "Using FetchContent jsoncpp")
    # Fallback to FetchContent if system package not available
    FetchContent_Declare(
        jsoncpp
        GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
        GIT_TAG 1.9.5
    )
    set(JSONCPP_WITH_TESTS OFF CACHE BOOL "" FORCE)
    set(JSONCPP_WITH_POST_BUILD_UNITTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
    # Create INTERFACE library alias for uniform linking
    add_library(jsoncpp_interface INTERFACE)
endif()

# OpenVR - Real SDK integration for VR head tracking
# CEO directive: VR tracking IS the product differentiator, not manual positioning
set(OPENVR_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/openvr")
# Use headers directory directly (include is a symlink that doesn't work on Windows)
if(EXISTS "${OPENVR_ROOT_DIR}/include")
    set(OPENVR_INCLUDE_DIRS "${OPENVR_ROOT_DIR}/include")
else()
    set(OPENVR_INCLUDE_DIRS "${OPENVR_ROOT_DIR}/headers")
endif()

if(WIN32)
    set(OPENVR_LIBRARIES "${OPENVR_ROOT_DIR}/lib/win64/openvr_api.lib")
    set(OPENVR_BINARIES "${OPENVR_ROOT_DIR}/bin/win64/openvr_api.dll")
    message(STATUS "Using OpenVR SDK for Windows: ${OPENVR_LIBRARIES}")
elseif(UNIX AND NOT APPLE)
    set(OPENVR_LIBRARIES "${OPENVR_ROOT_DIR}/lib/linux64/libopenvr_api.so")
    message(STATUS "Using OpenVR SDK for Linux: ${OPENVR_LIBRARIES}")
else()
    message(FATAL_ERROR "OpenVR platform not supported")
endif()

# REMOVED: add_compile_definitions(OPENVR_STUB_BUILD)
# We're implementing real VR functionality now - no more stubs

# ImGui with OpenGL backend
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.5
)

# GLFW for OpenGL context (overlay rendering)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# GLEW for OpenGL extensions
if(WIN32)
    set(GLEW_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/glew")
    set(GLEW_INCLUDE_DIRS "${GLEW_ROOT}/include")
    set(GLEW_LIBRARIES "${GLEW_ROOT}/lib/Release/x64/glew32.lib")
else()
    find_package(GLEW)
    if(NOT GLEW_FOUND)
        message(WARNING "GLEW not found, VR overlay functionality may be limited")
        set(GLEW_INCLUDE_DIRS "")
        set(GLEW_LIBRARIES "")
    endif()
endif()

if(JSONCPP_USE_SYSTEM)
    FetchContent_MakeAvailable(portaudio spdlog imgui glfw)
else()
    FetchContent_MakeAvailable(portaudio spdlog jsoncpp imgui glfw)
    # Link the FetchContent target to the interface for uniform usage
    target_link_libraries(jsoncpp_interface INTERFACE jsoncpp_lib)
endif()

# Disable warnings-as-errors for third-party PortAudio library on Windows
# Also remove WIN32_LEAN_AND_MEAN for PortAudio to get full Windows SDK headers
if(WIN32 AND MSVC AND TARGET portaudio_static)
    target_compile_options(portaudio_static PRIVATE /WX-)
    # PortAudio WASAPI needs full Windows SDK headers - undefine WIN32_LEAN_AND_MEAN
    get_target_property(PA_COMPILE_DEFS portaudio_static COMPILE_DEFINITIONS)
    if(PA_COMPILE_DEFS)
        list(REMOVE_ITEM PA_COMPILE_DEFS WIN32_LEAN_AND_MEAN)
        set_target_properties(portaudio_static PROPERTIES COMPILE_DEFINITIONS "${PA_COMPILE_DEFS}")
    endif()
endif()

# Source files organized by module
set(CORE_SOURCES
    core/src/main.cpp
    core/src/application.cpp
    core/src/config.cpp
    core/src/logger.cpp
)

set(AUDIO_SOURCES
    modules/audio/audio_engine.cpp
    modules/audio/hrtf_processor.cpp
)

# Windows-specific audio sources
if(WIN32)
    list(APPEND AUDIO_SOURCES
        modules/audio/windows_wasapi_virtual_device.cpp
    )
endif()

set(VR_SOURCES
    modules/vr/vr_tracker.cpp
)

set(UI_SOURCES
    # CREATIVE SOLUTION: Disable stub overlay_ui.cpp - route everything through WORKING audio_routing_overlay.cpp!
    # modules/ui/overlay_ui.cpp  # DISABLED: All stub implementations - causes black screen in VR
    # 🎧 CREATIVE CODER'S AUDIO COCKPIT - NOW ENABLED! 🎧
    modules/ui/audio_routing_overlay.cpp  # REAL VR overlay with working recording controls!
    # NOTE: audio_routing_integration.cpp disabled - interface mismatch with AudioRoutingOverlay
)

set(COMMON_SOURCES
    modules/common/utils.cpp
)

set(SOURCES
    ${CORE_SOURCES}
    ${AUDIO_SOURCES}
    ${VR_SOURCES}
    ${UI_SOURCES}
    ${COMMON_SOURCES}
)

# Header files organized by module
set(CORE_HEADERS
    core/include/application.h
    core/include/config.h
    core/include/logger.h
    core/include/vr_types.h
    core/include/ring_buffer.h
)

set(AUDIO_HEADERS
    modules/audio/audio_engine.h
    modules/audio/hrtf_processor.h
)

# Windows-specific audio headers
if(WIN32)
    list(APPEND AUDIO_HEADERS
        modules/audio/windows_wasapi_virtual_device.h
    )
endif()

set(VR_HEADERS
    modules/vr/vr_tracker.h
)

set(UI_HEADERS
    # modules/ui/overlay_ui.h  # DISABLED: Stub header for black screen overlay
    # 🎧 CREATIVE CODER'S AUDIO COCKPIT HEADER - NOW ENABLED! 🎧
    modules/ui/audio_routing_overlay.h  # REAL VR overlay header with working functionality!
)

set(COMMON_HEADERS
    modules/common/utils.h
    modules/common/simd/audio_simd.h
)

# Windows-specific common headers
if(WIN32)
    list(APPEND COMMON_HEADERS
        modules/common/windows_performance.h
    )
endif()

set(HEADERS
    ${CORE_HEADERS}
    ${AUDIO_HEADERS}
    ${VR_HEADERS}
    ${UI_HEADERS}
    ${COMMON_HEADERS}
)

# ImGui sources
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
)

# Windows resource files
set(WINDOWS_RESOURCES "")
if(WIN32)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/app.rc")
        list(APPEND WINDOWS_RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/app.rc")
    endif()
endif()

# Main executable
add_executable(vr_binaural_recorder ${SOURCES} ${HEADERS} ${IMGUI_SOURCES} ${WINDOWS_RESOURCES})

# Apply Windows-specific target configuration
if(WIN32)
    configure_windows_target(vr_binaural_recorder)
endif()

# Include directories
target_include_directories(vr_binaural_recorder PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/vr
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/common
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/common/simd
    ${OPENVR_INCLUDE_DIRS}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${GLEW_INCLUDE_DIRS}
    ${glfw_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(vr_binaural_recorder PRIVATE
    Threads::Threads
    portaudio_static
    spdlog::spdlog
    jsoncpp_interface
    ${OPENVR_LIBRARIES}
    glfw
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(vr_binaural_recorder PRIVATE
        winmm
        ws2_32
        opengl32
        glu32
        dwmapi
        shell32
    )
    
    # ASIO SDK if available
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/asiosdk")
        target_include_directories(vr_binaural_recorder PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/third_party/asiosdk/common"
        )
        target_compile_definitions(vr_binaural_recorder PRIVATE PA_USE_ASIO=1)
    endif()
    
    # Copy OpenVR and GLEW binaries to output directory
    add_custom_command(TARGET vr_binaural_recorder POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${OPENVR_BINARIES}"
        $<TARGET_FILE_DIR:vr_binaural_recorder>
    )
    
    if(EXISTS "${GLEW_ROOT}/bin/Release/x64/glew32.dll")
        add_custom_command(TARGET vr_binaural_recorder POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${GLEW_ROOT}/bin/Release/x64/glew32.dll"
            $<TARGET_FILE_DIR:vr_binaural_recorder>
        )
    endif()
elseif(UNIX AND NOT APPLE)
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    target_link_libraries(vr_binaural_recorder PRIVATE
        ${OPENGL_LIBRARIES}
        ${X11_LIBRARIES}
        GL
        GLU
    )

    # Test target OpenGL linking will be handled later after target is defined
endif()

# Installation
install(TARGETS vr_binaural_recorder
    RUNTIME DESTINATION bin
)

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/config/vr_binaural_config.json.example"
    DESTINATION bin
    RENAME vr_binaural_config.json
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/deps/hrtf"
    DESTINATION bin
)

# Testing
enable_testing()

# Test executable
add_executable(vr_binaural_tests
    tests/test_suite.cpp
    # Exclude main.cpp from sources for tests to avoid multiple main functions
    core/src/application.cpp
    core/src/config.cpp
    core/src/logger.cpp
    modules/audio/audio_engine.cpp
    modules/audio/hrtf_processor.cpp
    modules/vr/vr_tracker.cpp
    # modules/ui/overlay_ui.cpp  # DISABLED: Stub implementation for tests
    modules/ui/audio_routing_overlay.cpp  # REAL implementation for tests
    # NOTE: audio_routing_integration.cpp disabled - interface mismatch with AudioRoutingOverlay
    modules/common/utils.cpp
    ${IMGUI_SOURCES}
)

target_include_directories(vr_binaural_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/vr
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/common
    ${CMAKE_CURRENT_SOURCE_DIR}/modules/common/simd
    ${OPENVR_INCLUDE_DIRS}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${GLEW_INCLUDE_DIRS}
    ${glfw_SOURCE_DIR}/include
)

target_link_libraries(vr_binaural_tests PRIVATE
    Threads::Threads
    portaudio_static
    spdlog::spdlog
    jsoncpp_interface
    ${OPENVR_LIBRARIES}
    glfw
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
)

# Google Test
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

target_link_libraries(vr_binaural_tests PRIVATE
    gtest
    gtest_main
)

# Add platform-specific OpenGL libraries for tests (Linux)
if(UNIX AND NOT APPLE)
    target_link_libraries(vr_binaural_tests PRIVATE
        ${OPENGL_LIBRARIES}
        ${X11_LIBRARIES}
        GL
        GLU
    )
endif()

add_test(NAME VRBinauralTests COMMAND vr_binaural_tests)

# Add test subdirectories
add_subdirectory(tests/standalone)
add_subdirectory(tests)

# Windows dependency verification tests
add_subdirectory(tests/dependency_verification)

# CPack for packaging
set(CPACK_PACKAGE_NAME "VRBinauralRecorder")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION "Professional VR Spatial Audio Recording")
set(CPACK_PACKAGE_VENDOR "VRB Audio")

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "VR Binaural Recorder")
    set(CPACK_NSIS_HELP_LINK "https://github.com/vrbaudio/vr-binaural-recorder")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libportaudio2")
endif()

include(CPack)

# Windows packaging
if(WIN32)
    add_subdirectory(packaging/windows)
endif()