AM_CPPFLAGS = -I@top_srcdir@/src/mbio -I@top_srcdir@/src/mbaux @CDF_INC_DIR@ @GMT_INC_DIR@
LIBS = -L@top_srcdir@/src/mbio -L@top_srcdir@/src/mbaux -lmbio
AM_CFLAGS = ${libnetcdf_CFLAGS} ${libgmt_CFLAGS}

bin_PROGRAMS = mbcontour mbgrdtiff mbswath

mbcontour_SOURCES = mbcontour.c
mbcontour_CFLAGS = ${libgmt_CFLAGS}
mbcontour_LDADD = -lmbaux

mbgrdtiff_SOURCES = mbgrdtiff.c
mbgrdtiff_LDADD =

mbswath_SOURCES = mbswath.c
mbswath_CFLAGS = ${libgmt_CFLAGS}
mbswath_LDADD = -lmbaux
