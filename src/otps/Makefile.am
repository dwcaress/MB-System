bin_PROGRAMS = mbotps

if BUILD_MBTRN
  MBTRNINCDIR = -I${top_srcdir}/src/mbtrn
endif

AM_CFLAGS = ${libnetcdf_CFLAGS}

AM_CPPFLAGS = -I${top_srcdir}/src/mbio \
        -I${top_srcdir}/src/gsf \
        $(MBTRNINCDIR) \
        ${libnetcdf_CPPFLAGS}

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
