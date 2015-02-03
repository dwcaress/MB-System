lib_LTLIBRARIES = libmbgmt.la

include_HEADERS = 

AM_CPPFLAGS = -I${top_srcdir}/src/mbio \
	      -I${top_srcdir}/src/mbaux \
	      ${libgmt_CPPFLAGS} \
	      ${libnetcdf_CPPFLAGS}
AM_CFLAGS = ${libgmt_CFLAGS} ${libnetcdf_CFLAGS}

libmbgmt_la_LDFLAGS = -no-undefined -version-info 0:0:0
libmbgmt_la_SOURCES = mbswath.c mbcontour.c mbgrdtiff.c
libmbgmt_la_LIBADD = ${top_builddir}/src/mbio/libmbio.la \
			${top_builddir}/src/mbaux/libmbaux.la \
			${libgmt_LIBS} ${libnetcdf_LIBS}
