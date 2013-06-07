AM_CPPFLAGS = -I@top_srcdir@/src/mbio @CDF_INC_DIR@ @GMT_INC_DIR@
AM_CFLAGS = ${libnetcdf_CFLAGS} ${libgmt_CFLAGS}

if BUILD_MOTIF
  XGRINC = mb_xgraphics.h
endif
include_HEADERS = mb_aux.h $(XGRINC)

if BUILD_MOTIF
  XGRLIB = libmbxgr.la
endif
lib_LTLIBRARIES = libmbps.la libmbaux.la $(XGRLIB)

libmbps_la_LDFLAGS = -no-undefined -version-info 0:0:0 ${libgmt_CFLAGS}
libmbps_la_SOURCES = mb_pslibface.c
libmbps_la_LIBADD = ${libnetcdf_LIBS} ${libgmt_LIBS}
#Need -lgmtps in linux but remove for WIN32?

libmbaux_la_LDFLAGS = -no-undefined -version-info 0:0:0 ${libgmt_CFLAGS} -L$(top_builddir)/src/mbio
libmbaux_la_SOURCES = \
		      mb_spline.c mb_zgrid.c mb_surface.c mb_cheb.c \
		      mb_delaun.c mb_track.c mb_truecont.c mb_readgrd.c \
		      mb_intersectgrid.c
libmbaux_la_CFLAGS = ${libnetcdf_CFLAGS} ${libgmt_CFLAGS}
libmbaux_la_LIBADD = ${libnetcdf_LIBS} ${libgmt_LIBS} -lmbio libmbps.la

if BUILD_MOTIF
 libmbxgr_la_LDFLAGS = -no-undefined -version-info 0:0:0 ${libgmt_CFLAGS}
 libmbxgr_la_SOURCES = mb_xgraphics.c
 libmbxgr_la_LIBADD = @MOTIF_LIB_DIR@ @MOTIF_LIB@
endif
