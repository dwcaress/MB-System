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

# Tests of mbeditviz-qt. mbeditviz_qt_test runs without drawing (null backend,
# offscreen platform): ctest checks the windows without data; with --data and
# a datalist it also loads, grids and unloads swath data (not run by ctest).
# mbeditviz_qt_test.py checks the usage text.

if(APPLE)
  list(REMOVE_ITEM CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-utils")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(mbeditviz_qt_test mbeditviz_qt_test.cc)
set_target_properties(mbeditviz_qt_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(mbeditviz_qt_test PRIVATE mbeditviz-qt-ui)

add_test(NAME mbeditviz_qt_test COMMAND mbeditviz_qt_test)
set_tests_properties(mbeditviz_qt_test PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# mbeditviz inside another program's window (null backend, offscreen platform;
# with --data and a datalist also swath data, not run by ctest)
add_executable(mbeditviz_qt_embed_test mbeditviz_qt_embed_test.cc)
set_target_properties(mbeditviz_qt_embed_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(mbeditviz_qt_embed_test PRIVATE mbeditviz-qt-ui)
add_test(NAME mbeditviz_qt_embed_test COMMAND mbeditviz_qt_embed_test)
set_tests_properties(mbeditviz_qt_embed_test PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND)
  add_test(NAME mbeditviz_qt_usage_test COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/mbeditviz_qt_test.py")
  set_tests_properties(mbeditviz_qt_usage_test PROPERTIES ENVIRONMENT
                       "MBEDITVIZ_QT=$<TARGET_FILE:mbeditviz-qt>;QT_QPA_PLATFORM=offscreen")
endif()
