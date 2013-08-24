lib_LTLIBRARIES = libmbview.la

include_HEADERS = mbview.h

AM_CFLAGS = ${libnetcdf_CFLAGS} ${libgmt_CFLAGS}

AM_CPPFLAGS = -I@top_srcdir@/src/mbio \
       -I@top_srcdir@/src/mbaux \
       -I@top_srcdir@/src/proj \
       -I@top_srcdir@/src/mr1pr \
       -I@top_srcdir@/src/surf \
       @CDF_INC_DIR@ @MOTIF_INC_DIR@ \
       @GL_INC_DIR@ @GMT_INC_DIR@
if BUILD_GSF
  AM_CPPFLAGS += -I@top_srcdir@/src/gsf
endif

libmbview_la_CFLAGS = -DMBVIEW_LIBRARY
libmbview_la_LDFLAGS = -no-undefined -version-info 0:0:0
libmbview_la_SOURCES = \
       MB3DView.c MB3DSiteList.c MB3DRouteList.c MB3DNavList.c \
       mbview_callbacks.c mbview_process.c mbview_plot.c mbview_primary.c \
       mbview_secondary.c mbview_pick.c mbview_profile.c mbview_site.c \
       mbview_route.c mbview_nav.c mbview_vector.c mbview_bxutils.c mb_glwdrawa.c Mb3dsdg.c \
       mb3dsoundings_callbacks.c MBpedit.c mbpingedit_callbacks.c
libmbview_la_LIBADD = ${libnetcdf_LIBS} ${libgmt_LIBS} @XDR_LIB@ \
       $(top_builddir)/src/mr1pr/libmbmr1pr.la \
       $(top_builddir)/src/surf/libmbsapi.la \
       $(top_builddir)/src/proj/libmbproj.la \
       $(top_builddir)/src/mbio/libmbio.la \
       $(top_builddir)/src/mbaux/libmbaux.la \
       @MOTIF_LIB@ @GL_LIB@ \
       ${libnetcdf_LIBS} ${libgmt_LIBS}
