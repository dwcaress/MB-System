AM_CPPFLAGS = -I@top_srcdir@/src -I@top_srcdir@/src/mbio -I@top_srcdir@/src/mbaux -I@top_srcdir@/src/surf -I@top_srcdir@/src/gsf @CDF_INC_DIR@ @GMT_INC_DIR@
LIBS = -L@top_srcdir@/src/mbio -L@top_srcdir@/src/gsf -L@top_srcdir@/src/proj -L@top_srcdir@/src/mr1pr -L@top_srcdir@/src/surf
AM_CFLAGS = ${libnetcdf_CFLAGS} ${libgmt_CFLAGS}

bin_PROGRAMS = mbcontour mbgrdtiff mbswath

mbcontour_SOURCES = mbcontour.c
mbcontour_CFLAGS = ${libgmt_CFLAGS} -L@top_srcdir@/src/mbaux
mbcontour_LDADD = ${libnetcdf_LIBS} ${libgmt_LIBS} \
	-lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux -lmbps

mbgrdtiff_SOURCES = mbgrdtiff.c
mbgrdtiff_LDADD = ${libnetcdf_LIBS} ${libgmt_LIBS} \
	-lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbswath_SOURCES = mbswath.c
mbswath_CFLAGS = ${libgmt_CFLAGS} -L@top_srcdir@/src/mbaux
mbswath_LDADD = ${libnetcdf_LIBS} ${libgmt_LIBS} \
	-lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux -lmbps
