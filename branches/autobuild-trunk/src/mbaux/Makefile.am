INCLUDES = -I.. -I../mbio @CDF_INC_DIR@ @GMT_INC_DIR@
AM_CFLAGS = ${libnetcdf_CFLAGS} ${libgmt_CFLAGS}

if BUILD_MOTIF
  XGRINC = mb_xgraphics.h
endif
include_HEADERS = mb_aux.h $(XGRINC)

if BUILD_MOTIF
  XGRLIB = libmbxgr.la
endif
lib_LTLIBRARIES = libmbps.la libmbaux.la $(XGRLIB)

libmbps_la_LDFLAGS = -no-undefined -version-info 0:0:0
libmbps_la_SOURCES = mb_pslibface.c
libmbps_la_LIBADD = ${libnetcdf_LIBS} ${libgmt_LIBS} -lpsl -lgmtps
#Need -lgmtps in linux but remove for WIN32?

libmbaux_la_LDFLAGS = -no-undefined -version-info 0:0:0
libmbaux_la_SOURCES = \
		      mb_spline.c mb_zgrid.c mb_surface.c mb_cheb.c \
		      mb_delaun.c mb_track.c mb_truecont.c mb_readgrd.c
libmbaux_la_CFLAGS = -L../mbio
libmbaux_la_LIBADD = ${libnetcdf_LIBS} ${libgmt_LIBS} -lpsl libmbps.la -lmbio

if BUILD_MOTIF
 libmbxgr_la_LDFLAGS = -no-undefined -version-info 0:0:0
 libmbxgr_la_SOURCES = mb_xgraphics.c
endif




