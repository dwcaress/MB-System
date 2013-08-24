bin_PROGRAMS = mbgrdviz

AM_CFLAGS = ${libnetcdf_CFLAGS} ${libgmt_CFLAGS}

AM_CPPFLAGS = -I@top_srcdir@/src/mbio \
       -I@top_srcdir@/src/mbaux \
       -I@top_srcdir@/src/proj \
       -I@top_srcdir@/src/mr1pr \
       -I@top_srcdir@/src/surf \
       @CDF_INC_DIR@ @MOTIF_INC_DIR@ @GL_INC_DIR@ @GMT_INC_DIR@
if BUILD_GSF
  AM_CPPFLAGS += -I@top_srcdir@/src/gsf
endif

AM_LDFLAGS = -L$(top_builddir)/src/mbio \
       -L$(top_builddir)/src/mbaux \
       -L$(top_builddir)/src/mr1pr \
       -L$(top_builddir)/src/proj \
       -L$(top_builddir)/src/surf
if BUILD_GSF
  AM_LDFLAGS +=  -L$(top_builddir)/src/gsf
endif

mbgrdviz_SOURCES =  mbgrdviz_main.c mbgrdviz_callbacks.c  mbgrdviz_creation.c
mbgrdviz_LDADD = ${libnetcdf_LIBS} ${libgmt_LIBS} @MOTIF_LIB_DIR@ \
       $(top_builddir)/src/mbio/libmbio.la $(top_builddir)/src/mbaux/libmbaux.la \
       $(top_builddir)/src/mbaux/libmbxgr.la $(top_builddir)/src/mbaux/libmbps.la \
        $(top_builddir)/src/mbview/libmbview.la \
       @MOTIF_LIB@ @GL_LIB@ \
       ${libnetcdf_LIBS} ${libgmt_LIBS} $(LIBM)
