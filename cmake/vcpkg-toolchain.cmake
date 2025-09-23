# vcpkg-toolchain.cmake - vcpkg integration for Windows build system

# Set vcpkg root if not already set
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VCPKG_ROOT})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "Vcpkg toolchain file")
    elseif(WIN32)
        # Try common vcpkg locations on Windows
        set(VCPKG_PATHS
            "C:/vcpkg"
            "C:/dev/vcpkg"
            "C:/tools/vcpkg"
            "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg"
            "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg"
        )

        foreach(VCPKG_PATH ${VCPKG_PATHS})
            if(EXISTS "${VCPKG_PATH}/scripts/buildsystems/vcpkg.cmake")
                set(CMAKE_TOOLCHAIN_FILE "${VCPKG_PATH}/scripts/buildsystems/vcpkg.cmake"
                    CACHE STRING "Vcpkg toolchain file")
                break()
            endif()
        endforeach()
    endif()
endif()

# Configure vcpkg for Windows
if(WIN32)
    # Target Windows x64 by default
    if(NOT DEFINED VCPKG_TARGET_TRIPLET)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "Vcpkg target triplet")
        else()
            set(VCPKG_TARGET_TRIPLET "x86-windows" CACHE STRING "Vcpkg target triplet")
        endif()
    endif()

    # Static linking by default for easier deployment
    if(NOT DEFINED VCPKG_LIBRARY_LINKAGE)
        set(VCPKG_LIBRARY_LINKAGE "static" CACHE STRING "Vcpkg library linkage")
    endif()

    # Use static CRT for release builds
    if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "Vcpkg target triplet" FORCE)
    endif()
endif()

# Enable vcpkg manifest mode
set(VCPKG_MANIFEST_MODE ON CACHE BOOL "Use vcpkg manifest")
set(VCPKG_MANIFEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE STRING "Vcpkg manifest directory")

# Install dependencies if vcpkg is available
if(EXISTS "${CMAKE_TOOLCHAIN_FILE}")
    message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
    message(STATUS "Vcpkg target triplet: ${VCPKG_TARGET_TRIPLET}")

    # Enable features based on platform
    if(WIN32)
        list(APPEND VCPKG_MANIFEST_FEATURES "windows-audio")
        list(APPEND VCPKG_MANIFEST_FEATURES "vr-headsets")

        # Enable ASIO if SDK is available
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/asiosdk")
            list(APPEND VCPKG_MANIFEST_FEATURES "asio-sdk")
        endif()

        # Enable installer features for release builds
        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            list(APPEND VCPKG_MANIFEST_FEATURES "installer")
        endif()
    endif()

    set(VCPKG_MANIFEST_FEATURES ${VCPKG_MANIFEST_FEATURES} CACHE STRING "Vcpkg manifest features")

else()
    message(WARNING "vcpkg not found. Please install vcpkg or set VCPKG_ROOT environment variable.")
    message(STATUS "To install vcpkg on Windows:")
    message(STATUS "  git clone https://github.com/Microsoft/vcpkg.git C:/vcpkg")
    message(STATUS "  C:/vcpkg/bootstrap-vcpkg.bat")
    message(STATUS "  C:/vcpkg/vcpkg integrate install")
    message(STATUS "  set VCPKG_ROOT=C:/vcpkg")
endif()

# Function to find vcpkg packages with fallback to system packages
function(find_vcpkg_package PACKAGE_NAME)
    set(options REQUIRED)
    set(oneValueArgs VERSION)
    set(multiValueArgs COMPONENTS)
    cmake_parse_arguments(FIND_PKG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Try to find package via vcpkg first
    find_package(${PACKAGE_NAME} ${FIND_PKG_VERSION} QUIET ${FIND_PKG_COMPONENTS})

    if(NOT ${PACKAGE_NAME}_FOUND)
        # Fall back to system package manager
        find_package(${PACKAGE_NAME} ${FIND_PKG_VERSION} ${FIND_PKG_REQUIRED} ${FIND_PKG_COMPONENTS})
    endif()

    if(${PACKAGE_NAME}_FOUND)
        message(STATUS "Found ${PACKAGE_NAME}: ${${PACKAGE_NAME}_VERSION}")
    elseif(FIND_PKG_REQUIRED)
        message(FATAL_ERROR "Required package ${PACKAGE_NAME} not found")
    endif()
endfunction()

# Windows-specific package configuration
if(WIN32)
    # Ensure we use the correct Windows SDK version
    if(NOT CMAKE_SYSTEM_VERSION)
        set(CMAKE_SYSTEM_VERSION "10.0.19041.0" CACHE STRING "Windows SDK version")
    endif()

    # Set up Windows-specific compiler flags for vcpkg packages
    if(MSVC)
        # Ensure compatibility with vcpkg packages
        add_compile_definitions(_WIN32_WINNT=0x0A00)
        add_compile_definitions(WINVER=0x0A00)
    endif()
endif()

message(STATUS "vcpkg configuration complete")
message(STATUS "  Manifest mode: ${VCPKG_MANIFEST_MODE}")
message(STATUS "  Manifest directory: ${VCPKG_MANIFEST_DIR}")
if(DEFINED VCPKG_MANIFEST_FEATURES)
    message(STATUS "  Enabled features: ${VCPKG_MANIFEST_FEATURES}")
endif()