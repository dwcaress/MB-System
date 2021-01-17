if BUILD_PCLTOOLS
  MBNAVADJUSTFINE = mbnavadjustfine
endif

if BUILD_MBTRN
  MBTRNINCDIR = -I${top_srcdir}/src/mbtrn/r7kr -I${top_srcdir}/src/mbtrn/utils -I${top_srcdir}/src/mbtrn/mframe/src
  MBTRNLIB = ${top_builddir}/src/mbtrn/libr7kr.la
endif

bin_PROGRAMS = mbnavadjust mbnavadjustmerge $(MBNAVADJUSTFINE)

AM_CFLAGS =
AM_CFLAGS += ${libnetcdf_CFLAGS}
AM_CFLAGS += ${libgmt_CFLAGS}

AM_CPPFLAGS =
AM_CPPFLAGS += -I${top_srcdir}/src/mbio
AM_CPPFLAGS += -I${top_srcdir}/src/mbaux
AM_CPPFLAGS += -I${top_srcdir}/src/mbview
AM_CPPFLAGS += $(MBTRNINCDIR)
AM_CPPFLAGS += -I${top_srcdir}/src/gsf
AM_CPPFLAGS += ${libnetcdf_CPPFLAGS}
AM_CPPFLAGS += ${libmotif_CPPFLAGS}
AM_CPPFLAGS += ${libopengl_CPPFLAGS}

AM_LDFLAGS =
AM_LDFLAGS +=  ${libx11_LDFLAGS}
AM_LDFLAGS +=  ${libmotif_LDFLAGS}
AM_LDFLAGS +=  ${libopengl_LDFLAGS}

LIBS =
LIBS += ${top_builddir}/src/mbio/libmbio.la
LIBS += ${top_builddir}/src/mbaux/libmbaux.la
LIBS += ${top_builddir}/src/mbaux/libmbxgr.la
LIBS += ${top_builddir}/src/mbview/libmbview.la
LIBS += ${libgmt_LIBS}
LIBS += ${libnetcdf_LIBS}
LIBS += ${libproj_LIBS}
LIBS += $(MBTRNINCDIR)
LIBS += ${libopengl_LIBS}
LIBS += ${libmotif_LIBS}
LIBS += ${libx11_LIBS}
LIBS += $(LIBM)

mbnavadjust_SOURCES =
mbnavadjust_SOURCES += mbnavadjust.c
mbnavadjust_SOURCES += mbnavadjust.h
mbnavadjust_SOURCES += mbnavadjust_callbacks.c
mbnavadjust_SOURCES += mbnavadjust_creation.c
mbnavadjust_SOURCES += mbnavadjust_creation.h
mbnavadjust_SOURCES += mbnavadjust_extrawidgets.h
mbnavadjust_SOURCES += mbnavadjust_io.c
mbnavadjust_SOURCES += mbnavadjust_io.h
mbnavadjust_SOURCES += mbnavadjust_prog.c
mbnavadjust_SOURCES += mbnavadjust_util.c

mbnavadjustmerge_SOURCES = mbnavadjustmerge.c mbnavadjust_io.c mbnavadjust_io.h

mbnavadjustfine_SOURCES =
mbnavadjustfine_SOURCES += mbnavadjust_fine.cc
mbnavadjustfine_SOURCES += mbnavadjust_icp.cc
mbnavadjustfine_SOURCES += mbnavadjust_icp.h
mbnavadjustfine_SOURCES += mbnavadjust_io.c
mbnavadjustfine_SOURCES += mbnavadjust_io.h
