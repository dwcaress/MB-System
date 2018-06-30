bin_PROGRAMS = mbnavedit

if BUILD_MBTRN
  MBTRNINCDIR = -I${top_srcdir}/src/mbtrn
  MBTRNLIB = ${top_builddir}/src/mbtrn/libmbtrn.la
endif

AM_CFLAGS = ${libnetcdf_CFLAGS}

AM_CPPFLAGS = -I${top_srcdir}/src/mbio \
        -I${top_srcdir}/src/mbaux \
        -I${top_srcdir}/src/gsf \
        $(MBTRNINCDIR) \
        ${libmotif_CPPFLAGS}

AM_LDFLAGS = 

LIBS = ${top_builddir}/src/mbio/libmbio.la \
        ${top_builddir}/src/mbaux/libmbaux.la \
        ${top_builddir}/src/mbaux/libmbxgr.la \
        ${libgmt_LIBS} ${libnetcdf_LIBS} ${libproj_LIBS} \
        ${libmotif_LIBS} $(MBTRNLIB) \
        $(LIBM)

mbnavedit_SOURCES = mbnavedit.c mbnavedit_creation.c mbnavedit_util.c mbnavedit_callbacks.c \
	mbnavedit_prog.c mbnavedit_creation.h mbnavedit_extrawidgets.h mbnavedit.h


