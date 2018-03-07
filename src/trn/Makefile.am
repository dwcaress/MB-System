bin_PROGRAMS = mbtrnpreprocess

AM_CFLAGS = ${libnetcdf_CFLAGS}

AM_CPPFLAGS = -I${top_srcdir}/src/mbio \
	      -I${top_srcdir}/src/mbaux \
	      -I${top_srcdir}/src/gsf \
	      ${libnetcdf_CPPFLAGS}

AM_LDFLAGS = -L$(top_builddir)/src/gsf

LIBS = $(top_builddir)/src/mbio/libmbio.la \
       $(top_builddir)/src/mbaux/libmbaux.la \
       $(top_builddir)/src/mbaux/libmbxgr.la \
       ${libgmt_LIBS} ${libnetcdf_LIBS} ${libproj_LIBS} \
      $(LIBM)

mbtrnpreprocess_SOURCES = mbtrnpreprocess.c
