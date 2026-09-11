lib_LTLIBRARIES = libmbaux.la ${XGRLIB} ${XMUTILLIB}
include_HEADERS = mb_aux.h ${XGRINC} ${XMUTILINC}

if BUILD_MOTIF
  XGRLIB = libmbxgr.la
  XGRINC = mb_xgraphics.h
  XMUTILLIB = libmbxmutil.la
  XMUTILINC = mb_xmutil.h
endif
if BUILD_MBTRN
  MBTRNLIB = $(top_builddir)/src/mbtrn/libr7kr.la
endif

AM_CFLAGS =
AM_CFLAGS += ${libgmt_CFLAGS}
AM_CFLAGS += ${libgdal_CFLAGS}
AM_CFLAGS += ${libnetcdf_CFLAGS}

AM_CPPFLAGS =
AM_CPPFLAGS += -I${top_srcdir}/src/mbio
AM_CPPFLAGS += ${libgmt_CPPFLAGS}
AM_CPPFLAGS += ${libgdal_CPPFLAGS}
AM_CPPFLAGS += ${libnetcdf_CPPFLAGS}
AM_CPPFLAGS += ${libx11_CPPFLAGS}

libmbaux_la_LDFLAGS = -no-undefined -version-info 0:0:0

libmbaux_la_SOURCES =
libmbaux_la_SOURCES += mb_cheb.c
libmbaux_la_SOURCES += mb_delaun.c
libmbaux_la_SOURCES += mb_intersectgrid.c
libmbaux_la_SOURCES += mb_readwritegrd.c
libmbaux_la_SOURCES += mb_surface.c
libmbaux_la_SOURCES += mb_track.c
libmbaux_la_SOURCES += mb_truecont.c
libmbaux_la_SOURCES += mb_zgrid.c

libmbaux_la_LIBADD =
libmbaux_la_LIBADD += ${top_builddir}/src/mbio/libmbio.la
libmbaux_la_LIBADD += $(MBTRNLIB)
libmbaux_la_LIBADD += ${libgmt_LIBS}
libmbaux_la_LIBADD += ${libgdal_LIBS}
libmbaux_la_LIBADD += ${libnetcdf_LIBS}

if BUILD_MOTIF
  libmbxgr_la_CPPFLAGS = ${libx11_CPPFLAGS}
  libmbxgr_la_LDFLAGS = -no-undefined -version-info 0:0:0 ${libx11_LDFLAGS}
  libmbxgr_la_SOURCES = mb_xgraphics.c
  libmbxgr_la_LIBADD = ${libx11_LIBS}

  libmbxmutil_la_CPPFLAGS = ${libx11_CPPFLAGS} ${libmotif_CPPFLAGS}
  libmbxmutil_la_LDFLAGS = -no-undefined -version-info 0:0:0 ${libx11_LDFLAGS} ${libmotif_LDFLAGS}
  libmbxmutil_la_SOURCES = mb_xmutil.c
  libmbxmutil_la_LIBADD = ${libx11_LIBS} ${libmotif_LIBS}
endif
