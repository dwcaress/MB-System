
if BUILD_MBTNAV

TRNWLIBS = ${top_builddir}/src/mbtrnav/libtrnw.la ${top_builddir}/src/mbtrnav/libqnx.la \
${top_builddir}/src/mbtrnav/libnewmat.la  ${top_builddir}/src/mbtrnav/libtnav.la  \
${top_builddir}/src/mbtrnav/libgeolib.la ${top_builddir}/src/mbtrnav/libnetif.la

MBTRNLIBS = \
$(top_builddir)/src/mbtrn/libr7kr.la \
$(top_builddir)/src/mbtrn/libmb1r.la

MBTNAV_INC = -I${top_builddir}/src/mbtrnav/terrain-nav
MBTNAV_INC += -I${top_builddir}/src/mbtrnav/gctp
MBTNAV_INC += -I${top_builddir}/src/mbtrnav/newmat
MBTNAV_INC += -I${top_builddir}/src/mbtrnav/qnx-utils
MBTNAV_INC += -I${top_builddir}/src/mbtrnav/trnw
MBTNAV_INC += -I${top_builddir}/src/mbtrnav/utils
MBTNAV_INC += -I${top_builddir}/src/mbtrn/mb1r

MBTRN_INC = -I${top_srcdir}/src/mbtrn/r7kr -I${top_srcdir}/src/mbtrn/utils  -I${top_srcdir}/src/mbtrnframe/usr -I${top_srcdir}/src/mbtrnframe/mframe/src

MBTNAV_OPT=-DWITH_MBTNAV
endif

if BUILD_MOTIF
LIBMBXGR = $(top_builddir)/src/mbaux/libmbxgr.la
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
AM_CPPFLAGS = -g -O0 $(MBTRN_INC) $(MBTNAV_INC) \
 	-DHAVE_CONFIG_H  ${TEST_OPTS} -DWITH_MST_MSTATS -DMST_STATS_EN \
	-DWITH_PDEBUG -DWITH_MMDEBUG -D_GNU_SOURCE \
	-DMFRAME_BUILD="${BUILD_DATE}" \
	${libgmt_CPPFLAGS} ${libnetcdf_CPPFLAGS} \
	${FRAMES7K_VER} ${TRNC_VER} ${STREAM7K_VER} \
	${TBIN_VER} ${EMU7K_VER} ${MBTNAV_OPT}

bin_PROGRAMS =  mbtrnpp mbgrd2octree

mbtrnpp_SOURCES = mbtrnpp.c

mbtrnpp_LDADD =  $(top_builddir)/src/mbio/libmbio.la \
	$(top_builddir)/src/mbaux/libmbaux.la \
	$(LIBMBXGR) \
	${libgmt_LIBS} ${libnetcdf_LIBS} ${libproj_LIBS} \
    $(MBTRNLIBS) \
    $(TRNWLIBS) $(LIBM)

mbgrd2octree_SOURCES = mbgrd2octree.cc
mbgrd2octree_LDADD =  \
	${libgmt_LIBS} ${libnetcdf_LIBS} ${libproj_LIBS} \
  $(TRNWLIBS) $(LIBM)

CLEANFILES =
DISTCLEANFILES =
