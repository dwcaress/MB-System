#------------------------------------------------------------------------------
#    The MB-system:  CMakeLists.txt   8 November 2026
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

# mbeditviz-qt: the Qt6 version of mbeditviz (CMake only, built with
# -DbuildQt6Tools=ON). The program logic is the shared mbeditviz_core.c and the
# unchanged mbeditviz_prog.c of src/mbeditviz, built without Motif; the user
# interface functions they call are in mbeditviz_qt_ui.c and the Qt windows.
# The Motif mbeditviz is unchanged.

if(APPLE)
  list(REMOVE_ITEM CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-utils")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)

# everything but main, shared with the test in test/mbeditviz-qt
add_library(
  mbeditviz-qt-ui STATIC
  ${CMAKE_SOURCE_DIR}/src/mbeditviz/mbeditviz_core.c
  ${CMAKE_SOURCE_DIR}/src/mbeditviz/mbeditviz_prog.c
  mbeditviz_qt_bridge.h
  mbeditviz_qt_controller.cc
  mbeditviz_qt_controller.h
  mbeditviz_qt_dialogs.cc
  mbeditviz_qt_dialogs.h
  mbeditviz_qt_filelist.cc
  mbeditviz_qt_filelist.h
  mbeditviz_qt_ui.c
  mbeditviz_qt_window.cc
  mbeditviz_qt_window.h)
set_target_properties(mbeditviz-qt-ui PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_include_directories(mbeditviz-qt-ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/mbeditviz)
target_link_libraries(mbeditviz-qt-ui PUBLIC mbview-qt mbaux mbio Qt6::Widgets ${MB_SYSLIBS})

add_executable(mbeditviz-qt mbeditviz_qt_main.cc)
set_target_properties(mbeditviz-qt PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(mbeditviz-qt PRIVATE mbeditviz-qt-ui)

install(TARGETS mbeditviz-qt DESTINATION ${CMAKE_INSTALL_BINDIR})
