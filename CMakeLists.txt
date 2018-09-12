# The main CMake setting

cmake_minimum_required(VERSION 3.0.0)

# Set project
set(HWE_NAME "Heterogeneous Word Embedding")

# Set cache options
include(cmake/cache.cmake)

# Initialize project
project(HWE VERSION ${HWE_VERSION} 0.0.1)

# Set Makefile
set(CMAKE_COLOR_MAKEFILE ON)
# set(CMAKE_VERBOSE_MAKEFILE ON)

# Set module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

# Variables
string(ASCII 27 Esc)

# Set the packages
include(cmake/package.cmake)

# Add subdirectories
add_subdirectory(ext)
