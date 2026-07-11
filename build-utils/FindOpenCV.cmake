#--------------------------------------------------------------------
#    The MB-system:    build-utils/FindOpenCV.cmake    11/3/2023
#
#    Copyright (c) 2023-2023 by
#    David W. Caress (caress@mbari.org)
#      Monterey Bay Aquarium Research Institute
#      Moss Landing, California, USA
#    Dale N. Chayes
#      Center for Coastal and Ocean Mapping
#      University of New Hampshire
#      Durham, New Hampshire, USA
#    Christian dos Santos Ferreira
#      MARUM
#      University of Bremen
#      Bremen Germany
#
#    MB-System was created by Caress and Chayes in 1992 at the
#      Lamont-Doherty Earth Observatory
#      Columbia University
#      Palisades, NY 10964
#
#    See README.md file for copying and redistribution conditions.
#--------------------------------------------------------------------*/
#
# Find OpenCV Library (version 4 or later) (https://github.com/opencv/opencv)
#
# OpenCV 5 requires components built against it to use C++17 or later, and
# it splits the old "calib3d" module into "geometry", "calib", and "stereo"
# (calib3d.hpp is kept only as a legacy header that forwards to the three).
# Rather than chase component/library names across major versions, this
# module asks for the whole package and exposes it as the OpenCV::OpenCV
# imported target, linking the full ${OpenCV_LIBS} (or pkg-config) set,
# so it works unchanged against OpenCV 4.x and 5.x (and whatever module
# names later versions choose to use).
#
# On success this module sets the usual OpenCV_FOUND and defines the
# OpenCV::OpenCV INTERFACE IMPORTED target; callers should simply do:
#
#   find_package(OpenCV REQUIRED)
#   target_link_libraries(mytarget PRIVATE OpenCV::OpenCV)
#
set(OpenCV_FOUND FALSE)

# First try find_package() in config mode
if(NOT OpenCV_FOUND)

  # No minimum version is passed to find_package() here: OpenCV's own
  # OpenCVConfigVersion.cmake treats a requested major version as an exact
  # compatibility requirement, so asking for "4" would reject a present 5.x
  # package instead of accepting it as newer-and-compatible. The floor is
  # enforced manually below instead.
  find_package(OpenCV CONFIG)

  if(OpenCV_FOUND AND OpenCV_VERSION VERSION_LESS "4.0.0")
    message(FATAL_ERROR "OpenCV version ${OpenCV_VERSION} is not supported. Requires 4.x or later.")
  endif()

  if(OpenCV_FOUND)
    add_library(OpenCV::OpenCV INTERFACE IMPORTED)
    target_include_directories(OpenCV::OpenCV INTERFACE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(OpenCV::OpenCV INTERFACE ${OpenCV_LIBS})
  endif()

endif()

#
# Else if pkg-config exists then look for opencv4/opencv5
if(NOT OpenCV_FOUND)

  find_package(PkgConfig)
  if(PkgConfig_FOUND)
    if(APPLE)
      set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/opt/local/lib/opencv4/pkgconfig:/opt/local/lib/opencv5/pkgconfig")
    endif(APPLE)
    # Try version-specific pkg-config module names first, then the
    # unversioned name some distributions use.
    pkg_search_module(OPENCV_PKGCONFIG IMPORTED_TARGET opencv5 opencv4 opencv)

    if(OPENCV_PKGCONFIG_FOUND AND OPENCV_PKGCONFIG_VERSION VERSION_LESS "4.0.0")
      message(FATAL_ERROR "OpenCV version ${OPENCV_PKGCONFIG_VERSION} is not supported. Requires 4.x or later.")
    endif()

    if(OPENCV_PKGCONFIG_FOUND)
      add_library(OpenCV::OpenCV INTERFACE IMPORTED)
      target_link_libraries(OpenCV::OpenCV INTERFACE PkgConfig::OPENCV_PKGCONFIG)
      set(OpenCV_FOUND TRUE)
      set(OpenCV_VERSION ${OPENCV_PKGCONFIG_VERSION})
    endif(OPENCV_PKGCONFIG_FOUND)

  endif(PkgConfig_FOUND)
endif(NOT OpenCV_FOUND)

if(OpenCV_FOUND)
  message("-- OpenCV found!")
else()
  message("-- OpenCV NOT found...")
endif()
