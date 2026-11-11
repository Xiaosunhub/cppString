# SPDX-License-Identifier: MIT
# Copyright (c) 2025-2026 Ender_

cmake_minimum_required(VERSION 3.20)

project(cppString
    VERSION 5.6.0
    DESCRIPTION "A modern C++23 string utility library"
    HOMEPAGE_URL "https://github.com/Xiaosunhub/cppString"
    LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Public headers live in include/cppString/, implementations in src/.
# Only the headers are installed, so consumers add just <prefix>/include to
# their include path and write #include <cppString/xxx.h>.
file(GLOB CPPSTRING_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB CPPSTRING_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/cppString/*.h")

if(NOT CPPSTRING_SOURCES)
    message(FATAL_ERROR "No cppString sources found in src/")
endif()

add_library(cppString ${CPPSTRING_SOURCES})
add_library(cppString::cppString ALIAS cppString)

target_compile_features(cppString PUBLIC cxx_std_23)
target_include_directories(cppString
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        # Implementation files include their own headers by bare name.
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/cppString>)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(cppString PUBLIC CPPSTRING_BUILD_SHARED)
    target_compile_definitions(cppString PRIVATE CPPSTRING_EXPORTS)
    set_target_properties(cppString PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON)
endif()

set_target_properties(cppString PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    POSITION_INDEPENDENT_CODE ON)

if(MSVC)
    target_compile_options(cppString PRIVATE /utf-8 /W4 /WX-)
else()
    target_compile_options(cppString PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()

# GCC before 9 keeps std::filesystem in a separate library.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(cppString PUBLIC stdc++fs)
endif()

# ---- installation / package config (vcpkg friendly) ----

install(TARGETS cppString
    EXPORT cppStringTargets
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${CPPSTRING_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cppString)

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/cppString
    RENAME copyright)

set(CPPSTRING_CMAKE_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/cppString")

install(EXPORT cppStringTargets
    FILE cppStringTargets.cmake
    NAMESPACE cppString::
    DESTINATION ${CPPSTRING_CMAKE_DIR})

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cppStringConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cppStringConfig.cmake"
    INSTALL_DESTINATION ${CPPSTRING_CMAKE_DIR})

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/cppStringConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/cppStringConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/cppStringConfigVersion.cmake"
    DESTINATION ${CPPSTRING_CMAKE_DIR})
