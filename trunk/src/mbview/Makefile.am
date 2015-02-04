lib_LTLIBRARIES = libmbview.la

include_HEADERS = mbview.h

AM_CFLAGS = ${libgmt_CFLAGS} ${libnetcdf_CFLAGS}

AM_CPPFLAGS = -I${top_srcdir}/src/mbio \
       -I${top_srcdir}/src/mbaux \
       -I${top_srcdir}/src/mr1pr \
       -I${top_srcdir}/src/surf \
       -I${top_srcdir}/src/gsf \
       ${libgmt_CPPFLAGS} \
       ${libnetcdf_CPPFLAGS} \
       ${motif_CPPFLAGS} \
       ${GL_CPPFLAGS}

libmbview_la_CFLAGS = -DMBVIEW_LIBRARY
libmbview_la_LDFLAGS = -no-undefined -version-info 0:0:0
libmbview_la_SOURCES = \
       MB3DView.c MB3DSiteList.c MB3DRouteList.c MB3DNavList.c \
       mbview_callbacks.c mbview_process.c mbview_plot.c mbview_primary.c \
       mbview_secondary.c mbview_pick.c mbview_profile.c mbview_site.c \
       mbview_route.c mbview_nav.c mbview_vector.c mbview_bxutils.c mb_glwdrawa.c Mb3dsdg.c \
       mb3dsoundings_callbacks.c MBpedit.c mbpingedit_callbacks.c
libmbview_la_LIBADD = \
       ${top_builddir}/src/bsio/libmbbsio.la \
       ${top_builddir}/src/surf/libmbsapi.la \
       ${top_builddir}/src/mbio/libmbio.la \
       ${top_builddir}/src/mbaux/libmbaux.la \
       ${libgmt_LIBS} ${libnetcdf_LIBS} ${libproj_LIBS} \
       ${motif_LIBS} ${GL_LIBS} \
       ${XDR_LIB}
