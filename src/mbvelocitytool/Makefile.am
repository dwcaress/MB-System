bin_PROGRAMS = mbvelocitytool

if BUILD_MBTRN
  MBTRNINCDIR = -I${top_srcdir}/src/mbtrn/r7kr -I${top_srcdir}/src/mbtrn/utils -I${top_srcdir}/src/mbtrn/mframe/src
  MBTRNLIB = ${top_builddir}/src/mbtrn/libr7kr.la
endif

AM_CFLAGS = ${libnetcdf_CFLAGS}

AM_CPPFLAGS =
AM_CPPFLAGS += -I${top_srcdir}/src/mbio
AM_CPPFLAGS += -I${top_srcdir}/src/mbaux
AM_CPPFLAGS += -I${top_srcdir}/src/gsf
AM_CPPFLAGS += $(MBTRNINCDIR)
AM_CPPFLAGS += ${libnetcdf_CPPFLAGS}
AM_CPPFLAGS += ${libmotif_CPPFLAGS}

AM_LDFLAGS =

LIBS = 
LIBS += ${top_builddir}/src/mbio/libmbio.la
LIBS += ${top_builddir}/src/mbaux/libmbaux.la
LIBS += ${top_builddir}/src/mbaux/libmbxgr.la
LIBS += ${libgmt_LIBS}
LIBS += ${libnetcdf_LIBS}
LIBS += ${libproj_LIBS}
LIBS += ${libmotif_LIBS}
LIBS += $(MBTRNLIB)
LIBS += $(LIBM)

mbvelocitytool_SOURCES = mbvelocity_bxutils.c mbvelocity.c mbvelocity_callbacks.c \
        mbvelocity_creation.c mbvelocity_prog.c mbvelocity_creation.h mbvelocity.h
