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

# mbview-qt: the mbview library with a Qt6 user interface, for mbgrdviz-qt and
# mbeditviz-qt. The drawing core is the shared code of src/mbview, compiled here
# without Motif (MBVIEW_NO_MOTIF); the Motif library in src/mbview is unchanged.
# The legacy OpenGL backend runs the mbview drawing code in a 2.1 compatibility
# context; on macOS GL_SILENCE_DEPRECATION hides the deprecation warnings of the
# OpenGL framework.

if(APPLE)
  list(REMOVE_ITEM CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-utils")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)

set(CMAKE_AUTOMOC ON)

set(mbview_dir "${CMAKE_SOURCE_DIR}/src/mbview")

# the mbview core files, shared with the Motif library
set(mbview_core
    ${mbview_dir}/mbview_plot.c
    ${mbview_dir}/mbview_process.c
    ${mbview_dir}/mbview_primary.c
    ${mbview_dir}/mbview_secondary.c
    ${mbview_dir}/mbview_pick.c
    ${mbview_dir}/mbview_nav.c
    ${mbview_dir}/mbview_route.c
    ${mbview_dir}/mbview_site.c
    ${mbview_dir}/mbview_vector.c
    ${mbview_dir}/mbview_gispoint.c
    ${mbview_dir}/mbview_polygon.c
    ${mbview_dir}/mbview_polyline.c
    ${mbview_dir}/mbview_profile.c
    ${mbview_dir}/mbv_core.c
    ${mbview_dir}/mbv_soundings_core.c
    ${mbview_dir}/mbv_globals.c
    ${mbview_dir}/mbv_render.c)

add_library(
  mbview-qt
  ${mbview_core}
  mbvq_core.c
  mbvq_bridge.h
  mbvq_dialogs.cc
  mbvq_dialogs.h
  mbvq.h
  mbvq_backend.h
  mbvq_backend_gl.cc
  mbvq_backend_null.cc
  mbvq_input.cc
  mbvq_input.h
  mbvq_ui.cc
  mbvq_window.cc
  mbvq_window.h)

set_target_properties(mbview-qt PROPERTIES VERSION "0" SOVERSION "0" CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_compile_definitions(mbview-qt PUBLIC MBVIEW_NO_MOTIF)
if(APPLE)
  target_compile_definitions(mbview-qt PRIVATE GL_SILENCE_DEPRECATION)
endif()
target_include_directories(mbview-qt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${mbview_dir})

# OpenGL: on macOS the system framework, never a Mesa libGL
if(APPLE)
  find_library(MBVQ_OPENGL_FRAMEWORK OpenGL REQUIRED)
  set(mbvq_opengl ${MBVQ_OPENGL_FRAMEWORK})
else()
  find_package(OpenGL REQUIRED)
  set(mbvq_opengl OpenGL::GL OpenGL::GLU)
endif()

target_link_libraries(mbview-qt PUBLIC mbio Qt6::Widgets PRIVATE Qt6::OpenGL Qt6::OpenGLWidgets ${mbvq_opengl}
                                                                   ${MB_SYSLIBS})

install(TARGETS mbview-qt DESTINATION ${CMAKE_INSTALL_LIBDIR})
