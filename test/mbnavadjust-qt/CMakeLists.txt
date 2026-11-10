#------------------------------------------------------------------------------
#    The MB-system:  CMakeLists.txt   10 November 2026
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

# Tests of mbnavadjust-qt on a synthetic project, without a display: the program
# and a program that drives the plot widgets with mouse events.

if(APPLE)
  list(REMOVE_ITEM CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-utils")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(mbnavadjust_qt_view_test mbnavadjust_qt_view_test.cc)
set_target_properties(mbnavadjust_qt_view_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(mbnavadjust_qt_view_test PRIVATE mbnavadjust-qt-ui)

add_executable(mbnavadjust_qt_window_test mbnavadjust_qt_window_test.cc)
set_target_properties(mbnavadjust_qt_window_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(mbnavadjust_qt_window_test PRIVATE mbnavadjust-qt-ui)

add_executable(mbnavadjust_qt_embed_test mbnavadjust_qt_embed_test.cc)
set_target_properties(mbnavadjust_qt_embed_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(mbnavadjust_qt_embed_test PRIVATE mbnavadjust-qt-ui)

find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND)
  add_test(NAME mbnavadjust_qt_test COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/mbnavadjust_qt_test.py")
  set_tests_properties(
    mbnavadjust_qt_test
    PROPERTIES ENVIRONMENT
               "MBNAVADJUST_QT=$<TARGET_FILE:mbnavadjust-qt>;MBNAVADJUST_QT_VIEW_TEST=$<TARGET_FILE:mbnavadjust_qt_view_test>;MBNAVADJUST_QT_WINDOW_TEST=$<TARGET_FILE:mbnavadjust_qt_window_test>;MBNAVADJUST_QT_EMBED_TEST=$<TARGET_FILE:mbnavadjust_qt_embed_test>;MBNAVADJUSTMERGE=$<TARGET_FILE:mbnavadjustmerge>;MBINFO=$<TARGET_FILE:mbinfo>;SYNTH_KMALL_DIR=${CMAKE_SOURCE_DIR}/test/python")
endif()
