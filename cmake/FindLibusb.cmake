cmake_minimum_required(VERSION 3.15)

# Find using pkgconfig
find_package(PkgConfig QUIET REQUIRED)
pkg_check_modules(PC_Libusb QUIET libusb-1.0)

set(Libusb_FOUND ${PC_Libusb_FOUND})

set(Libusb_VERSION ${PC_Libusb_VERSION})
mark_as_advanced(Libusb_VERSION)

# Find header path
find_path(
  Libusb_INCLUDE_DIR
  NAMES libusb-1.0/libusb.h
  HINTS ${PC_Libusb_INCLUDE_DIRS})
mark_as_advanced(Libusb_INCLUDE_DIR)

# Find lib path
find_library(
  Libusb_LIBRARIES
  NAMES usb-1.0
  HINTS ${PC_Libusb_LIBRARY_DIRS})
mark_as_advanced(Libusb_LIBRARIES)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Libusb
  FOUND_VAR Libusb_FOUND
  REQUIRED_VARS Libusb_INCLUDE_DIR Libusb_LIBRARIES
  VERSION_VAR Libusb_VERSION)

if(Libusb_FOUND)
  set(Libusb_INCLUDE_DIRS ${Libusb_INCLUDE_DIR})

  add_library(Libusb::Libusb INTERFACE IMPORTED)
  target_include_directories(Libusb::Libusb INTERFACE ${Libusb_INCLUDE_DIRS})
  target_link_libraries(Libusb::Libusb INTERFACE ${Libusb_LIBRARIES})
endif()
