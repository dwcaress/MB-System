#------------------------------------------------------------------------------
#    The MB-system:  CMakeLists.txt   1 November 2026
#
#    Copyright (c) 2026 by
#    Christian dos Santos Ferreira
#      MARUM
#      University of Bremen
#      Bremen Germany
#    David W. Caress (caress@mbari.org)
#      Monterey Bay Aquarium Research Institute
#      Moss Landing, California, USA
#    Dale N. Chayes
#      Center for Coastal and Ocean Mapping
#      University of New Hampshire
#      Durham, New Hampshire, USA
#
#    MB-System was created by Caress and Chayes in 1992 at the
#      Lamont-Doherty Earth Observatory
#      Columbia University
#      Palisades, NY 10964
#
#    See README.md file for copying and redistribution conditions.
#------------------------------------------------------------------------------

# mbwcdview: Qt6 water column viewer (CMake only, built with -DbuildWCDView=ON).
# Needs only Qt6 Widgets, OpenGL and OpenGLWidgets (all in qtbase) and libmbio.

# On macOS Qt uses the system OpenGL framework. build-utils/FindOpenGL.cmake finds
# the X11 Mesa libGL of the Motif programs, so in this directory let Qt's
# find_package(OpenGL) use CMake's own module.
if(APPLE)
  list(REMOVE_ITEM CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-utils")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)

set(CMAKE_AUTOMOC ON)

add_executable(
  mbwcdview
  mbwcdview.cc
  MainWindow.cc
  MainWindow.h
  WcdView.cc
  WcdView.h
  wcd_data.cc
  wcd_data.h
  wcd_dump.cc
  wcd_dump.h
  wcd_plot.cc
  wcd_plot.h
  wcd_raytrace.cc
  wcd_raytrace.h
  wcd_scene.cc
  wcd_scene.h
  viridis.h)

# Qt6 needs C++17
set_target_properties(mbwcdview PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

target_link_libraries(mbwcdview PRIVATE mbio Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets ${MB_SYSLIBS})

install(TARGETS mbwcdview DESTINATION ${CMAKE_INSTALL_BINDIR})
