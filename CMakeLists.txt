cmake_minimum_required(VERSION 3.15)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

# Fix stuff that's broken by default
include(ForbidInSource)
include(SetBuildType)

project(
  hyperhotp
  VERSION 0.1
  DESCRIPTION "Open source programmer for the HOTP feature of Hypersecu USB tokens"
  LANGUAGES C)
# Library for features shared between CLI and GUI
add_library(lib${PROJECT_NAME} STATIC "src/log.c" "src/usb.c" "src/u2fhid.c" "src/hyperhotp.c")
# CLI
add_executable(${PROJECT_NAME} "src/cli/main.c" "src/cli/cli.c")
# GUI

target_compile_features(lib${PROJECT_NAME} PUBLIC c_std_11)
target_compile_features(${PROJECT_NAME} PRIVATE c_std_11)
# TODO: Fix for clang/MSVC
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wcast-align -Wcast-qual -Wformat=2 -Winit-self -Wlogical-op -Wredundant-decls -Wshadow -Wsign-conversion -Wstrict-overflow=5 -Wswitch-default -Wundef -Wunreachable-code -Wwrite-strings -fstack-protector-all)

# 3rd-party deps
find_package(Libusb 1.0 REQUIRED)

target_link_libraries(lib${PROJECT_NAME} PRIVATE Libusb::Libusb)
target_link_libraries(${PROJECT_NAME} PRIVATE lib${PROJECT_NAME})

install(
  TARGETS ${PROJECT_NAME}
  CONFIGURATIONS Release
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Define DEBUG macro in debug builds
target_compile_definitions(lib${PROJECT_NAME} PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")
target_compile_definitions(${PROJECT_NAME} PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")
