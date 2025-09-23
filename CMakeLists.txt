# CMakeLists.txt - Build configuration for VR Binaural Recorder
cmake_minimum_required(VERSION 3.16)
project(VRBinauralRecorder VERSION 1.0.0 LANGUAGES CXX)

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
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
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

# OpenVR
if(WIN32)
    set(OPENVR_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/openvr")
    set(OPENVR_LIBRARIES "${OPENVR_ROOT_DIR}/lib/win64/openvr_api.lib")
elseif(APPLE)
    set(OPENVR_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/openvr")
    set(OPENVR_LIBRARIES "${OPENVR_ROOT_DIR}/lib/osx32/libopenvr_api.dylib")
else()
    set(OPENVR_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/openvr")
    set(OPENVR_LIBRARIES "${OPENVR_ROOT_DIR}/lib/linux64/libopenvr_api.so")
endif()
set(OPENVR_INCLUDE_DIRS "${OPENVR_ROOT_DIR}/headers")

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
    find_package(GLEW REQUIRED)
endif()

FetchContent_MakeAvailable(portaudio spdlog jsoncpp imgui glfw)

# Source files
set(SOURCES
    src/main.cpp
    src/audio_engine.cpp
    src/hrtf_processor.cpp
    src/vr_tracker.cpp
    src/overlay_ui.cpp
    src/config.cpp
    src/logger.cpp
)

set(HEADERS
    include/audio_engine.h
    include/hrtf_processor.h
    include/vr_tracker.h
    include/overlay_ui.h
    include/config.h
    include/logger.h
    include/vr_types.h
    include/ring_buffer.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/include
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
endif()

# Installation
install(TARGETS vr_binaural_recorder
    RUNTIME DESTINATION bin
)

install(FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/vr_binaural_config.json.example"
    DESTINATION bin
    RENAME vr_binaural_config.json
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/hrtf_data"
    DESTINATION bin
)

# Testing
enable_testing()

# Test executable
add_executable(vr_binaural_tests
    tests/test_main.cpp
    tests/test_audio_engine.cpp
    tests/test_hrtf_processor.cpp
    tests/test_ring_buffer.cpp
    tests/test_config.cpp
    ${SOURCES}
)

target_include_directories(vr_binaural_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OPENVR_INCLUDE_DIRS}
)

target_link_libraries(vr_binaural_tests PRIVATE
    Threads::Threads
    portaudio_static
    spdlog::spdlog
    jsoncpp_lib
    ${OPENVR_LIBRARIES}
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

add_test(NAME VRBinauralTests COMMAND vr_binaural_tests)

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