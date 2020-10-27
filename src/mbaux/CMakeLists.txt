#
message("src/mbaux:OPENGL_INCLUDE_DIR: ${OPENGL_INCLUDE_DIR}")

foo()

add_library(mbaux
	    STATIC 
	    mb_cheb.c mb_delaun.c mb_intersectgrid.c mb_readwritegrd.c 
            mb_surface.c mb_track.c mb_truecont.c mb_xgraphics.c mb_zgrid.c)
            

