cmake_minimum_required(VERSION 3.15)

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
add_library(
  lib${PROJECT_NAME} STATIC "src/core/log.c" "src/core/usb.c"
                            "src/core/u2fhid.c" "src/core/hyperhotp.c")
# CLI
add_executable(${PROJECT_NAME} "src/cli/main.c" "src/cli/cli.c")
# GUI

target_compile_features(lib${PROJECT_NAME} PUBLIC c_std_11)
target_compile_features(${PROJECT_NAME} PRIVATE c_std_11)
# TODO: Fix for clang/MSVC
list(
  APPEND
  DEBUG_FLAGS
  -Wall
  -Wextra
  -Wpedantic
  -Wcast-align
  -Wcast-qual
  -Wformat=2
  -Winit-self
  -Wlogical-op
  -Wredundant-decls
  -Wshadow
  -Wsign-conversion
  -Wstrict-overflow=5
  -Wswitch-default
  -Wundef
  -Wunreachable-code
  -Wwrite-strings
  -fstack-protector-all)

target_compile_options(${PROJECT_NAME} PRIVATE ${DEBUG_FLAGS})
target_compile_options(lib${PROJECT_NAME} PRIVATE ${DEBUG_FLAGS})

# 3rd-party deps
find_package(Libusb 1.0 REQUIRED)

target_link_libraries(lib${PROJECT_NAME} PRIVATE Libusb::Libusb)
target_link_libraries(${PROJECT_NAME} PRIVATE lib${PROJECT_NAME})
list(APPEND INSTALLABLES ${PROJECT_NAME})

# GUI (Optional)
if(BUILD_GUI)
  find_package(GTK 4 REQUIRED)
  add_executable(${PROJECT_NAME}_gui "src/gui/main.c")
  target_compile_features(${PROJECT_NAME}_gui PRIVATE c_std_11)
  target_compile_options(${PROJECT_NAME}_gui PRIVATE ${DEBUG_FLAGS})
  target_link_libraries(${PROJECT_NAME}_gui PRIVATE GTK::GTK lib${PROJECT_NAME})
  list(APPEND INSTALLABLES ${PROJECT_NAME}_gui)
endif()

install(
  TARGETS ${INSTALLABLES}
  CONFIGURATIONS Release
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Define DEBUG macro in debug builds
target_compile_definitions(lib${PROJECT_NAME}
                           PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")
target_compile_definitions(${PROJECT_NAME} PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")
if(BUILD_GUI)
  target_compile_definitions(${PROJECT_NAME}_gui
                             PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")
endif()
