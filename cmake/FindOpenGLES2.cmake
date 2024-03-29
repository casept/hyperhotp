# Taken from
# https://github.com/mosra/magnum-integration/blob/master/modules/FindOpenGLES3.cmake,
# commit 911244611909e308398d0e8017fd2df6e74cec27. Modified for finding GLES2
# instead of GLES3.

# .rst: Find OpenGL ES 2
# ----------------
#
# Finds the OpenGL ES 2 library. This module defines:
#
# OpenGLES2_FOUND          - True if OpenGL ES 2 library is found
# OpenGLES2::OpenGLES2     - OpenGL ES 2 imported target
#
# Additionally these variables are defined for internal usage:
#
# OPENGLES2_LIBRARY        - OpenGL ES 2 library
#
# Please note this find module is tailored especially for the needs of Magnum.
# In particular, it depends on its platform definitions and doesn't look for
# OpenGL ES includes as Magnum has its own, generated using flextGL.
#

#
# This file is part of Magnum.
#
# Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020,
# 2021, 2022 Vladimír Vondruš <mosra@centrum.cz>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

# Under Emscripten, GL is linked implicitly. With MINIMAL_RUNTIME you need to
# specify -lGL. Simply set the library name to that.
if(CORRADE_TARGET_EMSCRIPTEN)
  set(OPENGLES2_LIBRARY
      GL
      CACHE STRING "Path to a library." FORCE)
else()
  find_library(
    OPENGLES2_LIBRARY
    NAMES # Used by Android
          GLESv2
          # On some platforms (e.g. desktop emulation with Mesa or NVidia) ES2
          # support is provided in ES2 lib
          GLESv2
          # ANGLE (CMake doesn't search for lib prefix on Windows)
          libGLESv2
          # iOS
          OpenGLES)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("OpenGLES2" DEFAULT_MSG OPENGLES2_LIBRARY)

if(NOT TARGET OpenGLES2::OpenGLES2)
  # Work around BUGGY framework support on macOS. Do this also in case of
  # Emscripten, since there we don't have a location either.
  # http://public.kitware.com/pipermail/cmake/2016-April/062179.html
  if((CORRADE_TARGET_APPLE AND ${OPENGLES2_LIBRARY} MATCHES "\\.framework$")
     OR CORRADE_TARGET_EMSCRIPTEN)
    add_library(OpenGLES2::OpenGLES2 INTERFACE IMPORTED)
    set_property(
      TARGET OpenGLES2::OpenGLES2
      APPEND
      PROPERTY INTERFACE_LINK_LIBRARIES ${OPENGLES2_LIBRARY})
  else()
    add_library(OpenGLES2::OpenGLES2 UNKNOWN IMPORTED)
    set_property(TARGET OpenGLES2::OpenGLES2 PROPERTY IMPORTED_LOCATION
                                                      ${OPENGLES2_LIBRARY})
  endif()

  # Emscripten needs a special flag to use WebGL 2. CMake 2.12 allows to set
  # this via INTERFACE_LINK_OPTIONS, for older versions we modify the global
  # CMAKE_EXE_LINKER_FLAGS inside FindMagnum.cmake.
  if(CORRADE_TARGET_EMSCRIPTEN AND NOT CMAKE_VERSION VERSION_LESS 2.12)
    # I could probably use target_link_options() here, but let's be consistent
    # with the rest
    set_property(
      TARGET OpenGLES2::OpenGLES2
      APPEND
      PROPERTY INTERFACE_LINK_OPTIONS "SHELL:-s USE_WEBGL2=1")
  endif()
endif()

mark_as_advanced(OPENGLES2_LIBRARY)
