bin_PROGRAMS = mbedit

if BUILD_MBTRN
  MBTRNINCDIR = -I${top_srcdir}/src/mbtrn/r7kr -I${top_srcdir}/src/mbtrn/utils -I${top_srcdir}/src/mbtrnframe/usr -I${top_srcdir}/src/mbtrnframe/mframe/src
  MBTRNLIB = ${top_builddir}/src/mbtrn/libr7kr.la
endif

AM_CFLAGS =
AM_CFLAGS += ${libnetcdf_CFLAGS}
AM_CFLAGS += ${libgmt_CFLAGS}

AM_CPPFLAGS =
AM_CPPFLAGS += -I${top_srcdir}/src/mbio
AM_CPPFLAGS += -I${top_srcdir}/src/mbaux
AM_CPPFLAGS += -I${top_srcdir}/src/gsf
AM_CPPFLAGS += $(MBTRNINCDIR)
AM_CPPFLAGS += ${libnetcdf_CPPFLAGS}
AM_CPPFLAGS += ${libmotif_CPPFLAGS}
AM_CPPFLAGS += ${libx11_CPPFLAGS}

AM_LDFLAGS =
AM_LDFLAGS +=  ${libmotif_LDFLAGS}
AM_LDFLAGS +=  ${libx11_LDFLAGS}

LIBS =
LIBS += $(top_builddir)/src/mbio/libmbio.la
LIBS += $(top_builddir)/src/mbaux/libmbaux.la
LIBS += $(top_builddir)/src/mbaux/libmbxgr.la
LIBS += ${libgmt_LIBS}
LIBS += ${libnetcdf_LIBS}
LIBS += ${libproj_LIBS}
LIBS += ${libmotif_LIBS}
LIBS += ${libx11_LIBS}
LIBS += $(MBTRNLIB)
LIBS += $(LIBM)

mbedit_SOURCES =
mbedit_SOURCES += mbedit.c
mbedit_SOURCES += mbedit_bxutils.c
mbedit_SOURCES += mbedit_callbacks.c
mbedit_SOURCES += mbedit_creation.c
mbedit_SOURCES += mbedit_prog.c
mbedit_SOURCES += mbedit_creation.h
mbedit_SOURCES += mbedit.h
