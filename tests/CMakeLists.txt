# tests/CMakeLists.txt - Comprehensive Test Suite Configuration

# Windows RC Validation Suite
add_executable(windows_rc_validation
    windows_rc_validation.cpp
)

target_include_directories(windows_rc_validation PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/vr
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/common
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/common/simd
    ${OPENVR_INCLUDE_DIRS}
)

target_link_libraries(windows_rc_validation PRIVATE
    gtest
    gtest_main
    spdlog::spdlog
    jsoncpp_lib
    Threads::Threads
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(windows_rc_validation PRIVATE
        ${WINDOWS_PLATFORM_LIBS}
        setupapi
        cfgmgr32
        psapi
        pdh
        version
    )
    target_compile_definitions(windows_rc_validation PRIVATE
        VRB_PLATFORM_WINDOWS=1
        VRB_ENABLE_WASAPI=1
        VRB_ENABLE_ASIO=1
    )
endif()

# Audio Engine Performance Tests
add_executable(audio_performance_tests
    audio_performance_tests.cpp
)

target_include_directories(audio_performance_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/common
)

target_link_libraries(audio_performance_tests PRIVATE
    gtest
    gtest_main
    spdlog::spdlog
    jsoncpp_lib
    portaudio_static
    Threads::Threads
)

# VR Headset Compatibility Tests
add_executable(vr_headset_tests
    vr_headset_compatibility_tests.cpp
)

target_include_directories(vr_headset_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/vr
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/audio
    ${OPENVR_INCLUDE_DIRS}
)

target_link_libraries(vr_headset_tests PRIVATE
    gtest
    gtest_main
    spdlog::spdlog
    jsoncpp_lib
    ${OPENVR_LIBRARIES}
    Threads::Threads
)

# Integration Tests
add_executable(integration_tests
    integration_tests.cpp
)

target_include_directories(integration_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/vr
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/common
)

target_link_libraries(integration_tests PRIVATE
    gtest
    gtest_main
    spdlog::spdlog
    jsoncpp_lib
    portaudio_static
    ${OPENVR_LIBRARIES}
    Threads::Threads
)

# Register tests with CTest
add_test(NAME WindowsRCValidation COMMAND windows_rc_validation)
add_test(NAME AudioPerformanceTests COMMAND audio_performance_tests)
add_test(NAME VRHeadsetTests COMMAND vr_headset_tests)
add_test(NAME IntegrationTests COMMAND integration_tests)

# Set test properties
set_tests_properties(WindowsRCValidation PROPERTIES
    TIMEOUT 300
    LABELS "windows;rc;validation"
)

set_tests_properties(AudioPerformanceTests PROPERTIES
    TIMEOUT 120
    LABELS "audio;performance"
)

set_tests_properties(VRHeadsetTests PROPERTIES
    TIMEOUT 60
    LABELS "vr;compatibility"
)

set_tests_properties(IntegrationTests PROPERTIES
    TIMEOUT 180
    LABELS "integration;system"
)