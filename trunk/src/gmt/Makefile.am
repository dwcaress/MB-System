AM_CPPFLAGS = -I.. -I../mbio -I../mbaux -I../surf -I../gsf @CDF_INC_DIR@ @GMT_INC_DIR@
LIBS = -L../mbio -L../gsf -L../proj -L../mr1pr -L../surf
AM_CFLAGS = ${libnetcdf_CFLAGS} ${libgmt_CFLAGS} 

bin_PROGRAMS = mbcontour mbgrdtiff mbswath

mbcontour_SOURCES = mbcontour.c
mbcontour_CFLAGS = -L../mbaux
mbcontour_LDADD = ${libnetcdf_LIBS} ${libgmt_LIBS} \
	-lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux -lmbps

mbgrdtiff_SOURCES = mbgrdtiff.c
mbgrdtiff_LDADD = ${libnetcdf_LIBS} ${libgmt_LIBS} \
	-lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbswath_SOURCES = mbswath.c
mbswath_CFLAGS = -L../mbaux 
mbswath_LDADD = ${libnetcdf_LIBS} ${libgmt_LIBS} \
	-lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux -lmbps


