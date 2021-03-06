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
  -fno-stack-protector
  -g
  -O0
  -fdiagnostics-color=always
  -fno-omit-frame-pointer)
list(APPEND DEBUG_LD_OPTS -fno-omit-frame-pointer)

target_compile_options(${PROJECT_NAME} PRIVATE ${DEBUG_FLAGS})
target_compile_options(lib${PROJECT_NAME} PRIVATE ${DEBUG_FLAGS})

# 3rd-party deps
find_package(Libusb 1.0 REQUIRED)

target_link_libraries(lib${PROJECT_NAME} PRIVATE Libusb::Libusb)
target_link_options(lib${PROJECT_NAME} PRIVATE ${DEBUG_LD_OPTS})
if(CMAKE_BUILD_STATIC)
  target_link_libraries(${PROJECT_NAME} PRIVATE lib${PROJECT_NAME} -static)
else()
  target_link_libraries(${PROJECT_NAME} PRIVATE lib${PROJECT_NAME})
endif()
target_link_options(${PROJECT_NAME} PRIVATE ${DEBUG_LD_OPTS})
list(APPEND INSTALLABLES ${PROJECT_NAME})

# GUI (Optional)
if(BUILD_GUI)
  find_package(GTK 4 REQUIRED)
  add_executable(${PROJECT_NAME}_gui "src/gui/main.c")
  target_compile_features(${PROJECT_NAME}_gui PRIVATE c_std_11)
  target_compile_options(${PROJECT_NAME}_gui PRIVATE ${DEBUG_FLAGS})
  target_link_libraries(${PROJECT_NAME}_gui PRIVATE GTK::GTK lib${PROJECT_NAME})
  target_link_options(${PROJECT_NAME}_gui PRIVATE ${DEBUG_LD_OPTS})
  list(APPEND INSTALLABLES ${PROJECT_NAME}_gui)
endif()

install(
  TARGETS ${INSTALLABLES}
  CONFIGURATIONS Release
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Hack to force cmake to add system (libc) header path to compile_commands.json.
# This also adds a lot of junk, but as long as it doesn't slow clangd too much
# I'll deal.
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES
      ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

# Define DEBUG macro in debug builds
target_compile_definitions(lib${PROJECT_NAME}
                           PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")
target_compile_definitions(${PROJECT_NAME} PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")
if(BUILD_GUI)
  target_compile_definitions(${PROJECT_NAME}_gui
                             PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")
endif()
