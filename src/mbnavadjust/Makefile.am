bin_PROGRAMS = mbnavadjust

AM_CFLAGS = ${libnetcdf_CFLAGS}

AM_CPPFLAGS = -I${top_srcdir}/src/mbio \
	      -I${top_srcdir}/src/mbaux \
	      ${libnetcdf_CPPFLAGS} \
	      ${motif_CPPFLAGS}
if BUILD_GSF
  AM_CPPFLAGS += -I${top_srcdir}/src/gsf
endif

AM_LDFLAGS = 

LIBS = ${top_builddir}/src/mbio/libmbio.la \
	${top_builddir}/src/mbaux/libmbaux.la \
	${top_builddir}/src/mbaux/libmbxgr.la \
	${libgmt_LIBS} ${libnetcdf_LIBS} \
	${motif_LIBS} \
	$(LIBM)

mbnavadjust_SOURCES = mbnavadjust.c mbnavadjust_callbacks.c mbnavadjust_creation.c \
	mbnavadjust_prog.c mbnavadjust_util.c mbnavadjust_creation.h \
	mbnavadjust_extrawidgets.h mbnavadjust.h
