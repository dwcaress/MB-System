bin_PROGRAMS = mbgrdviz

if BUILD_MBTRN
  MBTRNINCDIR = -I${top_srcdir}/src/mbtrn
endif

AM_CFLAGS = ${libgmt_CFLAGS} ${libnetcdf_CFLAGS}

AM_CPPFLAGS = -I${top_srcdir}/src/mbio \
        -I${top_srcdir}/src/mbaux \
        -I${top_srcdir}/src/mbview \
        $(MBTRNINCDIR) \
        ${libgmt_CPPFLAGS} ${libnetcdf_CPPFLAGS} \
        ${libmotif_CPPFLAGS} ${libopengl_CPPFLAGS}

AM_LDFLAGS =

mbgrdviz_SOURCES =  mbgrdviz_main.c mbgrdviz_callbacks.c  mbgrdviz_creation.c
mbgrdviz_LDADD = ${top_builddir}/src/mbio/libmbio.la \
		${top_builddir}/src/mbaux/libmbaux.la \
		${top_builddir}/src/mbaux/libmbxgr.la \
        ${top_builddir}/src/mbview/libmbview.la \
		${libgmt_LIBS} ${libnetcdf_LIBS} ${libproj_LIBS} \
		${libmotif_LIBS} ${libopengl_LIBS} \
		$(LIBM)

