# Standalone tests for VR Binaural Recorder
# These tests can be built and run independently from the main project
cmake_minimum_required(VERSION 3.16)
project(VRBinauralStandaloneTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(Threads REQUIRED)

# External dependencies
include(FetchContent)

# PortAudio
FetchContent_Declare(
    portaudio
    GIT_REPOSITORY https://github.com/PortAudio/portaudio.git
    GIT_TAG v19.7.0
)
set(PA_BUILD_SHARED OFF CACHE BOOL "Build shared library" FORCE)

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

FetchContent_MakeAvailable(portaudio spdlog jsoncpp)

# Audio engine test sources
set(AUDIO_TEST_SOURCES
    test_audio_engine.cpp
    ${CMAKE_SOURCE_DIR}/core/src/audio_engine.cpp
    ${CMAKE_SOURCE_DIR}/core/src/hrtf_processor.cpp
    ${CMAKE_SOURCE_DIR}/utils/src/logger.cpp
)

# VR tracker test sources
set(VR_TEST_SOURCES
    test_vr_tracker.cpp
    ${CMAKE_SOURCE_DIR}/vr/src/vr_tracker.cpp
    ${CMAKE_SOURCE_DIR}/utils/src/logger.cpp
)

# Build test executables
add_executable(test_audio_engine ${AUDIO_TEST_SOURCES})
add_executable(test_vr_tracker ${VR_TEST_SOURCES})

# Include directories for tests
target_include_directories(test_audio_engine PRIVATE
    ${CMAKE_SOURCE_DIR}/core/include
    ${CMAKE_SOURCE_DIR}/utils/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(test_vr_tracker PRIVATE
    ${CMAKE_SOURCE_DIR}/vr/include
    ${CMAKE_SOURCE_DIR}/utils/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link libraries for audio test
target_link_libraries(test_audio_engine PRIVATE
    Threads::Threads
    portaudio_static
    spdlog::spdlog
    jsoncpp_lib
)

# Link libraries for VR test
target_link_libraries(test_vr_tracker PRIVATE
    Threads::Threads
    spdlog::spdlog
    jsoncpp_lib
    ${OpenVR_LIBRARIES}
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(test_audio_engine PRIVATE winmm ws2_32)
    target_link_libraries(test_vr_tracker PRIVATE winmm ws2_32)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(test_audio_engine PRIVATE m)
    target_link_libraries(test_vr_tracker PRIVATE m)
endif()

# Add tests to CTest
enable_testing()
add_test(NAME AudioEngineTest COMMAND test_audio_engine)
add_test(NAME VRTrackerTest COMMAND test_vr_tracker)