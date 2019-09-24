
if BUILD_MBTNAV
TRNWLIBS = ${top_builddir}/src/mbtrnav/libtrnw.la ${top_builddir}/src/mbtrnav/libqnx.la \
${top_builddir}/src/mbtrnav/libnewmat.la  ${top_builddir}/src/mbtrnav/libtnav.la  \
${top_builddir}/src/mbtrnav/libgeolib.la
MBTNAV_INC=-I${top_builddir}/src/mbtrnav/terrain-nav -I${top_builddir}/src/mbtrnav/gctp -I${top_builddir}/src/mbtrnav/newmat -I${top_builddir}/src/mbtrnav/qnx-utils -I${top_builddir}/src/mbtrnav/trnw
MBTNAV_OPT=-DWITH_MBTNAV
endif

AM_CFLAGS = ${libgmt_CFLAGS} ${libnetcdf_CFLAGS}

# add -DMBTRN_VER="<version>" to override version defined in mbtrn.h
BUILD_DATE=`date +%Y/%m/%dT%H:%M:%S%z`

TRNC_VER='-DTRNC_VER=1.0'
FRAMES7K_VER='-DFRAMES7K_VER=1.0'
STREAM7K_VER='-DSTREAM7K_VER=1.0'
TBIN_VER='-DTBIN_VER=1.0'
EMU7K_VER='-DEMU_VER=1.0'
TEST_OPTS=-DWITH_TESTS -DWITH_R7KR_TEST
AM_CPPFLAGS = -g -O0 -I${top_srcdir}/src/mbtrn/r7kr -I${top_srcdir}/src/mbtrn/utils -I${top_srcdir}/src/mbtrn/mframe/src $(MBTNAV_INC)  -DHAVE_CONFIG_H  ${TEST_OPTS} -DWITH_MST_MSTATS -DMST_STATS_EN -DWITH_PDEBUG -DWITH_MMDEBUG -D_GNU_SOURCE -DMFRAME_BUILD="${BUILD_DATE}" ${FRAMES7K_VER} ${TRNC_VER} ${STREAM7K_VER} ${TBIN_VER} ${EMU7K_VER} ${MBTNAV_OPT}


bin_PROGRAMS =  mbtrnpreprocess

mbtrnpreprocess_SOURCES = mbtrnpreprocess.c
mbtrnpreprocess_LDADD =  $(top_builddir)/src/mbio/libmbio.la \
  $(top_builddir)/src/mbaux/libmbaux.la \
  $(top_builddir)/src/mbaux/libmbxgr.la \
  ${libgmt_LIBS} ${libnetcdf_LIBS} ${libproj_LIBS} \
  $(top_builddir)/src/mbtrn/libr7kr.la $(TRNWLIBS) $(LIBM)


CLEANFILES =
DISTCLEANFILES =
