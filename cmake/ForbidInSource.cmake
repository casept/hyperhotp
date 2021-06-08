# Forbid in-source builds

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
  message(
    FATAL_ERROR
      "In-source builds are disabled.
    Please create a subfolder and use `cmake ..` inside it.
    NOTE: cmake will now create CMakeCache.txt and CMakeFiles/*.
          You must delete them, or cmake will refuse to work.")
endif()
