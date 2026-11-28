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
# The tool is built as two libraries, so that another program can use it, and a
# small program: mbedit-qt-core is the editing session, without Qt, and
# mbedit-qt-ui holds the windows.

# On macOS Qt uses the system OpenGL framework. build-utils/FindOpenGL.cmake finds
# the X11 Mesa libGL of the Motif programs, so in this directory let Qt's
# find_package(OpenGL) use CMake's own module.
if(APPLE)
  list(REMOVE_ITEM CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-utils")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(mbedit-qt-core STATIC mbe_core.cc mbe_core.h)

target_include_directories(mbedit-qt-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(mbedit-qt-core PUBLIC mbaux mbio ${MB_SYSLIBS})

set(CMAKE_AUTOMOC ON)

add_library(
  mbedit-qt-ui STATIC
  mbe_canvas.cc
  mbe_canvas.h
  mbe_editor.cc
  mbe_editor.h
  mbe_panels.cc
  mbe_panels.h
  mbe_window.cc
  mbe_window.h)

target_link_libraries(mbedit-qt-ui PUBLIC mbedit-qt-core Qt6::Widgets)

add_executable(mbedit-qt mbedit-qt.cc)

target_link_libraries(mbedit-qt PRIVATE mbedit-qt-ui)

# Qt6 needs C++17
set_target_properties(mbedit-qt-core mbedit-qt-ui mbedit-qt PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

install(TARGETS mbedit-qt DESTINATION ${CMAKE_INSTALL_BINDIR})
