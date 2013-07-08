bin_PROGRAMS = mbgrdviz mbeditviz

AM_CFLAGS = ${libnetcdf_CFLAGS} ${libgmt_CFLAGS}

AM_CPPFLAGS = -I@top_srcdir@/src/mbio -I@top_srcdir@/src/mbaux @MOTIF_INC_DIR@ @GL_INC_DIR@ @GMT_INC_DIR@

LIBS = ${libnetcdf_LIBS} ${libgmt_LIBS} @MOTIF_LIB_DIR@ \
       $(top_builddir)/src/mbio/libmbio.la $(top_builddir)/src/mbaux/libmbaux.la \
       $(top_builddir)/src/mbaux/libmbxgr.la @MOTIF_LIB@ @GL_LIB@ \
       -lXm -lXt -lX11 $(LIBM)

include_HEADERS = mbview.h

mbgrdviz_SOURCES =  mbgrdviz_main.c mbgrdviz_callbacks.c  mbgrdviz_creation.c
mbgrdviz_LDADD = libmbview.la
#mbgrdviz_LDADD =  -lmbaux -lmbxgr @MOTIF_LIB@ @GL_LIB@

mbeditviz_SOURCES = mbeditviz_main.c mbeditviz_callbacks.c mbeditviz_prog.c mbeditviz_creation.c
#mbeditviz_LDADD = -lmbaux -lmbxgr @MOTIF_LIB@ @GL_LIB@ libmbview.la
mbeditviz_CFLAGS=-DMBEDITVIZ_DECLARE_GLOBALS
mbeditviz_LDADD = libmbview.la


lib_LTLIBRARIES = libmbview.la

libmbview_la_LDFLAGS = -no-undefined -version-info 0:0:0 -lm
libmbview_la_CFLAGS=-DMBVIEW_LIBRARY
# -L@top_srcdir@/src/mbio

libmbview_la_SOURCES = \
	MB3DView.c MB3DSiteList.c MB3DRouteList.c MB3DNavList.c \
	mbview_callbacks.c mbview_process.c mbview_plot.c mbview_primary.c \
	mbview_secondary.c mbview_pick.c mbview_profile.c mbview_site.c \
	mbview_route.c mbview_nav.c mbview_vector.c mbview_bxutils.c mb_glwdrawa.c Mb3dsdg.c \
	mb3dsoundings_callbacks.c MBpedit.c mbpingedit_callbacks.c

# libmbview_la_LIBADD = @MOTIF_LIB_DIR@ @GL_LIB_DIR@ @MOTIF_LIB@ @GL_LIB@ -lmbio
