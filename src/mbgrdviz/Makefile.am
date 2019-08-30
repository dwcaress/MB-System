bin_PROGRAMS = mbgrdviz

AM_CFLAGS = ${libgmt_CFLAGS} ${libnetcdf_CFLAGS}

AM_CPPFLAGS =
AM_CPPFLAGS += -I${top_srcdir}/src/mbio
AM_CPPFLAGS += -I${top_srcdir}/src/mbaux
AM_CPPFLAGS += -I${top_srcdir}/src/mbview
AM_CPPFLAGS += ${libgmt_CPPFLAGS} ${libnetcdf_CPPFLAGS}
AM_CPPFLAGS += ${libmotif_CPPFLAGS} ${libopengl_CPPFLAGS}

AM_LDFLAGS =

mbgrdviz_SOURCES = mbgrdviz_main.c mbgrdviz_callbacks.c mbgrdviz_creation.c
mbgrdviz_LDADD =
mbgrdviz_LDADD += ${top_builddir}/src/mbio/libmbio.la
mbgrdviz_LDADD += ${top_builddir}/src/mbaux/libmbaux.la
mbgrdviz_LDADD += ${top_builddir}/src/mbaux/libmbxgr.la
mbgrdviz_LDADD += ${top_builddir}/src/mbview/libmbview.la
mbgrdviz_LDADD += ${libgmt_LIBS}
mbgrdviz_LDADD += ${libnetcdf_LIBS}
mbgrdviz_LDADD += ${libproj_LIBS}
mbgrdviz_LDADD += ${libmotif_LIBS}
mbgrdviz_LDADD += ${libopengl_LIBS}
mbgrdviz_LDADD += $(LIBM)
