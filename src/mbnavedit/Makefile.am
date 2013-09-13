bin_PROGRAMS = mbnavedit

AM_CFLAGS = ${libnetcdf_CFLAGS}

AM_CPPFLAGS = -I@top_srcdir@/src/mbio -I@top_srcdir@/src/mbaux @CDF_INC_DIR@ @MOTIF_INC_DIR@
if BUILD_GSF
  AM_CPPFLAGS += -I@top_srcdir@/src/gsf
endif

AM_LDFLAGS = -L$(top_builddir)/src/mbio -L$(top_builddir)/src/mbaux -L$(top_builddir)/src/mr1pr -L$(top_builddir)/src/proj -L$(top_builddir)/src/surf ${libgmt_LDFLAGS} ${libnetcdf_LDFLAGS} ${libgmt_RPATH} ${libnetcdf_RPATH}
if BUILD_GSF
  AM_LDFLAGS += -L$(top_builddir)/src/gsf
endif

LIBS = ${libgmt_LIBS} ${libnetcdf_LIBS} @MOTIF_LIB_DIR@ \
       $(top_builddir)/src/mbio/libmbio.la $(top_builddir)/src/mbaux/libmbaux.la \
       $(top_builddir)/src/mbaux/libmbxgr.la \
       -lXm -lXt -lX11 $(LIBM)

mbnavedit_SOURCES = mbnavedit.c mbnavedit_creation.c mbnavedit_util.c mbnavedit_callbacks.c \
	mbnavedit_prog.c mbnavedit_creation.h mbnavedit_extrawidgets.h mbnavedit.h
