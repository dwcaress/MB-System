#------------------------------------------------------------------------------
#    The MB-system:  CMakeLists.txt   2 November 2026
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

# mbedit-qt: Qt6 port of mbedit (CMake only, built with -DbuildQt6Tools=ON).
# Needs Qt6 Widgets (in qtbase), libmbio, and libmbaux to read grids.

# On macOS Qt uses the system OpenGL framework. build-utils/FindOpenGL.cmake finds
# the X11 Mesa libGL of the Motif programs, so in this directory let Qt's
# find_package(OpenGL) use CMake's own module.
if(APPLE)
  list(REMOVE_ITEM CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-utils")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)

add_executable(
  mbedit-qt
  mbedit-qt.cc
  MainWindow.cc
  MainWindow.h
  mbe_canvas.cc
  mbe_canvas.h
  mbe_core.cc
  mbe_core.h
  mbe_panels.cc
  mbe_panels.h)

# Qt6 needs C++17
set_target_properties(mbedit-qt PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

target_link_libraries(mbedit-qt PRIVATE mbaux mbio Qt6::Widgets ${MB_SYSLIBS})

install(TARGETS mbedit-qt DESTINATION ${CMAKE_INSTALL_BINDIR})
