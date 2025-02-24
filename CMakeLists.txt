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
find_package(Qt6 REQUIRED COMPONENTS Widgets Network MultimediaWidgets)
qt_standard_project_setup()

###################################Add components

# Target to copy asset folder into binary folder
add_custom_target(assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
)

# Create dependencies
add_subdirectory(netlib)
add_subdirectory(external)

# Main app
add_subdirectory(app)
