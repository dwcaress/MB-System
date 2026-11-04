bin_PROGRAMS = mbsvpbest

AM_CFLAGS = ${libnetcdf_CFLAGS}

AM_CPPFLAGS =
AM_CPPFLAGS += -I${top_srcdir}/src/mbio
AM_CPPFLAGS += ${libnetcdf_CPPFLAGS}

AM_LDFLAGS =

LIBS =
LIBS += ${top_builddir}/src/mbio/libmbio.la
LIBS += ${libnetcdf_LIBS}
LIBS += $(LIBM)

noinst_HEADERS = mbsvp_tools.h

mbsvpbest_SOURCES = mbsvpbest.c
mbsvpbest_SOURCES += mbsvp_location.c
mbsvpbest_SOURCES += mbsvp_profile.c
mbsvpbest_SOURCES += mbsvp_score.c
mbsvpbest_SOURCES += mbsvp_util.c
mbsvpbest_SOURCES += mbsvp_woa.c
