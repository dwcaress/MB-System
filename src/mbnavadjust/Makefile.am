# As in CMakeLists.txt, the mbnavadjust_core library and the Motif/X11-free
# mbnavadjustmerge program are always built; the mbnavadjust GUI (and
# mbnavadjustfine) only when the Motif/OpenGL graphical tools are built.
if BUILD_MOTIF
if BUILD_GL
  MBNAVADJUSTGUI = mbnavadjust
if BUILD_PCLTOOLS
  MBNAVADJUSTFINE = mbnavadjustfine
endif
endif
endif

lib_LTLIBRARIES = libmbnavadjust_core.la

include_HEADERS = mbnavadjust_io.h mbnavadjust_core.h

bin_PROGRAMS = mbnavadjustmerge $(MBNAVADJUSTGUI) $(MBNAVADJUSTFINE)

AM_CFLAGS =
AM_CFLAGS += ${libnetcdf_CFLAGS}
AM_CFLAGS += ${libgmt_CFLAGS}

AM_CPPFLAGS =
AM_CPPFLAGS += -I${top_srcdir}/src/mbio
AM_CPPFLAGS += -I${top_srcdir}/src/mbaux
AM_CPPFLAGS += -I${top_srcdir}/src/mbview
AM_CPPFLAGS += -I${top_srcdir}/src/gsf
AM_CPPFLAGS += ${libnetcdf_CPPFLAGS}
AM_CPPFLAGS += ${libopengl_CPPFLAGS}
AM_CPPFLAGS += ${libmotif_CPPFLAGS}
AM_CPPFLAGS += ${libx11_CPPFLAGS}

AM_LDFLAGS =
AM_LDFLAGS +=  ${libopengl_LDFLAGS}
AM_LDFLAGS +=  ${libmotif_LDFLAGS}
AM_LDFLAGS +=  ${libx11_LDFLAGS}

libmbnavadjust_core_la_LDFLAGS = -no-undefined -version-info 0:0:0
libmbnavadjust_core_la_SOURCES =
libmbnavadjust_core_la_SOURCES += mbnavadjust_io.c
libmbnavadjust_core_la_SOURCES += mbnavadjust_io.h
libmbnavadjust_core_la_SOURCES += mbnavadjust_autopick.c
libmbnavadjust_core_la_SOURCES += mbnavadjust_invertnav.c
libmbnavadjust_core_la_SOURCES += mbnavadjust_core.h
libmbnavadjust_core_la_LIBADD =
libmbnavadjust_core_la_LIBADD += ${top_builddir}/src/mbio/libmbio.la
libmbnavadjust_core_la_LIBADD += ${top_builddir}/src/mbaux/libmbaux.la
libmbnavadjust_core_la_LIBADD += $(LIBM)

mbnavadjustmerge_SOURCES = mbnavadjustmerge.c
mbnavadjustmerge_LDADD =
mbnavadjustmerge_LDADD += libmbnavadjust_core.la
mbnavadjustmerge_LDADD += ${top_builddir}/src/mbio/libmbio.la
mbnavadjustmerge_LDADD += ${top_builddir}/src/mbaux/libmbaux.la
mbnavadjustmerge_LDADD += $(LIBM)

mbnavadjust_SOURCES =
mbnavadjust_SOURCES += mbnavadjust.c
mbnavadjust_SOURCES += mbnavadjust.h
mbnavadjust_SOURCES += mbnavadjust_callbacks.c
mbnavadjust_SOURCES += mbnavadjust_creation.c
mbnavadjust_SOURCES += mbnavadjust_creation.h
mbnavadjust_SOURCES += mbnavadjust_extrawidgets.h
mbnavadjust_SOURCES += mbnavadjust_prog.c
mbnavadjust_SOURCES += mbnavadjust_util.c
mbnavadjust_LDADD =
mbnavadjust_LDADD += libmbnavadjust_core.la
mbnavadjust_LDADD += ${top_builddir}/src/mbio/libmbio.la
mbnavadjust_LDADD += ${top_builddir}/src/mbaux/libmbaux.la
mbnavadjust_LDADD += ${top_builddir}/src/mbaux/libmbxgr.la
mbnavadjust_LDADD += ${top_builddir}/src/mbaux/libmbxmutil.la
mbnavadjust_LDADD += ${top_builddir}/src/mbview/libmbview.la
mbnavadjust_LDADD += ${libgmt_LIBS}
mbnavadjust_LDADD += ${libnetcdf_LIBS}
mbnavadjust_LDADD += ${libproj_LIBS}
mbnavadjust_LDADD += ${libopengl_LIBS}
mbnavadjust_LDADD += ${libmotif_LIBS}
mbnavadjust_LDADD += ${libx11_LIBS}
mbnavadjust_LDADD += $(LIBM)

mbnavadjustfine_SOURCES =
mbnavadjustfine_SOURCES += mbnavadjust_fine.cc
mbnavadjustfine_SOURCES += mbnavadjust_icp.cc
mbnavadjustfine_SOURCES += mbnavadjust_icp.h
mbnavadjustfine_LDADD =
mbnavadjustfine_LDADD += libmbnavadjust_core.la
mbnavadjustfine_LDADD += ${top_builddir}/src/mbio/libmbio.la
mbnavadjustfine_LDADD += ${top_builddir}/src/mbaux/libmbaux.la
mbnavadjustfine_LDADD += ${libproj_LIBS}
mbnavadjustfine_LDADD += $(LIBM)
