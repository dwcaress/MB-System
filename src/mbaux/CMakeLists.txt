#
message("src/mbaux:OPENGL_INCLUDE_DIR: ${OPENGL_INCLUDE_DIR}")

foo()

# Static library
add_library(mbaux
	    STATIC 
	    mb_cheb.c mb_delaun.c mb_intersectgrid.c mb_readwritegrd.c 
            mb_surface.c mb_track.c mb_truecont.c mb_xgraphics.c mb_zgrid.c)


target_include_directories(mbaux
                           PUBLIC
                           .
                           ${GMT_INCLUDE_DIR}
                           ${CMAKE_SOURCE_DIR}/src/mbio)

# Shared library
add_library(mbauxShared
	    SHARED
	    mb_cheb.c mb_delaun.c mb_intersectgrid.c mb_readwritegrd.c 
            mb_surface.c mb_track.c mb_truecont.c mb_xgraphics.c mb_zgrid.c)
            
target_include_directories(mbauxShared
                           PUBLIC
                           .
                           ${GMT_INCLUDE_DIR}
                           ${CMAKE_SOURCE_DIR}/src/mbio)
