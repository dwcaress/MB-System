if BUILD_MOTIF
  XGRLIB = libmbxgr.la
  XGRINC = mb_xgraphics.h
endif
if BUILD_MBTRN
  MBTRNINCDIR = -I${top_srcdir}/src/mbtrn/r7kr -I${top_srcdir}/src/mbtrn/utils -I${top_srcdir}/src/mbtrn/mframe/src
  MBTRNLIB = $(top_builddir)/src/mbtrn/libr7kr.la
endif

lib_LTLIBRARIES = libmbaux.la ${XGRLIB}
include_HEADERS = mb_aux.h ${XGRINC}

AM_CPPFLAGS =
AM_CPPFLAGS += -I${top_srcdir}/src/mbio
AM_CPPFLAGS += $(MBTRNINCDIR)
AM_CPPFLAGS += ${libgmt_CPPFLAGS}
AM_CPPFLAGS += ${libgdal_CPPFLAGS}
AM_CPPFLAGS += ${libnetcdf_CPPFLAGS}

AM_CFLAGS =
AM_CFLAGS += ${libgmt_CFLAGS}
AM_CFLAGS += ${libgdal_CFLAGS}
AM_CFLAGS += ${libnetcdf_CFLAGS}

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
  libmbxgr_la_LDFLAGS = -no-undefined -version-info 0:0:0
  libmbxgr_la_SOURCES = mb_xgraphics.c
  libmbxgr_la_LIBADD = ${libmotif_LIBS}
endif
