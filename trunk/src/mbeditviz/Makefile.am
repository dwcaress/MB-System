bin_PROGRAMS = mbeditviz

AM_CFLAGS = ${libgmt_CFLAGS} ${libnetcdf_CFLAGS}

AM_CPPFLAGS = -I${top_srcdir}/src/mbio \
	-I${top_srcdir}/src/mbaux \
	-I${top_srcdir}/src/mbview \
	-I${top_srcdir}/src/mr1pr \
	-I${top_srcdir}/src/surf \
	-I${top_srcdir}/src/gsf \
	${libgmt_CPPFLAGS} \
	${libnetcdf_CPPFLAGS} \
	${motif_CPPFLAGS} ${GL_CPPFLAGS}

AM_LDFLAGS = 

mbeditviz_SOURCES = mbeditviz_main.c mbeditviz_callbacks.c mbeditviz_prog.c mbeditviz_creation.c
mbeditviz_LDADD = ${top_builddir}/src/mbio/libmbio.la \
		  ${top_builddir}/src/mbaux/libmbaux.la \
		  ${top_builddir}/src/mbaux/libmbxgr.la \
		  ${top_builddir}/src/mbview/libmbview.la \
		  ${libgmt_LIBS} ${libnetcdf_LIBS} ${libproj_LIBS} \
		  ${motif_LIBS} ${GL_LIBS} \
		  $(LIBM)
