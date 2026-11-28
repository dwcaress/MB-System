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
# The QRhi backend (Metal on macOS, Direct3D 11 on Windows, OpenGL elsewhere)
# draws what the core records in a batch; it needs Qt 6.8 or later with its
# private Gui module and Qt Shader Tools, and is the default on macOS. The
# legacy OpenGL backend runs the mbview drawing code in a 2.1 compatibility
# context; it is built with -DbuildQt6LegacyGL=ON, the default except on macOS,
# and always without QRhi, and is the default elsewhere where it is built.
# MB_RENDERER chooses another backend at run time. The core keeps its OpenGL
# drawing code in every build, so the system OpenGL and GLU stay linked, and on
# macOS GL_SILENCE_DEPRECATION hides the deprecation warnings of the OpenGL
# framework.

if(APPLE)
  list(REMOVE_ITEM CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-utils")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# QRhi needs Qt 6.8 or later, the private Gui module (rhi/qrhi.h) and Qt Shader
# Tools (qt6_add_shaders); without them the views are drawn with OpenGL only
set(mbvq_rhi OFF)
set(mbvq_rhi_headers ON)
if(buildQt6Rhi)
  find_package(Qt6 QUIET COMPONENTS GuiPrivate ShaderTools)
  if(Qt6_VERSION VERSION_GREATER_EQUAL 6.8 AND TARGET Qt6::GuiPrivate AND COMMAND qt6_add_shaders)
    # the private headers themselves: Debian and Ubuntu install the CMake files
    # of Qt6::GuiPrivate and Qt6::CorePrivate with qt6-base-dev but their
    # headers with qt6-base-private-dev, and CMake stops at generate time when
    # an include directory of an imported target is missing; so look at each
    # one (Qt wraps them in a condition on private headers) and draw with
    # OpenGL where one is missing
    foreach(mbvq_private Qt6::GuiPrivate Qt6::CorePrivate)
      if(TARGET ${mbvq_private})
        get_target_property(mbvq_private_dirs ${mbvq_private} INTERFACE_INCLUDE_DIRECTORIES)
        if(NOT mbvq_private_dirs)
          set(mbvq_private_dirs "")
        endif()
        foreach(mbvq_dir IN LISTS mbvq_private_dirs)
          string(REGEX REPLACE "^\\$<\\$<BOOL:.*>>:(.*)>$" "\\1" mbvq_dir "${mbvq_dir}")
          if(mbvq_dir AND NOT mbvq_dir MATCHES "\\$<" AND NOT EXISTS "${mbvq_dir}")
            set(mbvq_rhi_headers OFF)
          endif()
        endforeach()
      endif()
    endforeach()
    if(mbvq_rhi_headers)
      set(mbvq_rhi ON)
    endif()
  endif()
endif()
# the legacy OpenGL backend: with -DbuildQt6LegacyGL=ON, and always without QRhi
set(mbvq_gl ${buildQt6LegacyGL})
if(NOT mbvq_rhi)
  set(mbvq_gl ON)
endif()
# for the tests (test/mbview-qt is added later from the same scope)
set(MBVQ_RHI ${mbvq_rhi} PARENT_SCOPE)
set(MBVQ_GL ${mbvq_gl} PARENT_SCOPE)

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
    ${mbview_dir}/mbv_lists_core.c
    ${mbview_dir}/mbv_gislist_core.c
    ${mbview_dir}/mbv_soundings_core.c
    ${mbview_dir}/mbv_globals.c
    ${mbview_dir}/mbv_render.c
    ${mbview_dir}/mbv_camera.c
    ${mbview_dir}/mbv_marks.c
    ${mbview_dir}/mbv_draw.c)

# static, like the libraries of the other Qt tools: not installed
add_library(
  mbview-qt STATIC
  ${mbview_core}
  mbvq_core.c
  mbvq_pick_cpu.c
  mbvq_render_core.c
  mbvq_bridge.h
  mbvq_dialogs.cc
  mbvq_dialogs.h
  mbvq.h
  mbvq_backend.h
  mbvq_backend_null.cc
  mbvq_input.cc
  mbvq_input.h
  mbvq_lists.cc
  mbvq_lists.h
  mbvq_profile.cc
  mbvq_profile.h
  mbvq_soundings.cc
  mbvq_soundings.h
  mbvq_ui.cc
  mbvq_window.cc
  mbvq_window.h)

