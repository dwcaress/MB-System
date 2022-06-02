#
# Gets GMT_INCLUDE_DIRS from root CMakeLists
#

message("in mbview")

# add static or shared library, depending on BUILD_SHARED_LIBS
add_library(mbview
            MB3DNavList.c   MBpedit.c               mbview_primary.c
            MB3DRouteList.c   mbpingedit_callbacks.c  mbview_process.c
            Mb3dsdg.c        mbview_bxutils.c        mbview_profile.c
            MB3DSiteList.c    mbview_callbacks.c      mbview_route.c
            mb3dsoundings_callbacks.c  mbview_nav.c   mbview_secondary.c
            MB3DView.c                 mbview_pick.c           mbview_site.c
            mb_glwdrawa.c    mbview_plot.c   mbview_vector.c)

IF (APPLE)
  # Resolve missing symbols at runtime
  set_target_properties(mbview PROPERTIES LINK_FLAGS
                  "-undefined dynamic_lookup -flat_namespace")
ENDIF (APPLE)

target_include_directories(mbview
                           PUBLIC
                           .
			   ${CMAKE_INSTALL_PREFIX}/include
			   ${MOTIF_INCLUDE_DIRS}			   
			   ${X11_INCLUDE_DIRS}
                           ${OPENGL_INCLUDE_DIRS}  			   
                           ${GMT_INCLUDE_DIRS}
                           ${CMAKE_SOURCE_DIR}/src/mbio
                           ${CMAKE_SOURCE_DIR}/src/mbaux)
                           
install(TARGETS mbview 
	DESTINATION ${CMAKE_INSTALL_LIBDIR})
