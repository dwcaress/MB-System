# TODO: X11 / Motif config by autoconf

AM_CPPFLAGS = -I@top_srcdir@/src/mbio -I@top_srcdir@/src/mbaux @MOTIF_INC_DIR@

AM_LDFLAGS = -L$(top_builddir)/src/mbio -L$(top_builddir)/src/mbaux

LIBS = ${libnetcdf_LIBS} ${libgmt_LIBS} @MOTIF_LIB_DIR@ \
       $(top_builddir)/src/mbio/libmbio.la $(top_builddir)/src/mbaux/libmbaux.la \
       $(top_builddir)/src/mbaux/libmbxgr.la \
       -lXm -lXt -lX11 $(LIBM)

bin_PROGRAMS = mbedit
mbedit_SOURCES = mbedit.c mbedit_bxutils.c mbedit_callbacks.c mbedit_creation.c mbedit_prog.c mbedit_creation.h mbedit.h
