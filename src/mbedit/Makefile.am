bin_PROGRAMS = mbedit

include_HEADERS =

AM_CPPFLAGS = -I@top_srcdir@/src/mbio -I@top_srcdir@/src/mbaux @MOTIF_INC_DIR@
LIBS = ${libnetcdf_LIBS} ${libgmt_LIBS} @MOTIF_LIB_DIR@ -L@top_srcdir@/src/mbio -L@top_srcdir@/src/mbaux -lmbio
AM_CFLAGS =

mbedit_SOURCES = mbedit.c mbedit_bxutils.c mbedit_callbacks.c mbedit_creation.c mbedit_prog.c mbedit_creation.h mbedit.h
mbedit_LDADD = -lmbaux -lmbxgr -lXm -lXt -lX11
