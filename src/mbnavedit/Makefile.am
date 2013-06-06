bin_PROGRAMS = mbnavedit

include_HEADERS =

AM_CPPFLAGS = -I@top_srcdir@/src/mbio -I@top_srcdir@/src/mbaux @MOTIF_INC_DIR@
LIBS = ${libnetcdf_LIBS} ${libgmt_LIBS} @MOTIF_LIB_DIR@ -L@top_srcdir@/src/mbio -L@top_srcdir@/src/mbaux -lmbio
AM_CFLAGS =

mbnavedit_SOURCES = mbnavedit.c mbnavedit_creation.c mbnavedit_util.c mbnavedit_callbacks.c \
	mbnavedit_prog.c mbnavedit_creation.h mbnavedit_extrawidgets.h mbnavedit.h
mbnavedit_LDADD = -lmbaux -lmbxgr -lXm -lXt -lX11
