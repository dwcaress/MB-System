if BUILD_MOTIF
  XGRLIB = libmbxgr.la
endif
lib_LTLIBRARIES = libmbaux.la $(XGRLIB)

if BUILD_MOTIF
  XGRINC = mb_xgraphics.h
endif
include_HEADERS = mb_aux.h $(XGRINC)

AM_CPPFLAGS = -I${top_srcdir}/src/mbio \
	      ${libgmt_CPPFLAGS} \
	      ${libnetcdf_CPPFLAGS}
AM_CFLAGS = ${libgmt_CFLAGS} ${libnetcdf_CFLAGS}

libmbaux_la_LDFLAGS = -no-undefined -version-info 0:0:0
libmbaux_la_SOURCES = \
		      mb_spline.c mb_zgrid.c mb_surface.c mb_cheb.c \
		      mb_delaun.c mb_track.c mb_truecont.c mb_readwritegrd.c \
		      mb_intersectgrid.c
libmbaux_la_LIBADD =  ${top_builddir}/src/mbio/libmbio.la \
			${libgmt_LIBS} ${libnetcdf_LIBS}

if BUILD_MOTIF
 libmbxgr_la_LDFLAGS = -no-undefined -version-info 0:0:0
 libmbxgr_la_SOURCES = mb_xgraphics.c
 libmbxgr_la_LIBADD = ${libmotif_LIBS}
endif
