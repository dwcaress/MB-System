bin_PROGRAMS = mbnavadjust

include_HEADERS =

AM_CPPFLAGS = -I@top_srcdir@/src -I@top_srcdir@/src/mbio -I@top_srcdir@/src/mbaux -I@top_srcdir@/src/surf -I@top_srcdir@/src/gsf @MOTIF_INC_DIR@

LIBS = @MOTIF_LIB_DIR@

mbnavadjust_SOURCES = mbnavadjust.c mbnavadjust_callbacks.c mbnavadjust_creation.c \
	mbnavadjust_prog.c mbnavadjust_util.c mbnavadjust_creation.h \
	mbnavadjust_extrawidgets.h mbnavadjust.h

mbnavadjust_LDADD = -L@top_srcdir@/src/mbio -L@top_srcdir@/src/gsf -L@top_srcdir@/src/proj -L@top_srcdir@/src/mr1pr -L@top_srcdir@/src/surf -L@top_srcdir@/src/mbaux @MOTIF_LIB_DIR@ -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux -lmbxgr -lXm -lXt -lX11
