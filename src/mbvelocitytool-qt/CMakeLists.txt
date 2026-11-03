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

# mbvelocitytool-qt: Qt6 port of mbvelocitytool (CMake only, built with
# -DbuildQt6Tools=ON). Needs only Qt6 Widgets (in qtbase) and libmbio.

# On macOS Qt uses the system OpenGL framework. build-utils/FindOpenGL.cmake finds
# the X11 Mesa libGL of the Motif programs, so in this directory let Qt's
# find_package(OpenGL) use CMake's own module.
if(APPLE)
  list(REMOVE_ITEM CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-utils")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)

add_executable(
  mbvelocitytool-qt
  mbvelocitytool-qt.cc
  MainWindow.cc
  MainWindow.h
  mbvt_core.cc
  mbvt_core.h
  mbvt_plots.cc
  mbvt_plots.h)

# Qt6 needs C++17
set_target_properties(mbvelocitytool-qt PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

target_link_libraries(mbvelocitytool-qt PRIVATE mbio Qt6::Widgets ${MB_SYSLIBS})

install(TARGETS mbvelocitytool-qt DESTINATION ${CMAKE_INSTALL_BINDIR})
