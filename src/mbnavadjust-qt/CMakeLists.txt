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

# mbnavadjust-qt: Qt6 port of mbnavadjust (CMake only, built with -DbuildQt6Tools=ON).

# mbnavadjust-qt-core: no Qt and no X11. It compiles the unchanged
# src/mbnavadjust/mbnavadjust_prog.c, so the x11 directory, which holds a
# stand-in for the one X11 header that file needs, is private to this library.
add_library(
  mbnavadjust-qt-core STATIC
  mbnaq_bridge.cc
  mbnaq_core.cc
  mbnaq_core.h
  mbnaq_noview.c
  mbnaq_prog.c)

set_target_properties(mbnavadjust-qt-core PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

target_include_directories(mbnavadjust-qt-core BEFORE PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/x11)
target_include_directories(
  mbnavadjust-qt-core
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE ${CMAKE_SOURCE_DIR}/src/mbview ${CMAKE_SOURCE_DIR}/src/mbnavadjust)

target_link_libraries(mbnavadjust-qt-core PUBLIC mbnavadjust_core mbaux mbio ${MB_SYSLIBS})

# On macOS Qt uses the system OpenGL framework. build-utils/FindOpenGL.cmake finds
# the X11 Mesa libGL of the Motif programs, so in this directory let Qt's
# find_package(OpenGL) use CMake's own module.
if(APPLE)
  list(REMOVE_ITEM CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-utils")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# mbnavadjust-qt-ui: the plots as images and the widgets that show them.
add_library(
  mbnavadjust-qt-ui STATIC
  mbnaq_plots.cc
  mbnaq_plots.h
  mbnaq_views.cc
  mbnaq_views.h
  mbnaq_controller.cc
  mbnaq_controller.h
  mbnaq_terminal.cc
  mbnaq_terminal.h
  mbnaq_widgets.cc
  mbnaq_widgets.h
  NavAdjustWindow.cc
  NavAdjustWindow.h)

set_target_properties(mbnavadjust-qt-ui PROPERTIES AUTOMOC ON CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_include_directories(mbnavadjust-qt-ui BEFORE PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/x11)
target_include_directories(mbnavadjust-qt-ui PRIVATE ${CMAKE_SOURCE_DIR}/src/mbnavadjust)
target_link_libraries(mbnavadjust-qt-ui PUBLIC mbnavadjust-qt-core Qt6::Widgets)

add_executable(mbnavadjust-qt mbnavadjust-qt.cc)
set_target_properties(mbnavadjust-qt PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(mbnavadjust-qt PRIVATE mbnavadjust-qt-ui)

install(TARGETS mbnavadjust-qt DESTINATION ${CMAKE_INSTALL_BINDIR})
