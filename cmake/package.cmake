if(HWE_USE_PACKAGE)
  set(findtype REQUIRED)
else()
  set(findtype "")
endif()

# Check compiler support
if(HWE_USE_PACKAGE)
  string(REGEX REPLACE " " ";" cflags "${CMAKE_C_FLAGS}")
  include(CheckCCompilerFlag)
  foreach(cflag ${cflags})
    string(TOUPPER ${cflag} cflagname)
    string(REGEX REPLACE "=" "_" cflagname ${cflagname})
    string(REGEX REPLACE "\\+" "X" cflagname ${cflagname})
    CHECK_C_COMPILER_FLAG(${cflag} C_SUPPORTS_${cflagname})
    if(NOT C_SUPPORTS_${cflagname})
      message(
        FATAL_ERROR
        "The compiler ${CMAKE_C_COMPILER} does not support ${cflag}. "
        "Please use a different C compiler."
      )
    endif()
  endforeach()

  string(REGEX REPLACE " " ";" cxxflags "${CMAKE_CXX_FLAGS}")
  include(CheckCXXCompilerFlag)
  foreach(cxxflag ${cxxflags})
    string(TOUPPER ${cxxflag} cxxflagname)
    string(REGEX REPLACE "=" "_" cxxflagname ${cxxflagname})
    string(REGEX REPLACE "\\+" "X" cxxflagname ${cxxflagname})
    CHECK_CXX_COMPILER_FLAG(${cxxflag} CXX_SUPPORTS_${cxxflagname})
    if(NOT CXX_SUPPORTS_${cxxflagname})
      message(
        FATAL_ERROR
        "The compiler ${CMAKE_CXX_COMPILER} does not support ${cxxflag}. "
        "Please use a different C++ compiler."
      )
    endif()
  endforeach()
endif()

# Set target
find_library(
  M_LIBRARY
  NAMES m
  DOC "libm"
)
if(NOT M_LIBRARY)
  CHECK_C_COMPILER_FLAG("-lm" M_LIBRARY_DETECTED)
  if(M_LIBRARY_DETECTED)
    set(M_LIBRARY "-lm" CACHE STRING "libm" FORCE)
  endif()
endif()

find_library(
  PTHREAD_LIBRARY
  NAMES pthread
  DOC "libpthread"
)
if(NOT PTHREAD_LIBRARY)
  CHECK_C_COMPILER_FLAG("-lpthread" PTHREAD_LIBRARY_DETECTED)
  if(PTHREAD_LIBRARY_DETECTED)
    set(PTHREAD_LIBRARY "-lpthread" CACHE STRING "libpthread" FORCE)
  endif()
endif()

mark_as_advanced(M_LIBRARY PTHREAD_LIBRARY)
set(DEFAULT_LIBRARY ${M_LIBRARY} ${PTHREAD_LIBRARY})

function(HWE_SET_TARGET target)
  target_link_libraries(${target} ${DEFAULT_LIBRARY})
  set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " -Wl,--no-as-needed")
endfunction()

# Doxygen
if(HWE_BUILD_DOC)
  find_package(Doxygen REQUIRED)
endif()
