# The main CMake setting

cmake_minimum_required(VERSION 3.0.0)

# Set project
set(HWE_NAME "Heterogeneous Word Embedding")
include(cmake/cache.cmake)
project(HWE VERSION ${HWE_VERSION} 0.0.1)

# Set Makefile
set(CMAKE_COLOR_MAKEFILE ON)
# set(CMAKE_VERBOSE_MAKEFILE ON)

# Set module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

# Variables
string(ASCII 27 Esc)

# Setup
include(cmake/option.cmake)
include(cmake/package.cmake)

# Add subdirectories
add_subdirectory(ext)
add_subdirectory(src)
add_subdirectory(doxygen)
