# The CMake setting of 'src'

# Set include paths
include_directories("${PROJECT_SOURCE_DIR}/include")

# libhwe
file(GLOB_RECURSE files "${CMAKE_CURRENT_SOURCE_DIR}/hwe/*")
add_library(hwe ${files})
hwe_set_target(hwe)
hwe_set_target_omp(hwe CXX)
