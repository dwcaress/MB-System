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

# Tests of mbedit-qt: without a window, and a program that drives the window with
# mouse and key events, run by mbedit_qt_test.py, which writes the synthetic swath files.

if(APPLE)
  list(REMOVE_ITEM CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-utils")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)

add_executable(mbedit_qt_window_test mbedit_qt_window_test.cc)

set_target_properties(mbedit_qt_window_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

target_link_libraries(mbedit_qt_window_test PRIVATE mbedit-qt-ui)

add_executable(mbedit_qt_session_test mbedit_qt_session_test.cc)

set_target_properties(mbedit_qt_session_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

target_link_libraries(mbedit_qt_session_test PRIVATE mbedit-qt-core)

add_executable(mbedit_qt_embed_test mbedit_qt_embed_test.cc)

set_target_properties(mbedit_qt_embed_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

target_link_libraries(mbedit_qt_embed_test PRIVATE mbedit-qt-ui)

find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND)
  add_test(NAME mbedit_qt_test COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/mbedit_qt_test.py")
  set_tests_properties(mbedit_qt_test PROPERTIES ENVIRONMENT
                       "MBEDIT_QT=$<TARGET_FILE:mbedit-qt>;MBEDIT_QT_WINDOW_TEST=$<TARGET_FILE:mbedit_qt_window_test>;MBEDIT_QT_SESSION_TEST=$<TARGET_FILE:mbedit_qt_session_test>;MBEDIT_QT_EMBED_TEST=$<TARGET_FILE:mbedit_qt_embed_test>;MBCLEAN=$<TARGET_FILE:mbclean>;MBGRID=$<TARGET_FILE:mbgrid>;MBHISTORY=$<TARGET_FILE:mbhistory>")
endif()
