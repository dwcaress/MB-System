#
# Gets GMT_INCLUDE_DIR from root CMakeLists
#

message("in mbaux")

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
                           

#[[
# Shared library
add_library(mbauxShared
	    SHARED
	    mb_cheb.c mb_delaun.c mb_intersectgrid.c mb_readwritegrd.c 
            mb_surface.c mb_track.c mb_truecont.c mb_zgrid.c)
            
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


target_include_directories(mbxgrShared
                           PUBLIC
                           .
			   ${X11_INCLUDE_DIR}			   
                           ${GMT_INCLUDE_DIR}
                           ${GDAL_INCLUDE_DIR}
                           ${CMAKE_SOURCE_DIR}/src/mbio)
]]
