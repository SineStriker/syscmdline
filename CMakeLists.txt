cmake_minimum_required(VERSION 3.17)

project(syscmdline VERSION 0.0.1.1 LANGUAGES CXX)

# ----------------------------------
# Build Options
# ----------------------------------
option(SYSCMDLINE_BUILD_STATIC "Build static library" ON)
option(SYSCMDLINE_BUILD_TESTS "Build test cases" ON)

# ----------------------------------
# CMake Settings
# ----------------------------------
if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

if(NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

if(NOT DEFINED CMAKE_BUILD_SHARE_DIR)
    set(CMAKE_BUILD_SHARE_DIR ${CMAKE_BINARY_DIR}/share)
endif()

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /manifest:no")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /manifest:no")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /manifest:no")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")

    if(NOT DEFINED CMAKE_DEBUG_POSTFIX)
        set(CMAKE_DEBUG_POSTFIX "d")
    endif()
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# ----------------------------------
# Project Variables
# ----------------------------------
set(SYSCMDLINE_VERSION ${PROJECT_VERSION})
set(SYSCMDLINE_INSTALL_NAME ${PROJECT_NAME})

# ----------------------------------
# Main Project
# ----------------------------------
add_subdirectory(src)

if(SYSCMDLINE_BUILD_TESTS)
    add_subdirectory(tests)
endif()