#
# Gets GMT_INCLUDE_DIR from root CMakeLists
#

message("in mbview")

# Static library
add_library(mbview STATIC
            MB3DNavList.c   MBpedit.c               mbview_primary.c
            MB3DRouteList.c   mbpingedit_callbacks.c  mbview_process.c
            Mb3dsdg.c        mbview_bxutils.c        mbview_profile.c
            MB3DSiteList.c    mbview_callbacks.c      mbview_route.c
            mb3dsoundings_callbacks.c  mbview_nav.c   mbview_secondary.c
            MB3DView.c                 mbview_pick.c           mbview_site.c
            mb_glwdrawa.c    mbview_plot.c   mbview_vector.c)



target_include_directories(mbview
                           PUBLIC
                           .
			   ${MOTIF_INCLUDE_DIR}
			   ${X11_INCLUDE_DIR}			   
                           ${OPENGL_INCLUDE_DIR}			   
			   ${X11_INCLUDE_DIR}
                           ${GMT_INCLUDE_DIR}
                           ${CMAKE_SOURCE_DIR}/src/mbio
                           ${CMAKE_SOURCE_DIR}/src/mbaux)


# Building dylib on Apple results in undefined symbols from third-party
# libraries, not sure how to deal with those yet...
if (NOT APPLE)
# Shared library
add_library(mbviewShared SHARED
            MB3DNavList.c   MBpedit.c               mbview_primary.c
            MB3DRouteList.c   mbpingedit_callbacks.c  mbview_process.c
            Mb3dsdg.c        mbview_bxutils.c        mbview_profile.c
            MB3DSiteList.c    mbview_callbacks.c      mbview_route.c
            mb3dsoundings_callbacks.c  mbview_nav.c   mbview_secondary.c
            MB3DView.c                 mbview_pick.c           mbview_site.c
            mb_glwdrawa.c    mbview_plot.c   mbview_vector.c)


set_target_properties(mbviewShared PROPERTIES OUTPUT_NAME mbview)

target_include_directories(mbviewShared
                           PUBLIC
                           .
			   ${CMAKE_INSTALL_PREFIX}/include
			   ${MOTIF_INCLUDE_DIR}			   
			   ${X11_INCLUDE_DIR}
                           ${OPENGL_INCLUDE_DIR}  			   
                           ${GMT_INCLUDE_DIR}
                           ${CMAKE_SOURCE_DIR}/src/mbio
                           ${CMAKE_SOURCE_DIR}/src/mbaux)
                           
endif(NOT APPLE)
