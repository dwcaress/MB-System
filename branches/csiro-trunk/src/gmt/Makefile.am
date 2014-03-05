bin_PROGRAMS = mbcontour mbgrdtiff mbswath

AM_CFLAGS = ${libgmt_CFLAGS} ${libnetcdf_CFLAGS}

AM_CPPFLAGS = -I${top_srcdir}/src/mbio \
	      -I${top_srcdir}/src/mbaux \
	      ${libgmt_CPPFLAGS} \
	      ${libnetcdf_CPPFLAGS}

AM_LDFLAGS = 

LIBS = $(top_builddir)/src/mbio/libmbio.la \
	${libgmt_LIBS} ${libnetcdf_LIBS} \
	$(LIBM)

mbcontour_SOURCES = mbcontour.c
mbcontour_LDADD = $(top_builddir)/src/mbaux/libmbaux.la $(top_builddir)/src/mbaux/libmbps.la

mbgrdtiff_SOURCES = mbgrdtiff.c

mbswath_SOURCES = mbswath.c
mbswath_LDADD = $(top_builddir)/src/mbaux/libmbaux.la
