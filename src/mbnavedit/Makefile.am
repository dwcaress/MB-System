bin_PROGRAMS = mbnavedit

AM_CFLAGS = ${libnetcdf_CFLAGS}

AM_CPPFLAGS =
AM_CPPFLAGS += -I${top_srcdir}/src/mbio
AM_CPPFLAGS += -I${top_srcdir}/src/mbaux
AM_CPPFLAGS += -I${top_srcdir}/src/gsf
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
LIBS += $(LIBM)

mbnavedit_SOURCES =
mbnavedit_SOURCES += mbnavedit.c
mbnavedit_SOURCES += mbnavedit.h
mbnavedit_SOURCES += mbnavedit_callbacks.c
mbnavedit_SOURCES += mbnavedit_creation.c
mbnavedit_SOURCES += mbnavedit_creation.h
mbnavedit_SOURCES += mbnavedit_extrawidgets.h
mbnavedit_SOURCES += mbnavedit_prog.c
mbnavedit_SOURCES += mbnavedit_util.c


