# The CMake setting of 'doxygen/'

# Break
if(NOT HWE_BUILD_DOC)

  # Add rule 'doc'
  add_custom_target(
    doc
    COMMAND eval "exit 1"
    COMMENT "${Esc}[1;33mWarning: Documentation are not enabled! Please set 'HWE_BUILD_DOC' using ccmake${Esc}[0m."
  )

  return()
endif()

# Set documentation directory
set(HWE_DOCS_DIR "${PROJECT_BINARY_DIR}/docs")
set(HWE_DOCS_DIR "${HWE_DOCS_DIR}" PARENT_SCOPE)

# Set install rule
install(DIRECTORY "${HWE_DOCS_DIR}" DESTINATION docs OPTIONAL)

# Configure Doxyfile
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" @ONLY)

# Add rule
add_custom_target(
  doc
  COMMAND doxygen "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
  COMMENT "Generating documentation with Doxygen"
)

# Add to clean rule
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${HWE_DOCS_DIR}")
