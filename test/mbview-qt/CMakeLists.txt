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
# the OpenGL backend, and --rhi with the QRhi backend where it is built; they
# need a display and are not run by ctest.

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

# The 3D soundings window: ctest runs the checks without drawing;
# --gl, --rhi (the QRhi backend, where it is built), --time and --show need a
# display.
add_executable(mbview_qt_soundings_test mbview_qt_soundings_test.cc)
set_target_properties(mbview_qt_soundings_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(mbview_qt_soundings_test PRIVATE mbview-qt mbio Qt6::Widgets Qt6::OpenGLWidgets ${MB_SYSLIBS})

add_test(NAME mbview_qt_soundings_test COMMAND mbview_qt_soundings_test --null)
set_tests_properties(mbview_qt_soundings_test PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# Maps and the 3D soundings editor inside another program's window: ctest runs
# the checks without drawing; --gl needs a display.
add_executable(mbview_qt_embed_test mbview_qt_embed_test.cc)
set_target_properties(mbview_qt_embed_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(mbview_qt_embed_test PRIVATE mbview-qt mbio Qt6::Widgets Qt6::OpenGLWidgets ${MB_SYSLIBS})

add_test(NAME mbview_qt_embed_test COMMAND mbview_qt_embed_test --null)
set_tests_properties(mbview_qt_embed_test PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# The QRhi backend, where it is built (src/mbview-qt sets MBVQ_RHI): four
# headless modes on the offscreen platform, each with its own watchdog. The
# Metal mode returns 77 (skipped) where no Metal QRhi can be made. --display
# and --bench (frame times, also of the OpenGL backend) draw in a window and
# need a display; they are not run by ctest.
if(MBVQ_RHI)
  add_executable(mbview_qt_rhi_test mbview_qt_rhi_test.cc)
  set_target_properties(mbview_qt_rhi_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
  target_link_libraries(mbview_qt_rhi_test PRIVATE mbview-qt mbaux mbio Qt6::Widgets ${MB_SYSLIBS})
  add_test(NAME mbview_qt_rhi_model COMMAND mbview_qt_rhi_test --model)
  add_test(NAME mbview_qt_rhi_widgets COMMAND mbview_qt_rhi_test --widgets)
  add_test(NAME mbview_qt_rhi_null COMMAND mbview_qt_rhi_test --render-null)
  set_tests_properties(mbview_qt_rhi_model mbview_qt_rhi_widgets mbview_qt_rhi_null
                       PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 60)
  # the widgets again at a pixel ratio of 2, where the profile view's cap is
  # half as wide
  add_test(NAME mbview_qt_rhi_widgets_ratio2 COMMAND mbview_qt_rhi_test --widgets)
  set_tests_properties(mbview_qt_rhi_widgets_ratio2 PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_SCALE_FACTOR=2"
                                                               TIMEOUT 60)
  if(APPLE)
    add_test(NAME mbview_qt_rhi_metal COMMAND mbview_qt_rhi_test --render-metal)
    set_tests_properties(mbview_qt_rhi_metal PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 60
                                                        SKIP_RETURN_CODE 77)
  endif()
endif()
