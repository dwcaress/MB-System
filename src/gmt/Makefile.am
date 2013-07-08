AM_CFLAGS = ${libnetcdf_CFLAGS} ${libgmt_CFLAGS}

AM_CPPFLAGS = -I@top_srcdir@/src/mbio -I@top_srcdir@/src/mbaux @CDF_INC_DIR@ @GMT_INC_DIR@

AM_LDFLAGS = -L$(top_builddir)/src/mbio -L$(top_builddir)/src/mbaux -L$(top_builddir)/src/mr1pr -L$(top_builddir)/src/proj -L$(top_builddir)/src/surf
if BUILD_GSF
  AM_LDFLAGS +=  -L$(top_builddir)/src/gsf
endif

LIBS = ${libnetcdf_LIBS} ${libgmt_LIBS} $(top_builddir)/src/mbio/libmbio.la $(LIBM)

bin_PROGRAMS = mbcontour mbgrdtiff mbswath

mbcontour_SOURCES = mbcontour.c
mbcontour_LDADD = $(top_builddir)/src/mbaux/libmbaux.la $(top_builddir)/src/mbaux/libmbps.la

mbgrdtiff_SOURCES = mbgrdtiff.c

mbswath_SOURCES = mbswath.c
mbswath_LDADD = $(top_builddir)/src/mbaux/libmbaux.la
