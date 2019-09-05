if BUILD_MOTIF
  XGRLIB = libmbxgr.la
  XGRINC = mb_xgraphics.h
endif

if BUILD_MBTRN
  MBTRNINCDIR = -I${top_srcdir}/src/mbtrn -I${top_srcdir}/src/mbtrn/mframe/src
  MBTRNLIB = $(top_builddir)/src/mbtrn/libr7kr.la
endif

lib_LTLIBRARIES = libmbaux.la $(XGRLIB)
include_HEADERS = mb_aux.h $(XGRINC)

AM_CPPFLAGS = -I${top_srcdir}/src/mbio \
	      $(MBTRNINCDIR) \
	      ${libgmt_CPPFLAGS} \
	      ${libgdal_CPPFLAGS} \
	      ${libnetcdf_CPPFLAGS}
AM_CFLAGS = ${libgmt_CFLAGS} ${libgdal_CFLAGS} ${libnetcdf_CFLAGS}

libmbaux_la_LDFLAGS = -no-undefined -version-info 0:0:0
libmbaux_la_SOURCES = \
		      mb_zgrid.c mb_surface.c mb_cheb.c \
		      mb_delaun.c mb_track.c mb_truecont.c mb_readwritegrd.c \
		      mb_intersectgrid.c
libmbaux_la_LIBADD =  ${top_builddir}/src/mbio/libmbio.la \
          $(MBTRNLIB) \
			    ${libgmt_LIBS} \
			    ${libgdal_LIBS} \
			    ${libnetcdf_LIBS}

if BUILD_MOTIF
 libmbxgr_la_LDFLAGS = -no-undefined -version-info 0:0:0
 libmbxgr_la_SOURCES = mb_xgraphics.c
 libmbxgr_la_LIBADD = ${libmotif_LIBS}
endif
