# CMakeLists.txt - Build configuration for VR Binaural Recorder
cmake_minimum_required(VERSION 3.16)
project(VRBinauralRecorder VERSION 1.0.0 LANGUAGES C CXX)

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
    set(PA_USE_ASIO ON CACHE BOOL "Enable ASIO" FORCE)
    set(PA_USE_WASAPI ON CACHE BOOL "Enable WASAPI" FORCE)
endif()

# spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.11.0
)

# jsoncpp
FetchContent_Declare(
    jsoncpp
    GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
    GIT_TAG 1.9.5
)

# OpenVR - Use stub headers for development
set(OPENVR_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/openvr_stub")
set(OPENVR_INCLUDE_DIRS "${OPENVR_ROOT_DIR}/headers")

# Using stub implementation - no libraries needed for compilation
set(OPENVR_LIBRARIES "")

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

FetchContent_MakeAvailable(portaudio spdlog jsoncpp imgui glfw)

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

set(VR_SOURCES
    modules/vr/vr_tracker.cpp
)

set(UI_SOURCES
    modules/ui/overlay_ui.cpp
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

set(VR_HEADERS
    modules/vr/vr_tracker.h
)

set(UI_HEADERS
    modules/ui/overlay_ui.h
)

set(COMMON_HEADERS
    modules/common/utils.h
    modules/common/simd/audio_simd.h
)

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

# Main executable
add_executable(vr_binaural_recorder ${SOURCES} ${HEADERS} ${IMGUI_SOURCES})

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
    jsoncpp_lib
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
        "${OPENVR_ROOT_DIR}/bin/win64/openvr_api.dll"
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
    modules/ui/overlay_ui.cpp
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
    jsoncpp_lib
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

# Add standalone tests subdirectory
add_subdirectory(tests/standalone)

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