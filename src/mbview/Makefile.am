lib_LTLIBRARIES = libmbview.la

if BUILD_MBTRN
  MBTRNINCDIR = -I${top_srcdir}/src/mbtrn/r7kr -I${top_srcdir}/src/mbtrn/utils -I${top_srcdir}/src/mbtrn/mframe/src
  MBTRNLIB = ${top_builddir}/src/mbtrn/libr7kr.la
endif

include_HEADERS = mbview.h

AM_CFLAGS =
AM_CFLAGS += ${libnetcdf_CFLAGS}
AM_CFLAGS += ${libgmt_CFLAGS}

AM_CPPFLAGS =
AM_CPPFLAGS += -I${top_srcdir}/src/mbio
AM_CPPFLAGS += -I${top_srcdir}/src/mbaux
AM_CPPFLAGS += $(MBTRNINCDIR)
AM_CPPFLAGS += ${libgmt_CPPFLAGS}
AM_CPPFLAGS += ${libnetcdf_CPPFLAGS}
AM_CPPFLAGS += ${libx11_CPPFLAGS}
AM_CPPFLAGS += ${libmotif_CPPFLAGS}
AM_CPPFLAGS += ${libopengl_CPPFLAGS}

libmbview_la_CFLAGS = -DMBVIEW_LIBRARY

libmbview_la_SOURCES =
libmbview_la_SOURCES += MB3DView.c
libmbview_la_SOURCES += MB3DSiteList.c
libmbview_la_SOURCES += MB3DRouteList.c
libmbview_la_SOURCES += MB3DNavList.c
libmbview_la_SOURCES += mbview_callbacks.c
libmbview_la_SOURCES += mbview_process.c
libmbview_la_SOURCES += mbview_plot.c
libmbview_la_SOURCES += mbview_primary.c
libmbview_la_SOURCES += mbview_secondary.c
libmbview_la_SOURCES += mbview_pick.c
libmbview_la_SOURCES += mbview_profile.c
libmbview_la_SOURCES += mbview_site.c
libmbview_la_SOURCES += mbview_route.c
libmbview_la_SOURCES += mbview_nav.c
libmbview_la_SOURCES += mbview_vector.c
libmbview_la_SOURCES += mbview_bxutils.c
libmbview_la_SOURCES += mb_glwdrawa.c
libmbview_la_SOURCES += Mb3dsdg.c
libmbview_la_SOURCES += mb3dsoundings_callbacks.c
libmbview_la_SOURCES += MBpedit.c
libmbview_la_SOURCES += mbpingedit_callbacks.c

libmbview_la_LIBADD =
libmbview_la_LIBADD += ${top_builddir}/src/bsio/libmbbsio.la
libmbview_la_LIBADD += ${top_builddir}/src/surf/libmbsapi.la
libmbview_la_LIBADD += ${top_builddir}/src/mbio/libmbio.la
libmbview_la_LIBADD += ${top_builddir}/src/mbaux/libmbaux.la
libmbview_la_LIBADD += ${libgmt_LIBS}
libmbview_la_LIBADD += ${libnetcdf_LIBS}
libmbview_la_LIBADD += ${libproj_LIBS}
libmbview_la_LIBADD += ${MBTRNLIB}
libmbview_la_LIBADD += ${libopengl_LIBS}
libmbview_la_LIBADD += ${libmotif_LIBS}
libmbview_la_LIBADD += ${libx11_LIBS}
libmbview_la_LIBADD += ${XDR_LIB}

libmbview_la_LDFLAGS =
libmbview_la_LDFLAGS += -no-undefined -version-info 0:0:0
libmbview_la_LDFLAGS += ${libx11_LDFLAGS}
libmbview_la_LDFLAGS += ${libmotif_LDFLAGS}
libmbview_la_LDFLAGS += ${libopengl_LDFLAGS}
