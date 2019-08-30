bin_PROGRAMS = mbedit

AM_CFLAGS = ${libnetcdf_CFLAGS}

AM_CPPFLAGS =
AM_CPPFLAGS += -I${top_srcdir}/src/mbio
AM_CPPFLAGS += -I${top_srcdir}/src/mbaux
AM_CPPFLAGS += -I${top_srcdir}/src/gsf
AM_CPPFLAGS += ${libnetcdf_CPPFLAGS}
AM_CPPFLAGS += ${libmotif_CPPFLAGS}

AM_LDFLAGS = -L$(top_builddir)/src/gsf

LIBS =
LIBS += $(top_builddir)/src/mbio/libmbio.la
LIBS += $(top_builddir)/src/mbaux/libmbaux.la
LIBS += $(top_builddir)/src/mbaux/libmbxgr.la
LIBS += ${libgmt_LIBS}
LIBS += ${libnetcdf_LIBS}
LIBS += ${libproj_LIBS}
LIBS += ${libmotif_LIBS}
LIBS += $(LIBM)

mbedit_SOURCES =
mbedit_SOURCES += mbedit.c
mbedit_SOURCES += mbedit_bxutils.c
mbedit_SOURCES += mbedit_callbacks.c
mbedit_SOURCES += mbedit_creation.c
mbedit_SOURCES += mbedit_prog.c
mbedit_SOURCES += mbedit_creation.h
mbedit_SOURCES += mbedit.h
