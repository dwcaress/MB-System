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

# Tests of mbnavedit-qt, run by mbnavedit_qt_test.py, which writes the synthetic swath files.

# As in src/mbnavedit-qt, let Qt's find_package(OpenGL) use CMake's own module on macOS.
if(APPLE)
  list(REMOVE_ITEM CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-utils")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(mbnavedit_qt_canvas_test mbnavedit_qt_canvas_test.cc)
target_link_libraries(mbnavedit_qt_canvas_test PRIVATE mbnavedit-qt-ui)
add_executable(mbnavedit_qt_window_test mbnavedit_qt_window_test.cc)
target_link_libraries(mbnavedit_qt_window_test PRIVATE mbnavedit-qt-ui)
add_executable(mbnavedit_qt_embed_test mbnavedit_qt_embed_test.cc)
target_link_libraries(mbnavedit_qt_embed_test PRIVATE mbnavedit-qt-ui)
set_target_properties(mbnavedit_qt_canvas_test mbnavedit_qt_window_test mbnavedit_qt_embed_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND)
  add_test(NAME mbnavedit_qt_test COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/mbnavedit_qt_test.py")
  set_tests_properties(mbnavedit_qt_test PROPERTIES ENVIRONMENT "MBNAVEDIT_QT=$<TARGET_FILE:mbnavedit-qt>;MBNAVEDIT_QT_CANVAS_TEST=$<TARGET_FILE:mbnavedit_qt_canvas_test>;MBNAVEDIT_QT_WINDOW_TEST=$<TARGET_FILE:mbnavedit_qt_window_test>;MBNAVEDIT_QT_EMBED_TEST=$<TARGET_FILE:mbnavedit_qt_embed_test>")
endif()
