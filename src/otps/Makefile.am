bin_PROGRAMS = mbotps

AM_CFLAGS = ${libnetcdf_CFLAGS}

AM_CPPFLAGS = -I@top_srcdir@/src/mbio @CDF_INC_DIR@
if BUILD_GSF
  AM_CPPFLAGS += -I@top_srcdir@/src/gsf
endif

AM_LDFLAGS = -L$(top_builddir)/src/mbio -L$(top_builddir)/src/mr1pr -L$(top_builddir)/src/proj -L$(top_builddir)/src/surf
if BUILD_GSF
  AM_LDFLAGS += -L$(top_builddir)/src/gsf
endif

LIBS = ${libnetcdf_LIBS} $(top_builddir)/src/mbio/libmbio.la $(LIBM)

mbotps_SOURCES = mbotps.c

nodist_mbotps_SOURCES = otps.h

BUILT_SOURCES = otps.h

CLEANFILES = otps.h
DISTCLEANFILES = otps.h

otps.h: Makefile
	echo 'char *otps_location = "$(OTPS_DIR)";' >$@
