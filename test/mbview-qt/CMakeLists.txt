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

# Tests of the mbview-qt library. ctest runs the checks without drawing (null
# backend, offscreen platform). mbview_qt_test --gl checks drawing and picks with
# the OpenGL backend and needs a display; it is not run by ctest.

if(APPLE)
  list(REMOVE_ITEM CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-utils")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)

set(CMAKE_AUTOMOC ON)

add_executable(mbview_qt_test mbview_qt_test.cc)
set_target_properties(mbview_qt_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(mbview_qt_test PRIVATE mbview-qt mbaux mbio Qt6::Widgets Qt6::OpenGLWidgets ${MB_SYSLIBS})

add_test(NAME mbview_qt_test COMMAND mbview_qt_test --null)
set_tests_properties(mbview_qt_test PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
