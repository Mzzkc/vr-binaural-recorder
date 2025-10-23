# CMakeLists.txt for Windows Dependency Verification Tests
# Created by Mike Rodriguez for Windows build verification

cmake_minimum_required(VERSION 3.16)
project(DependencyVerificationTests)

# Use same C++ standard as main project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows-specific configuration
if(WIN32)
    # Include Windows configuration
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/WindowsConfig.cmake")
        include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/WindowsConfig.cmake")
    endif()

    # Windows-specific compile definitions
    add_compile_definitions(
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _WIN32_WINNT=0x0601
    )
endif()

# Find required packages
find_package(Threads REQUIRED)

# External dependencies (same as main project)
include(FetchContent)

# jsoncpp for config testing
FetchContent_Declare(
    jsoncpp
    GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
    GIT_TAG 1.9.5
)

# PortAudio for audio testing
FetchContent_Declare(
    portaudio
    GIT_REPOSITORY https://github.com/PortAudio/portaudio.git
    GIT_TAG v19.7.0
)

# Configure PortAudio for Windows
set(PA_BUILD_SHARED OFF CACHE BOOL "Build shared library" FORCE)
if(WIN32)
    set(PA_USE_WASAPI ON CACHE BOOL "Enable WASAPI" FORCE)
    # ASIO is optional - disable for now since SDK may not be available
    set(PA_USE_ASIO OFF CACHE BOOL "Enable ASIO" FORCE)
    set(PA_USE_DIRECTSOUND ON CACHE BOOL "Enable DirectSound" FORCE)
endif()

# Make dependencies available
FetchContent_MakeAvailable(jsoncpp portaudio)

# Test 1: jsoncpp verification
add_executable(test_jsoncpp test_jsoncpp.cpp)

target_link_libraries(test_jsoncpp PRIVATE
    jsoncpp_lib
    Threads::Threads
)

if(WIN32)
    target_link_libraries(test_jsoncpp PRIVATE
        kernel32
        user32
        gdi32
        ole32
        oleaut32
    )
endif()

# Test 2: PortAudio verification
add_executable(test_portaudio test_portaudio.cpp)

target_link_libraries(test_portaudio PRIVATE
    portaudio_static
    Threads::Threads
)

if(WIN32)
    target_link_libraries(test_portaudio PRIVATE
        winmm
        ole32
        oleaut32
        advapi32
        user32
        kernel32
        ws2_32
        setupapi
    )
endif()

# Test 3: HRTF verification (standalone - no external deps needed)
add_executable(test_hrtf test_hrtf.cpp)

target_link_libraries(test_hrtf PRIVATE
    Threads::Threads
)

if(WIN32)
    target_link_libraries(test_hrtf PRIVATE
        kernel32
        user32
    )
endif()

# Windows-specific target configuration
if(WIN32 AND COMMAND configure_windows_target)
    configure_windows_target(test_jsoncpp)
    configure_windows_target(test_portaudio)
    configure_windows_target(test_hrtf)
endif()

# Test targets for CTest
enable_testing()

add_test(NAME jsoncpp_verification COMMAND test_jsoncpp)
add_test(NAME portaudio_verification COMMAND test_portaudio)
add_test(NAME hrtf_verification COMMAND test_hrtf)

# Set test properties
set_tests_properties(jsoncpp_verification PROPERTIES
    TIMEOUT 30
    LABELS "dependency;config;windows"
)

set_tests_properties(portaudio_verification PROPERTIES
    TIMEOUT 60
    LABELS "dependency;audio;windows"
)

set_tests_properties(hrtf_verification PROPERTIES
    TIMEOUT 45
    LABELS "dependency;hrtf;spatial;windows"
)

# Custom target to run all dependency tests
add_custom_target(verify_dependencies
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_jsoncpp test_portaudio test_hrtf
    COMMENT "Running Windows dependency verification tests"
)

# Installation (for testing deployment)
install(TARGETS test_jsoncpp test_portaudio test_hrtf
    RUNTIME DESTINATION bin/tests
    COMPONENT DependencyTests
)

# Copy test configuration files
if(EXISTS "${CMAKE_SOURCE_DIR}/config/vr_binaural_config.json.example")
    configure_file(
        "${CMAKE_SOURCE_DIR}/config/vr_binaural_config.json.example"
        "${CMAKE_CURRENT_BINARY_DIR}/test_config.json"
        COPYONLY
    )
endif()

# Summary
message(STATUS "Dependency verification tests configured:")
message(STATUS "  - test_jsoncpp: Config parsing and validation")
message(STATUS "  - test_portaudio: Windows WASAPI audio functionality")
message(STATUS "  - test_hrtf: Spatial audio and fallback system")
message(STATUS "  - Use 'make verify_dependencies' to run all tests")

if(WIN32)
    message(STATUS "Windows-specific features enabled:")
    message(STATUS "  - WASAPI audio support")
    message(STATUS "  - COM initialization for audio")
    message(STATUS "  - Windows path handling")
    if(PA_USE_ASIO)
        message(STATUS "  - ASIO audio support")
    endif()
endif()