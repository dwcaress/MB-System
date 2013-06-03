bin_PROGRAMS = mbvelocitytool

include_HEADERS =

AM_CPPFLAGS = -I@top_srcdir@/src -I@top_srcdir@/src/mbio -I@top_srcdir@/src/mbaux -I@top_srcdir@/src/surf -I@top_srcdir@/src/gsf @MOTIF_INC_DIR@

LIBS = @MOTIF_LIB_DIR@

mbvelocitytool_SOURCES = mbvelocity_bxutils.c mbvelocity.c mbvelocity_callbacks.c \
mbvelocity_creation.c mbvelocity_prog.c mbvelocity_creation.h mbvelocity.h
mbvelocitytool_LDADD = -L@top_srcdir@/src/mbio -L@top_srcdir@/src/gsf -L@top_srcdir@/src/proj -L@top_srcdir@/src/mr1pr -L@top_srcdir@/src/surf -L@top_srcdir@/src/mbaux @MOTIF_LIB_DIR@ -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbxgr -lXm -lXt -lX11
