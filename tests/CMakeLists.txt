# tests/CMakeLists.txt - Comprehensive Test Suite Configuration

# Jordan (QA Guardian): Compilation Fixes Validation Suite
# ImGui sources for UI tests
set(IMGUI_SOURCES_TEST
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
)

add_executable(compilation_fixes_validation
    compilation_fixes_validation.cpp
    # Include actual implementation files
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/src/application.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/src/config.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/src/logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/audio/audio_engine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/audio/hrtf_processor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/vr/vr_tracker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/ui/audio_routing_overlay.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/common/utils.cpp
    ${IMGUI_SOURCES_TEST}
)

target_include_directories(compilation_fixes_validation PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/vr
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/common
    ${OPENVR_INCLUDE_DIRS}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${GLEW_INCLUDE_DIRS}
    ${glfw_SOURCE_DIR}/include
)

target_link_libraries(compilation_fixes_validation PRIVATE
    gtest
    gtest_main
    spdlog::spdlog
    jsoncpp_interface
    portaudio_static
    Threads::Threads
    ${OPENVR_LIBRARIES}
    glfw
    ${GLEW_LIBRARIES}
)

# Add OpenGL libraries (needed for audio_routing_overlay)
find_package(OpenGL REQUIRED)
target_link_libraries(compilation_fixes_validation PRIVATE
    ${OPENGL_LIBRARIES}
)

# Add platform-specific OpenGL libraries for tests (Linux)
if(UNIX AND NOT APPLE)
    target_link_libraries(compilation_fixes_validation PRIVATE
        ${X11_LIBRARIES}
        GL
        GLU
    )
endif()

target_compile_definitions(compilation_fixes_validation PRIVATE
    VR_TESTING_MODE=1
)

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
    jsoncpp_interface
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
    jsoncpp_interface
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
    jsoncpp_interface
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
    jsoncpp_interface
    portaudio_static
    ${OPENVR_LIBRARIES}
    Threads::Threads
)

# Alex's Audio Cockpit Final Validation
add_executable(audio_cockpit_validation
    audio_cockpit_validation.cpp
)

target_include_directories(audio_cockpit_validation PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/vr
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/common
    ${OPENVR_INCLUDE_DIRS}
)

target_link_libraries(audio_cockpit_validation PRIVATE
    gtest
    gtest_main
    spdlog::spdlog
    jsoncpp_interface
    Threads::Threads
)

target_compile_definitions(audio_cockpit_validation PRIVATE
    VR_TESTING_MODE=1
)

# Sam Rivera (QA Guardian): DEPLOYMENT BLOCKING TESTS - spatial_audio_validation_BLOCKING.cpp
add_executable(spatial_audio_validation_BLOCKING
    spatial_audio_validation_BLOCKING.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/audio/hrtf_processor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/src/logger.cpp
)

target_include_directories(spatial_audio_validation_BLOCKING PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/vr
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/common
)

target_link_libraries(spatial_audio_validation_BLOCKING PRIVATE
    gtest
    gtest_main
    spdlog::spdlog
    jsoncpp_interface
    Threads::Threads
)

target_compile_definitions(spatial_audio_validation_BLOCKING PRIVATE
    VR_TESTING_MODE=1
)

# Alex Kim (Creative Coder): CEO SPATIAL VALIDATION - ceo_spatial_validation.cpp
add_executable(ceo_spatial_validation
    ceo_spatial_validation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/audio/hrtf_processor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/src/logger.cpp
)

target_include_directories(ceo_spatial_validation PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/vr
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/common
)

target_link_libraries(ceo_spatial_validation PRIVATE
    gtest
    gtest_main
    spdlog::spdlog
    jsoncpp_interface
    Threads::Threads
)

target_compile_definitions(ceo_spatial_validation PRIVATE
    VR_TESTING_MODE=1
)

# Register tests with CTest
add_test(NAME CompilationFixesValidation COMMAND compilation_fixes_validation)
add_test(NAME WindowsRCValidation COMMAND windows_rc_validation)
add_test(NAME AudioPerformanceTests COMMAND audio_performance_tests)
add_test(NAME VRHeadsetTests COMMAND vr_headset_tests)
add_test(NAME IntegrationTests COMMAND integration_tests)
add_test(NAME AudioCockpitValidation COMMAND audio_cockpit_validation)
add_test(NAME SpatialAudioValidationBLOCKING COMMAND spatial_audio_validation_BLOCKING)
add_test(NAME CEOSpatialValidation COMMAND ceo_spatial_validation)

# Set test properties
set_tests_properties(CompilationFixesValidation PROPERTIES
    TIMEOUT 60
    LABELS "compilation;fixes;validation;critical"
)

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

set_tests_properties(AudioCockpitValidation PROPERTIES
    TIMEOUT 60
    LABELS "ui;audio;cockpit;alex;critical"
)

set_tests_properties(SpatialAudioValidationBLOCKING PROPERTIES
    TIMEOUT 60
    LABELS "spatial;audio;blocking;qa;critical;wolf-prevention"
)

set_tests_properties(CEOSpatialValidation PROPERTIES
    TIMEOUT 60
    LABELS "spatial;audio;ceo;validation;critical"
)