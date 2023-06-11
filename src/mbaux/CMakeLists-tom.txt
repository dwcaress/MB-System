#
# Gets GMT_INCLUDE_DIRS from root CMakeLists
#

message("in mbaux")

# Static or shared, depends on BUILD_SHARED_LIBS
add_library(mbaux
	    mb_cheb.c mb_delaun.c mb_intersectgrid.c mb_readwritegrd.c 
            mb_surface.c mb_track.c mb_truecont.c mb_zgrid.c)

IF (APPLE)
  # Resolve missing symbols at runtime
  set_target_properties(mbaux PROPERTIES LINK_FLAGS
                  "-undefined dynamic_lookup -flat_namespace")
ENDIF (APPLE)

target_include_directories(mbaux
                           PUBLIC
                           .
			   ${X11_INCLUDE_DIRS}			   
                           ${GMT_INCLUDE_DIRS}
                           ${GDAL_INCLUDE_DIRS}
                           ${CMAKE_SOURCE_DIR}/src/mbio)

# add static or shared library, depending on BUILD_SHARED_LIBS
add_library(mbxgr
            mb_xgraphics.c)


IF (APPLE)
  # Resolve missing symbols at runtime
  set_target_properties(mbxgr PROPERTIES LINK_FLAGS
                  "-undefined dynamic_lookup -flat_namespace")
ENDIF (APPLE)

target_include_directories(mbxgr
                           PUBLIC
                           .
			   ${X11_INCLUDE_DIRS}			   
                           ${GMT_INCLUDE_DIRS}
                           ${GDAL_INCLUDE_DIRS}
                           ${CMAKE_SOURCE_DIR}/src/mbio)

install(TARGETS mbaux mbxgr
	DESTINATION ${CMAKE_INSTALL_LIBDIR})

