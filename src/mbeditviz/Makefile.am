bin_PROGRAMS = mbeditviz

if BUILD_MBTRN
  MBTRNINCDIR = -I${top_srcdir}/src/mbtrn/r7kr -I${top_srcdir}/src/mbtrn/utils -I${top_srcdir}/src/mbtrn/mframe/src
  MBTRNLIB = ${top_builddir}/src/mbtrn/libr7kr.la
endif

AM_CFLAGS =
AM_CFLAGS += ${libnetcdf_CFLAGS}
AM_CFLAGS += ${libgmt_CFLAGS}

AM_CPPFLAGS =
AM_CPPFLAGS += -I${top_srcdir}/src/mbio
AM_CPPFLAGS += -I${top_srcdir}/src/mbaux
AM_CPPFLAGS += -I${top_srcdir}/src/mbview
AM_CPPFLAGS += $(MBTRNINCDIR)
AM_CPPFLAGS += ${libgmt_CPPFLAGS}
AM_CPPFLAGS += ${libnetcdf_CPPFLAGS}
AM_CPPFLAGS += ${libmotif_CPPFLAGS}
AM_CPPFLAGS += ${libopengl_CPPFLAGS}

AM_LDFLAGS =
AM_LDFLAGS +=  ${libx11_LDFLAGS}
AM_LDFLAGS +=  ${libmotif_LDFLAGS}
AM_LDFLAGS +=  ${libopengl_LDFLAGS}

mbeditviz_SOURCES = mbeditviz_main.c mbeditviz_callbacks.c mbeditviz_prog.c mbeditviz_creation.c
mbeditviz_LDADD =
mbeditviz_LDADD += ${top_builddir}/src/mbio/libmbio.la
mbeditviz_LDADD += ${top_builddir}/src/mbaux/libmbaux.la
mbeditviz_LDADD += ${top_builddir}/src/mbaux/libmbxgr.la
mbeditviz_LDADD += ${top_builddir}/src/mbview/libmbview.la
mbeditviz_LDADD += ${libgmt_LIBS}
mbeditviz_LDADD += ${libnetcdf_LIBS}
mbeditviz_LDADD += ${libproj_LIBS}
mbeditviz_LDADD += ${libx11_LIBS}
mbeditviz_LDADD += ${libmotif_LIBS}
mbeditviz_LDADD += ${libopengl_LIBS}
mbeditviz_LDADD += $(MBTRNLIB)
mbeditviz_LDADD += $(LIBM)
