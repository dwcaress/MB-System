bin_PROGRAMS = mbotps

AM_CFLAGS = ${libnetcdf_CFLAGS}

AM_CPPFLAGS =
AM_CPPFLAGS += -I${top_srcdir}/src/mbio
AM_CPPFLAGS += -I${top_srcdir}/src/gsf
AM_CPPFLAGS += ${libnetcdf_CPPFLAGS}

AM_LDFLAGS =

LIBS =
LIBS += ${top_builddir}/src/mbio/libmbio.la
LIBS += ${libgmt_LIBS}
LIBS += ${libnetcdf_LIBS}
LIBS += ${libproj_LIBS}
LIBS += $(LIBM)

mbotps_SOURCES = mbotps.c

nodist_mbotps_SOURCES = otps.h

BUILT_SOURCES = otps.h

CLEANFILES = otps.h
DISTCLEANFILES = otps.h

otps.h: Makefile
	echo 'char *otps_location = "$(OTPS_DIR)";' >$@
