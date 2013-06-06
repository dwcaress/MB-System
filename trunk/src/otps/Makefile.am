bin_PROGRAMS = mbotps

AM_CPPFLAGS = -I@top_srcdir@/src/mbio -I@top_srcdir@/src/mbaux
LIBS = ${libnetcdf_LIBS} ${libgmt_LIBS} -L@top_srcdir@/src/mbio -L@top_srcdir@/src/mbaux -lmbio
AM_CFLAGS =

mbotps_SOURCES = mbotps.c
mbotps_LDADD = -lmbaux

nodist_mbotps_SOURCES = otps.h

BUILT_SOURCES = otps.h

CLEANFILES = otps.h
DISTCLEANFILES = otps.h

otps.h: Makefile
	echo 'char *otps_location = "$(OTPS_DIR)";' >$@
