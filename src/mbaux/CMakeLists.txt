#
# Gets GMT_INCLUDE_DIR from root CMakeLists
#

message("in mbaux")
message("CMAKE_INSTALL_LIBDIR: ${CMAKE_INSTALL_LIBDIR}")
message("CMAKE_INSTALL_DATADIR: ${CMAKE_INSTALL_DATADIR}")

# Static library
add_library(mbaux
	    STATIC 
	    mb_cheb.c mb_delaun.c mb_intersectgrid.c mb_readwritegrd.c 
            mb_surface.c mb_track.c mb_truecont.c mb_zgrid.c)


target_include_directories(mbaux
                           PUBLIC
                           .
			   ${X11_INCLUDE_DIR}
                           ${GMT_INCLUDE_DIR}
                           ${GDAL_INCLUDE_DIR}
                           ${CMAKE_SOURCE_DIR}/src/mbio)

# Static library
add_library(mbxgr
	    STATIC 
            mb_xgraphics.c)


target_include_directories(mbxgr
                           PUBLIC
                           .
			   ${X11_INCLUDE_DIR}			   
                           ${GMT_INCLUDE_DIR}
                           ${GDAL_INCLUDE_DIR}
                           ${CMAKE_SOURCE_DIR}/src/mbio)
                           

# Install
message("set up mbaux install targets")
message("CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")

install(TARGETS mbaux mbxgr
	DESTINATION ${CMAKE_INSTALL_LIBDIR})

## install(TARGETS mbaux DESTINATION ${CMAKE_INSTALL_LIBDIR})
## install(TARGETS mbxgr DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Shared library
add_library(mbauxShared
	    SHARED
	    mb_cheb.c mb_delaun.c mb_intersectgrid.c mb_readwritegrd.c 
            mb_surface.c mb_track.c mb_truecont.c mb_zgrid.c)

set_target_properties(mbauxShared PROPERTIES OUTPUT_NAME mbaux)

IF (APPLE)
  # Resolve missing symbols at runtime
  set_target_properties(mbauxShared PROPERTIES LINK_FLAGS
                  "-undefined dynamic_lookup -flat_namespace")
ENDIF (APPLE)

target_include_directories(mbauxShared
                           PUBLIC
                           .
			   ${X11_INCLUDE_DIR}			   
                           ${GMT_INCLUDE_DIR}
                           ${GDAL_INCLUDE_DIR}
                           ${CMAKE_SOURCE_DIR}/src/mbio)



# Shared library
add_library(mbxgrShared
	    SHARED
            mb_xgraphics.c)

set_target_properties(mbxgrShared PROPERTIES OUTPUT_NAME mbxgr)

IF (APPLE)
  # Resolve missing symbols at runtime
  set_target_properties(mbxgrShared PROPERTIES LINK_FLAGS
                  "-undefined dynamic_lookup -flat_namespace")
ENDIF (APPLE)

target_include_directories(mbxgrShared
                           PUBLIC
                           .
			   ${X11_INCLUDE_DIR}			   
                           ${GMT_INCLUDE_DIR}
                           ${GDAL_INCLUDE_DIR}
                           ${CMAKE_SOURCE_DIR}/src/mbio)


install(TARGETS mbauxShared DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS mbxgrShared DESTINATION ${CMAKE_INSTALL_LIBDIR})


