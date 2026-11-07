#------------------------------------------------------------------------------
#    The MB-system:  CMakeLists.txt   7 November 2026
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

# mbgrdviz-qt: the Qt6 version of mbgrdviz (CMake only, built with
# -DbuildQt6Tools=ON). The program logic is the shared mbgrdviz_core.c of
# src/mbgrdviz; the user interface functions it calls are in
# mbgrdviz_qt_ui.c and the Qt main window. The Motif mbgrdviz is unchanged.

if(APPLE)
  list(REMOVE_ITEM CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-utils")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)

# everything but main, shared with the test in test/mbgrdviz-qt
add_library(
  mbgrdviz-qt-ui STATIC
  ${CMAKE_SOURCE_DIR}/src/mbgrdviz/mbgrdviz_core.c
  mbgrdviz_qt_bridge.h
  mbgrdviz_qt_ui.c
  mbgrdviz_qt_window.cc
  mbgrdviz_qt_window.h)
set_target_properties(mbgrdviz-qt-ui PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_include_directories(mbgrdviz-qt-ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/mbgrdviz)
target_link_libraries(mbgrdviz-qt-ui PUBLIC mbview-qt mbaux mbio Qt6::Widgets ${MB_SYSLIBS})

add_executable(mbgrdviz-qt mbgrdviz_qt_main.cc)
set_target_properties(mbgrdviz-qt PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(mbgrdviz-qt PRIVATE mbgrdviz-qt-ui)

install(TARGETS mbgrdviz-qt DESTINATION ${CMAKE_INSTALL_BINDIR})
