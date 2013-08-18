bin_PROGRAMS = mbedit

AM_CFLAGS = ${libnetcdf_CFLAGS}

AM_CPPFLAGS = -I@top_srcdir@/src/mbio -I@top_srcdir@/src/mbaux @CDF_INC_DIR@ @MOTIF_INC_DIR@
if BUILD_GSF
  AM_CPPFLAGS += -I@top_srcdir@/src/gsf
endif

AM_LDFLAGS = -L$(top_builddir)/src/mbio -L$(top_builddir)/src/mbaux -L$(top_builddir)/src/mr1pr -L$(top_builddir)/src/proj -L$(top_builddir)/src/surf
if BUILD_GSF
  AM_LDFLAGS += -L$(top_builddir)/src/gsf
endif

LIBS = ${libnetcdf_LIBS} @MOTIF_LIB_DIR@ \
       $(top_builddir)/src/mbio/libmbio.la $(top_builddir)/src/mbaux/libmbaux.la \
       $(top_builddir)/src/mbaux/libmbxgr.la \
       -lXm -lXt -lX11 $(LIBM)

mbedit_SOURCES = mbedit.c mbedit_bxutils.c mbedit_callbacks.c mbedit_creation.c mbedit_prog.c mbedit_creation.h mbedit.h
