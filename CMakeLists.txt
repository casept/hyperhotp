cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

# Fix stuff that's broken by default
include(ForbidInSource)
include(SetBuildType)

project(
  hyperhotp
  VERSION 0.1
  DESCRIPTION
    "Open source programmer for the HOTP feature of Hypersecu USB tokens"
  LANGUAGES C)

# Library for features shared between CLI and GUI
add_library(hyperhotp_core STATIC "src/core/log.c" "src/core/usb.c"
                                  "src/core/u2fhid.c" "src/core/hyperhotp.c")
target_compile_features(hyperhotp_core PUBLIC c_std_11)
set_target_properties(hyperhotp_core PROPERTIES OUTPUT_NAME "hyperhotp_core")
find_package(Libusb 1.0 REQUIRED)
target_link_libraries(hyperhotp_core PUBLIC Libusb::Libusb)

# CLI
add_executable(hyperhotp_cli "src/cli/main.c" "src/cli/cli.c")
set_target_properties(hyperhotp_cli PROPERTIES OUTPUT_NAME "hyperhotp")
target_link_libraries(hyperhotp_cli PRIVATE hyperhotp_core)

# GUI (Optional)
if(BUILD_GUI)
  find_package(SDL2 REQUIRED)
  find_package(OpenGLES2 REQUIRED)
  find_library(LIBM_LIBRARIES m REQUIRED)
  add_executable(hyperhotp_gui "src/gui/main.c")
  target_compile_features(hyperhotp_gui PRIVATE c_std_11)
  target_link_libraries(hyperhotp_gui PRIVATE SDL2::SDL2 OpenGLES2::OpenGLES2
                                              ${LIBM_LIBRARIES} hyperhotp_core)
  list(APPEND INSTALLABLES hyperhotp_gui)
endif()

# Install executables and the man page
install(TARGETS ${INSTALLABLES} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_SOURCE_DIR}/hyperhotp.1 DESTINATION share/man/man1/)

# Hack to force cmake to add system (libc) header path to compile_commands.json.
# This also adds a lot of junk, but as long as it doesn't slow clangd too much
# I'll deal.
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES
      ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

# Define DEBUG macro in debug builds
target_compile_definitions(hyperhotp_core PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")
target_compile_definitions(hyperhotp_cli PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")
if(BUILD_GUI)
  target_compile_definitions(hyperhotp_gui PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>
  ")
endif()
