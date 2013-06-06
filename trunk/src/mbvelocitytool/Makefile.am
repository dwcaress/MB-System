bin_PROGRAMS = mbvelocitytool

include_HEADERS =

AM_CPPFLAGS = -I@top_srcdir@/src/mbio -I@top_srcdir@/src/mbaux @MOTIF_INC_DIR@
LIBS = ${libnetcdf_LIBS} ${libgmt_LIBS} @MOTIF_LIB_DIR@ -L@top_srcdir@/src/mbio -L@top_srcdir@/src/mbaux -lmbio
AM_CFLAGS =

mbvelocitytool_SOURCES = mbvelocity_bxutils.c mbvelocity.c mbvelocity_callbacks.c \
        mbvelocity_creation.c mbvelocity_prog.c mbvelocity_creation.h mbvelocity.h
mbvelocitytool_LDADD = -lmbaux -lmbxgr -lXm -lXt -lX11
