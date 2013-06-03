bin_PROGRAMS = mbnavedit

include_HEADERS =

AM_CPPFLAGS = -I@top_srcdir@/src -I@top_srcdir@/src/mbio -I@top_srcdir@/src/mbaux -I@top_srcdir@/src/surf -I@top_srcdir@/src/gsf @MOTIF_INC_DIR@

LIBS = @MOTIF_LIB_DIR@

mbnavedit_SOURCES = mbnavedit.c mbnavedit_creation.c mbnavedit_util.c mbnavedit_callbacks.c \
	mbnavedit_prog.c mbnavedit_creation.h mbnavedit_extrawidgets.h mbnavedit.h

mbnavedit_LDADD = -L@top_srcdir@/src/mbio -L@top_srcdir@/src/gsf -L@top_srcdir@/src/proj -L@top_srcdir@/src/mr1pr -L@top_srcdir@/src/surf -L@top_srcdir@/src/mbaux @MOTIF_LIB_DIR@ -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux -lmbxgr -lXm -lXt -lX11
