cmake_minimum_required(VERSION 3.20...3.28)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

if (NOT WIN32)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
endif()

# Allow for LSPs to find include paths
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Make all executables output into the build directory
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Compilation flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -Werror")

# For windows
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

# Project definition
project(mercury)

# Sets the default installation path for mac. If you aren't on mac, you
# should figure out where it installs to and add an extra case to this
# if statement that appends your installation location. Alternatively,
# you can run "cmake .. -DCMAKE_PREFIX_PATH=your_installation"
if (WIN32)
  # list(APPEND CMAKE_PREFIX_PATH "")
elseif (APPLE)
  list(APPEND CMAKE_PREFIX_PATH "~/Qt/6.8.1/macos")
elseif (UNIX)
  # list(APPEND CMAKE_PREFIX_PATH "")
endif()

# Install qt
find_package(Qt6 REQUIRED COMPONENTS Gui Widgets Network Multimedia MultimediaWidgets Test)
qt_standard_project_setup()
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

## FFmpeg -----

include(ExternalProject)

# Define FFmpeg source and install directories
set(FFMPEG_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/ffmpeg)
set(FFMPEG_INSTALL_DIR ${CMAKE_BINARY_DIR}/external/ffmpeg/install)

# Ensure FFmpeg submodule is initialized
execute_process(
    COMMAND git submodule update --init --recursive
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE GIT_SUBMODULE_RESULT
)

if (NOT GIT_SUBMODULE_RESULT EQUAL "0")
    message(FATAL_ERROR "Failed to initialize Git submodules. Make sure Git is installed.")
endif()

# Ensure FFmpeg is built
ExternalProject_Add(ffmpeg
    SOURCE_DIR ${FFMPEG_SOURCE_DIR}
    CONFIGURE_COMMAND ${FFMPEG_SOURCE_DIR}/configure --prefix=${FFMPEG_INSTALL_DIR} --disable-programs --disable-doc --disable-ffplay
    BUILD_COMMAND make -j$(nproc)
    INSTALL_COMMAND make install
    BUILD_IN_SOURCE 1
)

# Set FFmpeg binary path
set(FFMPEG_EXECUTABLE ${FFMPEG_INSTALL_DIR}/bin/ffmpeg)

## FFmpeg -----

# GTest keeps throwing annoying deprecration warnings that don't apply to us
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

# Install Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

###################################Add components

# Target to copy asset folder into binary folder
add_custom_target(assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
)

add_custom_target(config
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/config ${CMAKE_CURRENT_BINARY_DIR}/config
)

# Create dependencies
add_subdirectory(netlib)

# Main app
add_subdirectory(app)
