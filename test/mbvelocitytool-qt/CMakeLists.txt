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

# Tests of mbvelocitytool-qt: without a window, and a program that drives the window
# with mouse events, run by mbvelocitytool_qt_test.py, which writes the synthetic swath files.

if(APPLE)
  list(REMOVE_ITEM CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-utils")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)

set(tool_dir "${CMAKE_SOURCE_DIR}/src/mbvelocitytool-qt")

add_executable(
  mbvelocitytool_qt_window_test
  mbvelocitytool_qt_window_test.cc
  ${tool_dir}/MainWindow.cc
  ${tool_dir}/MainWindow.h
  ${tool_dir}/mbvt_core.cc
  ${tool_dir}/mbvt_core.h
  ${tool_dir}/mbvt_plots.cc
  ${tool_dir}/mbvt_plots.h)

set_target_properties(mbvelocitytool_qt_window_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

target_include_directories(mbvelocitytool_qt_window_test PRIVATE ${tool_dir})

target_link_libraries(mbvelocitytool_qt_window_test PRIVATE mbio Qt6::Widgets ${MB_SYSLIBS})

find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND)
  add_test(NAME mbvelocitytool_qt_test COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/mbvelocitytool_qt_test.py")
  set_tests_properties(mbvelocitytool_qt_test PROPERTIES ENVIRONMENT
                       "MBVELOCITYTOOL_QT=$<TARGET_FILE:mbvelocitytool-qt>;MBVELOCITYTOOL_QT_WINDOW_TEST=$<TARGET_FILE:mbvelocitytool_qt_window_test>")
endif()
