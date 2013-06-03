bin_PROGRAMS = mbotps

AM_CPPFLAGS = -I@top_srcdir@/src -I@top_srcdir@/src/mbio -I@top_srcdir@/src/mbaux -I@top_srcdir@/src/surf -I@top_srcdir@/src/gsf @CDF_INC_DIR@ @GMT_INC_DIR@

mbotps_SOURCES = mbotps.c
mbotps_CFLAGS = ${libnetcdf_CFLAGS} ${libgmt_CFLAGS} -L@top_srcdir@/src/mbio -L@top_srcdir@/src/gsf -L@top_srcdir@/src/proj -L@top_srcdir@/src/mr1pr -L@top_srcdir@/src/surf
mbotps_LDADD = ${libnetcdf_LIBS} ${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

nodist_mbotps_SOURCES = otps.h

BUILT_SOURCES = otps.h

CLEANFILES = otps.h
DISTCLEANFILES = otps.h

otps.h: Makefile
	echo 'char *otps_location = "$(OTPS_DIR)";' >$@
