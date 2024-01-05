#------------------------------------------------------------------------------
#    The MB-system:  CMakeLists.txt   27 June 2023
#
#    Copyright (c) 2023-2024 by
#    David W. Caress (caress@mbari.org)
#      Monterey Bay Aquarium Research Institute
#      Moss Landing, California, USA
#    Dale N. Chayes 
#      Center for Coastal and Ocean Mapping
#      University of New Hampshire
#      Durham, New Hampshire, USA
#    Christian dos Santos Ferreira
#      MARUM
#      University of Bremen
#      Bremen Germany
#      
#    MB-System was created by Caress and Chayes in 1992 at the
#      Lamont-Doherty Earth Observatory
#      Columbia University
#      Palisades, NY 10964
#
#    See README.md file for copying and redistribution conditions.
#------------------------------------------------------------------------------

message("In src/mbview")

find_package(X11 REQUIRED)
find_package(Motif REQUIRED)
find_package(OpenGL REQUIRED)

add_library(
  mbview
  MB3DNavList.c
  MBpedit.c
  mbview_primary.c
  MB3DRouteList.c
  mbpingedit_callbacks.c
  mbview_process.c
  Mb3dsdg.c
  mbview_bxutils.c
  mbview_profile.c
  MB3DSiteList.c
  mbview_callbacks.c
  mbview_route.c
  mb3dsoundings_callbacks.c
  mbview_nav.c
  mbview_secondary.c
  MB3DView.c
  mbview_pick.c
  mbview_site.c
  mb_glwdrawa.c
  mbview_plot.c
  mbview_vector.c)

set_target_properties(mbview PROPERTIES VERSION "0" SOVERSION "0")
target_include_directories(mbview PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mbview PRIVATE OpenGL::GL OpenGL::GLU mbio
	             ${MOTIF_LIBRARIES}
		     ${X11_LIBRARIES}
		     ${X11_Xt_LIB})


install(TARGETS mbview DESTINATION ${CMAKE_INSTALL_LIBDIR})
