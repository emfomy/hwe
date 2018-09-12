# Set options

option(HWE_BUILD_BIN  "Build binaries."        "ON")
option(HWE_BUILD_DOC  "Build documentation."   "OFF")

# Set variables
if(HWE_BUILD_BIN)
  set(HWE_USE_PACKAGE "ON")
else()
  set(HWE_USE_PACKAGE "OFF")
endif()

# Set environment variables
list(APPEND ENVS "ASAN_OPTIONS=color=always:protect_shadow_gap=0")
