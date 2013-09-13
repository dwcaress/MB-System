if BUILD_MOTIF
  XGRLIB = libmbxgr.la
endif
lib_LTLIBRARIES = libmbps.la libmbaux.la $(XGRLIB)

if BUILD_MOTIF
  XGRINC = mb_xgraphics.h
endif
include_HEADERS = mb_aux.h $(XGRINC)

AM_CPPFLAGS = -I@top_srcdir@/src/mbio @CDF_INC_DIR@ @GMT_INC_DIR@
AM_CFLAGS = ${libnetcdf_CFLAGS} ${libgmt_CFLAGS}

libmbps_la_LDFLAGS = -no-undefined -version-info 0:0:0 ${libgmt_LDFLAGS} ${libnetcdf_LDFLAGS} ${libgmt_RPATH} ${libnetcdf_RPATH}
libmbps_la_SOURCES = mb_pslibface.c
libmbps_la_LIBADD = ${libgmt_LIBS} ${libnetcdf_LIBS} 

libmbaux_la_LDFLAGS = -no-undefined -version-info 0:0:0 -L$(top_builddir)/src/mbio ${libgmt_LDFLAGS} ${libnetcdf_LDFLAGS} ${libgmt_RPATH} ${libnetcdf_RPATH}
libmbaux_la_SOURCES = \
		      mb_spline.c mb_zgrid.c mb_surface.c mb_cheb.c \
		      mb_delaun.c mb_track.c mb_truecont.c mb_readgrd.c \
		      mb_intersectgrid.c
#libmbaux_la_CFLAGS = ${libnetcdf_CFLAGS} ${libgmt_CFLAGS}
libmbaux_la_LIBADD =  ${libgmt_LIBS} ${libnetcdf_LIBS} -lmbio libmbps.la

if BUILD_MOTIF
 libmbxgr_la_LDFLAGS = -no-undefined -version-info 0:0:0
 libmbxgr_la_SOURCES = mb_xgraphics.c
 libmbxgr_la_LIBADD = @MOTIF_LIB_DIR@ @MOTIF_LIB@
endif
