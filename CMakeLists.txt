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
add_executable(${PROJECT_NAME} "src/main.c" "src/cli.c" "src/log.c" "src/usb.c" "src/u2fhid.c" "src/hyperhotp.c")


target_compile_features(${PROJECT_NAME} PRIVATE c_std_11)
# TODO: Fix for clang/MSVC
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wcast-align -Wcast-qual -Wformat=2 -Winit-self -Wlogical-op -Wredundant-decls -Wshadow -Wsign-conversion -Wstrict-overflow=5 -Wswitch-default -Wundef -Wunreachable-code -Wwrite-strings -fstack-protector-all)

# 3rd-party deps
find_package(Libusb 1.0 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Libusb::Libusb)

install(
  TARGETS ${PROJECT_NAME}
  CONFIGURATIONS Release
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Define DEBUG macro in debug builds
target_compile_definitions(${PROJECT_NAME} PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")
