bin_PROGRAMS = mbotps

AM_CFLAGS = ${libnetcdf_CFLAGS}

AM_CPPFLAGS = -I${top_srcdir}/src/mbio ${libnetcdf_CPPFLAGS}
if BUILD_GSF
  AM_CPPFLAGS += -I${top_srcdir}/src/gsf
endif

AM_LDFLAGS = 

LIBS = ${top_builddir}/src/mbio/libmbio.la \
	${libgmt_LIBS} ${libnetcdf_LIBS} ${libproj_LIBS} \
	$(LIBM)

mbotps_SOURCES = mbotps.c

nodist_mbotps_SOURCES = otps.h

BUILT_SOURCES = otps.h

CLEANFILES = otps.h
DISTCLEANFILES = otps.h

otps.h: Makefile
	echo 'char *otps_location = "$(OTPS_DIR)";' >$@
