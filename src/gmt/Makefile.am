lib_LTLIBRARIES = mbsystem.la

include_HEADERS =

AM_CPPFLAGS =
AM_CPPFLAGS += -I${top_srcdir}/src/mbio
AM_CPPFLAGS += -I${top_srcdir}/src/mbaux
AM_CPPFLAGS += ${libgmt_CPPFLAGS}
AM_CPPFLAGS += ${libgdal_CPPFLAGS}
AM_CPPFLAGS += ${libnetcdf_CPPFLAGS}

AM_CFLAGS = ${libgmt_CFLAGS} ${libgdal_CFLAGS} ${libnetcdf_CFLAGS}

mbsystem_la_LDFLAGS = -module -avoid-version -shared
mbsystem_la_SOURCES =
mbsystem_la_SOURCES += mbcontour.c
mbsystem_la_SOURCES += mbgrdtiff.c
mbsystem_la_SOURCES += mbswath.c

mbsystem_la_LIBADD =
mbsystem_la_LIBADD += ${top_builddir}/src/mbio/libmbio.la
mbsystem_la_LIBADD += ${top_builddir}/src/mbaux/libmbaux.la
mbsystem_la_LIBADD += ${libgmt_LIBS}
mbsystem_la_LIBADD += ${libgdal_LIBS}
mbsystem_la_LIBADD += ${libnetcdf_LIBS}
