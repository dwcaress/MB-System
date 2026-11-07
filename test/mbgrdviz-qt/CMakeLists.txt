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

# Test of mbgrdviz-qt without drawing (null backend, offscreen platform): the
# main window menus, and sites and routes saved and read again through the
# shared mbgrdviz core.

if(APPLE)
  list(REMOVE_ITEM CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-utils")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(mbgrdviz_qt_test mbgrdviz_qt_test.cc)
set_target_properties(mbgrdviz_qt_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(mbgrdviz_qt_test PRIVATE mbgrdviz-qt-ui)

add_test(NAME mbgrdviz_qt_test COMMAND mbgrdviz_qt_test ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(mbgrdviz_qt_test PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
