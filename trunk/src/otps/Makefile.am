AM_CPPFLAGS = -I@top_srcdir@/src/mbio -I@top_srcdir@/src/mbaux
AM_LDFLAGS = -L$(top_builddir)/src/mbio -L$(top_builddir)/src/mbaux
LIBS = ${libnetcdf_LIBS} ${libgmt_LIBS} $(top_builddir)/src/mbio/libmbio.la $(top_builddir)/src/mbaux/libmbaux.la $(LIBM)

bin_PROGRAMS = mbotps
mbotps_SOURCES = mbotps.c

nodist_mbotps_SOURCES = otps.h

BUILT_SOURCES = otps.h

CLEANFILES = otps.h
DISTCLEANFILES = otps.h

otps.h: Makefile
	echo 'char *otps_location = "$(OTPS_DIR)";' >$@
