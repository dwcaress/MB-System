bin_PROGRAMS = mbphotomosaic mbgetphotocorrection mbphotogrammetry
bin_SCRIPTS = mbm_makeimagelist

include_HEADERS =

AM_CPPFLAGS =
AM_CPPFLAGS += -I${top_srcdir}/src/mbio
AM_CPPFLAGS += -I${top_srcdir}/src/mbaux
AM_CPPFLAGS += ${opencv4_CPPFLAGS}

AM_LDFLAGS =

LIBS =
LIBS += $(top_builddir)/src/mbio/libmbio.la
LIBS += $(top_builddir)/src/mbaux/libmbaux.la
LIBS += ${opencv4_LIBS}

mbphotomosaic_SOURCES = mbphotomosaic.cc
mbgetphotocorrection_SOURCES = mbgetphotocorrection.cc
mbphotogrammetry_SOURCES = mbphotogrammetry.cc