set_target_properties(mbview-qt PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_compile_definitions(mbview-qt PUBLIC MBVIEW_NO_MOTIF)
if(APPLE)
  target_compile_definitions(mbview-qt PRIVATE GL_SILENCE_DEPRECATION)
endif()
target_include_directories(mbview-qt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${mbview_dir})

# OpenGL, for the core's OpenGL drawing code, compiled in every build: on macOS
# the system framework, never a Mesa libGL
if(APPLE)
  find_library(MBVQ_OPENGL_FRAMEWORK OpenGL REQUIRED)
  set(mbvq_opengl ${MBVQ_OPENGL_FRAMEWORK})
else()
  find_package(OpenGL REQUIRED)
  set(mbvq_opengl OpenGL::GL OpenGL::GLU)
endif()

target_link_libraries(mbview-qt PUBLIC mbio Qt6::Widgets PRIVATE ${mbvq_opengl})
# mb_define.h includes mb_xdr_win32.h on Windows (src/mbaux); mbio links that
# library privately, and mbview-qt does not use mbaux
if(WIN32)
  target_link_libraries(mbview-qt PUBLIC mb_xdr_win32)
endif()

# the QRhi backend: the default on macOS
if(mbvq_rhi)
  target_sources(mbview-qt PRIVATE mbvq_rhi.h mbvq_rhi_model.cc mbvq_rhi_model.h mbvq_rhi_glu.cc mbvq_rhi_glu.h
                                   mbvq_rhi_frame.cc mbvq_rhi_frame.h mbvq_rhi_renderer.cc mbvq_rhi_renderer.h
                                   mbvq_backend_rhi.cc)
  # GLU's marker vertices are float products, matched bit for bit: no fused
  # multiply-adds (GCC contracts them by default, Clang within expressions)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(mbvq_rhi_glu.cc PROPERTIES COMPILE_OPTIONS -ffp-contract=off)
  endif()
  target_link_libraries(mbview-qt PRIVATE Qt6::GuiPrivate)
  target_compile_definitions(mbview-qt PUBLIC MBVQ_HAVE_RHI)
  qt6_add_shaders(mbview-qt "mbvq_shaders" PREFIX "/mbvq" BASE shaders FILES shaders/mbvq_terrain.vert
                  shaders/mbvq_colour.frag shaders/mbvq_line.vert shaders/mbvq_point.vert shaders/mbvq_wide.vert
                  shaders/mbvq_contour.vert shaders/mbvq_wide_contour.vert shaders/mbvq_marker.vert
                  shaders/mbvq_stipple_thin.vert shaders/mbvq_stipple_wide.vert shaders/mbvq_stipple.frag
                  shaders/mbvq_batch_pointquad.vert shaders/mbvq_points.vert shaders/mbvq_pointquad.vert
                  shaders/mbvq_wide_pairs.vert)
endif()

# the legacy OpenGL backend: the default elsewhere where it is built
if(mbvq_gl)
  find_package(Qt6 REQUIRED COMPONENTS OpenGL OpenGLWidgets)
  target_sources(mbview-qt PRIVATE mbvq_backend_gl.cc)
  target_link_libraries(mbview-qt PRIVATE Qt6::OpenGL Qt6::OpenGLWidgets)
  target_compile_definitions(mbview-qt PUBLIC MBVQ_HAVE_GL)
endif()

# what QRhi needs, for the lines below where it is not built
set(mbvq_rhi_needs "QRhi needs -DbuildQt6Rhi=ON, Qt 6.8 or later, Qt Shader Tools and Qt's private Gui headers")
if(NOT mbvq_rhi_headers)
  set(mbvq_rhi_needs "QRhi needs Qt's private headers, on Debian and Ubuntu in the package qt6-base-private-dev")
endif()
if(mbvq_rhi AND mbvq_gl AND APPLE)
  message(STATUS "mbview-qt: the maps, profiles and 3D soundings are drawn with QRhi, or with OpenGL with MB_RENDERER=legacy-gl")
elseif(mbvq_rhi AND mbvq_gl)
  message(STATUS "mbview-qt: the maps, profiles and 3D soundings are drawn with OpenGL, or with QRhi with MB_RENDERER=auto")
elseif(mbvq_rhi)
  message(STATUS "mbview-qt: the maps, profiles and 3D soundings are drawn with QRhi")
elseif(APPLE)
  message(WARNING "mbview-qt: the maps, profiles and 3D soundings are drawn with OpenGL. ${mbvq_rhi_needs}")
else()
  message(STATUS "mbview-qt: the maps, profiles and 3D soundings are drawn with OpenGL (${mbvq_rhi_needs})")
endif()
