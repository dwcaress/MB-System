bin_PROGRAMS = mbotps

INCLUDES = -I.. -I../mbio -I../mbaux -I../surf -I../gsf @CDF_INC_DIR@ @GMT_INC_DIR@

mbotps_SOURCES = mbotps.c
mbotps_CFLAGS = ${libnetcdf_CFLAGS} ${libgmt_CFLAGS} -L../mbio -L../gsf -L../proj -L../mr1pr -L../surf
mbotps_LDADD = ${libnetcdf_LIBS} ${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

nodist_mbotps_SOURCES = otps.h

BUILT_SOURCES = otps.h

CLEANFILES = otps.h
DISTCLEANFILES = otps.h

otps.h: Makefile
	echo 'char *otps_location = "$(OTPS_DIR)";' >$@
