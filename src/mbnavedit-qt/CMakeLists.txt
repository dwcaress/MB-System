#------------------------------------------------------------------------------
#    The MB-system:  CMakeLists.txt   9 November 2026
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

# mbnavedit-qt: Qt6 port of mbnavedit (CMake only, built with -DbuildQt6Tools=ON).
# The tool is built as libraries, so that another program can use it, and a small
# program: mbnavedit-qt-core is the editing session, without Qt, and mbnavedit-qt-ui
# holds what is drawn on the screen.

# On macOS Qt uses the system OpenGL framework. build-utils/FindOpenGL.cmake finds
# the X11 Mesa libGL of the Motif programs, so in this directory let Qt's
# find_package(OpenGL) use CMake's own module.
if(APPLE)
  list(REMOVE_ITEM CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-utils")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(mbnavedit-qt-core STATIC mbne_core.cc mbne_core.h)

target_include_directories(mbnavedit-qt-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(mbnavedit-qt-core PUBLIC mbaux mbio ${MB_SYSLIBS})

# mb_define.h includes the header of this library on Windows, and mbio links it privately
if(WIN32)
  target_link_libraries(mbnavedit-qt-core PUBLIC mb_xdr_win32)
endif()

set(CMAKE_AUTOMOC ON)

add_library(mbnavedit-qt-ui STATIC mbne_canvas.cc mbne_canvas.h)

target_link_libraries(mbnavedit-qt-ui PUBLIC mbnavedit-qt-core Qt6::Widgets)

add_executable(mbnavedit-qt mbnavedit-qt.cc)

target_link_libraries(mbnavedit-qt PRIVATE mbnavedit-qt-ui)

set_target_properties(mbnavedit-qt-core mbnavedit-qt-ui mbnavedit-qt PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

install(TARGETS mbnavedit-qt DESTINATION ${CMAKE_INSTALL_BINDIR})
