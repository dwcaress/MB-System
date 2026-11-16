#------------------------------------------------------------------------------
#    The MB-system:  CMakeLists.txt   1 November 2026
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

# mbwcdview: Qt6 water column viewer (CMake only, built with -DbuildQt6Tools=ON).
# Needs Qt6 Widgets and libmbio. The swath is drawn with QRhi (Metal on macOS,
# Direct3D 11 on Windows, OpenGL on Linux) when Qt 6.8 or later is found with its
# private Gui module and Qt Shader Tools, and with OpenGL (Qt6 OpenGL and
# OpenGLWidgets) otherwise. -DbuildQt6LegacyGL=ON, the default except on macOS,
# builds the OpenGL drawing too, chosen at run time with MB_RENDERER=legacy-gl.
# The tool is built as two libraries, so that another program can use it, and a
# small program: mbwcdview-core reads the data and traces the rays, without Qt,
# and mbwcdview-ui holds the plot and the windows.

# On macOS Qt uses the system OpenGL framework. build-utils/FindOpenGL.cmake finds
# the X11 Mesa libGL of the Motif programs, so in this directory let Qt's
# find_package(OpenGL) use CMake's own module.
if(APPLE)
  list(REMOVE_ITEM CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-utils")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# QRhi needs Qt 6.8 or later, the private Gui module (rhi/qrhi.h) and Qt Shader
# Tools (qt6_add_shaders); without them the swath is drawn with OpenGL
set(mbwcdview_rhi OFF)
if(buildQt6Rhi)
  find_package(Qt6 QUIET COMPONENTS GuiPrivate ShaderTools)
  if(Qt6_VERSION VERSION_GREATER_EQUAL 6.8 AND TARGET Qt6::GuiPrivate AND COMMAND qt6_add_shaders)
    set(mbwcdview_rhi ON)
  endif()
endif()
set(mbwcdview_gl ${buildQt6LegacyGL})
if(NOT mbwcdview_rhi)
  set(mbwcdview_gl ON)
endif()
if(mbwcdview_gl)
  find_package(Qt6 REQUIRED COMPONENTS OpenGL OpenGLWidgets)
endif()

if(mbwcdview_rhi AND mbwcdview_gl)
  message(STATUS "mbwcdview: the swath is drawn with QRhi, or with OpenGL with MB_RENDERER=legacy-gl")
elseif(mbwcdview_rhi)
  message(STATUS "mbwcdview: the swath is drawn with QRhi")
elseif(APPLE)
  message(WARNING "mbwcdview: the swath is drawn with OpenGL. QRhi needs -DbuildQt6Rhi=ON and Qt 6.8 or later with Qt Shader Tools")
else()
  message(STATUS "mbwcdview: the swath is drawn with OpenGL (QRhi needs Qt 6.8 or later with Qt Shader Tools)")
endif()

add_library(
  mbwcdview-core STATIC
  wcd_data.cc
  wcd_data.h
  wcd_dump.cc
  wcd_dump.h
  wcd_raytrace.cc
  wcd_raytrace.h)

target_include_directories(mbwcdview-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(mbwcdview-core PUBLIC mbio ${MB_SYSLIBS})

# mb_define.h includes the header of this library on Windows, and mbio links it privately
if(WIN32)
  target_link_libraries(mbwcdview-core PUBLIC mb_xdr_win32)
endif()

set(CMAKE_AUTOMOC ON)

add_library(
  mbwcdview-ui STATIC
  WcdView.cc
  WcdView.h
  wcd_plot.cc
  wcd_plot.h
  wcd_scene.cc
  wcd_scene.h
  wcd_widget.cc
  wcd_widget.h
  wcd_window.cc
  wcd_window.h
  viridis.h)

target_link_libraries(mbwcdview-ui PUBLIC mbwcdview-core Qt6::Widgets)

if(mbwcdview_rhi)
  target_sources(mbwcdview-ui PRIVATE wcd_swath_rhi.cc wcd_swath_rhi.h wcd_view_rhi.cc wcd_view_rhi.h)
  target_link_libraries(mbwcdview-ui PRIVATE Qt6::GuiPrivate)
  target_compile_definitions(mbwcdview-ui PUBLIC MBWCD_HAVE_RHI)
  qt6_add_shaders(mbwcdview-ui "mbwcdview_shaders" PREFIX "/mbwcdview" BASE shaders FILES shaders/wcd_swath.vert
                  shaders/wcd_swath.frag)
endif()
if(mbwcdview_gl)
  target_sources(mbwcdview-ui PRIVATE wcd_view_gl.cc wcd_view_gl.h)
  target_link_libraries(mbwcdview-ui PUBLIC Qt6::OpenGL Qt6::OpenGLWidgets)
  target_compile_definitions(mbwcdview-ui PUBLIC MBWCD_HAVE_GL)
endif()

add_executable(mbwcdview mbwcdview.cc)

target_link_libraries(mbwcdview PRIVATE mbwcdview-ui)

# Qt6 needs C++17
set_target_properties(mbwcdview-core mbwcdview-ui mbwcdview PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

install(TARGETS mbwcdview DESTINATION ${CMAKE_INSTALL_BINDIR})
