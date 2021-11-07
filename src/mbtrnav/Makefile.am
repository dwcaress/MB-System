lib_LTLIBRARIES = libgeolib.la libnewmat.la libqnx.la libtnav.la libtrnw.la libnetif.la

include_HEADERS =

AM_CFLAGS = ${libgmt_CFLAGS} ${libnetcdf_CFLAGS}

AM_CPPFLAGS = -g -O0
AM_CPPFLAGS += -I${top_srcdir}/src/mbtrnav/trnw
AM_CPPFLAGS += -I${top_srcdir}/src/mbtrnav/utils
AM_CPPFLAGS += -I${top_srcdir}/src/mbtrnav/terrain-nav
AM_CPPFLAGS += -I${top_srcdir}/src/mbtrnav/qnx-utils
AM_CPPFLAGS += -I${top_srcdir}/src/mbtrnav/newmat
AM_CPPFLAGS += -I${top_srcdir}/src/mbtrnav/gctp/source
AM_CPPFLAGS += -I${top_srcdir}/src/mbtrn/utils
AM_CPPFLAGS += -I${top_srcdir}/src/mbtrn/mframe/src
AM_CPPFLAGS += ${libgmt_CFLAGS}
AM_CPPFLAGS += ${libnetcdf_CFLAGS}
AM_CPPFLAGS += -DHAVE_CONFIG_H
AM_CPPFLAGS += -DWITH_TESTS
AM_CPPFLAGS += -DWITH_MMDEBUG
AM_CPPFLAGS += -D_GNU_SOURCE
AM_CPPFLAGS += -DWITH_MST_MSTATS
AM_CPPFLAGS += -DMST_STATS_EN

libgeolib_la_LDFLAGS =  -no-undefined -version-info 0:0:0

libgeolib_la_SOURCES =
libgeolib_la_SOURCES += gctp/source/gctp.c
libgeolib_la_SOURCES += gctp/source/alberfor.c
libgeolib_la_SOURCES += gctp/source/alberinv.c
libgeolib_la_SOURCES += gctp/source/alconfor.c
libgeolib_la_SOURCES += gctp/source/alconinv.c
libgeolib_la_SOURCES += gctp/source/azimfor.c
libgeolib_la_SOURCES += gctp/source/aziminv.c
libgeolib_la_SOURCES += gctp/source/eqconfor.c
libgeolib_la_SOURCES += gctp/source/eqconinv.c
libgeolib_la_SOURCES += gctp/source/equifor.c
libgeolib_la_SOURCES += gctp/source/equiinv.c
libgeolib_la_SOURCES += gctp/source/gnomfor.c
libgeolib_la_SOURCES += gctp/source/gnominv.c
libgeolib_la_SOURCES += gctp/source/goodfor.c
libgeolib_la_SOURCES += gctp/source/goodinv.c
libgeolib_la_SOURCES += gctp/source/gvnspfor.c
libgeolib_la_SOURCES += gctp/source/gvnspinv.c
libgeolib_la_SOURCES += gctp/source/hamfor.c
libgeolib_la_SOURCES += gctp/source/haminv.c
libgeolib_la_SOURCES += gctp/source/imolwfor.c
libgeolib_la_SOURCES += gctp/source/imolwinv.c
libgeolib_la_SOURCES += gctp/source/lamazfor.c
libgeolib_la_SOURCES += gctp/source/lamazinv.c
libgeolib_la_SOURCES += gctp/source/merfor.c
libgeolib_la_SOURCES += gctp/source/merinv.c
libgeolib_la_SOURCES += gctp/source/millfor.c
libgeolib_la_SOURCES += gctp/source/millinv.c
libgeolib_la_SOURCES += gctp/source/molwfor.c
libgeolib_la_SOURCES += gctp/source/molwinv.c
libgeolib_la_SOURCES += gctp/source/obleqfor.c
libgeolib_la_SOURCES += gctp/source/obleqinv.c
libgeolib_la_SOURCES += gctp/source/omerfor.c
libgeolib_la_SOURCES += gctp/source/omerinv.c
libgeolib_la_SOURCES += gctp/source/orthfor.c
libgeolib_la_SOURCES += gctp/source/orthinv.c
libgeolib_la_SOURCES += gctp/source/polyfor.c
libgeolib_la_SOURCES += gctp/source/polyinv.c
libgeolib_la_SOURCES += gctp/source/psfor.c
libgeolib_la_SOURCES += gctp/source/psinv.c
libgeolib_la_SOURCES += gctp/source/robfor.c
libgeolib_la_SOURCES += gctp/source/robinv.c
libgeolib_la_SOURCES += gctp/source/sinfor.c
libgeolib_la_SOURCES += gctp/source/sininv.c
libgeolib_la_SOURCES += gctp/source/somfor.c
libgeolib_la_SOURCES += gctp/source/sominv.c
libgeolib_la_SOURCES += gctp/source/sterfor.c
libgeolib_la_SOURCES += gctp/source/sterinv.c
libgeolib_la_SOURCES += gctp/source/stplnfor.c
libgeolib_la_SOURCES += gctp/source/stplninv.c
libgeolib_la_SOURCES += gctp/source/tmfor.c
libgeolib_la_SOURCES += gctp/source/tminv.c
libgeolib_la_SOURCES += gctp/source/utmfor.c
libgeolib_la_SOURCES += gctp/source/utminv.c
libgeolib_la_SOURCES += gctp/source/vandgfor.c
libgeolib_la_SOURCES += gctp/source/vandginv.c
libgeolib_la_SOURCES += gctp/source/wivfor.c
libgeolib_la_SOURCES += gctp/source/wivinv.c
libgeolib_la_SOURCES += gctp/source/wviifor.c
libgeolib_la_SOURCES += gctp/source/wviiinv.c
libgeolib_la_SOURCES += gctp/source/for_init.c
libgeolib_la_SOURCES += gctp/source/inv_init.c
libgeolib_la_SOURCES += gctp/source/cproj.c
libgeolib_la_SOURCES += gctp/source/report.c
libgeolib_la_SOURCES += gctp/source/lamccfor.c
libgeolib_la_SOURCES += gctp/source/lamccinv.c
libgeolib_la_SOURCES += gctp/source/paksz.c
libgeolib_la_SOURCES += gctp/source/untfz.c
libgeolib_la_SOURCES += gctp/source/sphdz.c
libgeolib_la_SOURCES += gctp/source/br_gctp.c

libnewmat_la_LDFLAGS =  -no-undefined -version-info 0:0:0

libnewmat_la_SOURCES =
libnewmat_la_SOURCES += newmat/newmat1.cpp
libnewmat_la_SOURCES += newmat/newmat2.cpp
libnewmat_la_SOURCES += newmat/newmat3.cpp
libnewmat_la_SOURCES += newmat/newmat4.cpp
libnewmat_la_SOURCES += newmat/newmat5.cpp
libnewmat_la_SOURCES += newmat/newmat6.cpp
libnewmat_la_SOURCES += newmat/newmat7.cpp
libnewmat_la_SOURCES += newmat/newmat8.cpp
libnewmat_la_SOURCES += newmat/newmatex.cpp
libnewmat_la_SOURCES += newmat/bandmat.cpp
libnewmat_la_SOURCES += newmat/submat.cpp
libnewmat_la_SOURCES += newmat/myexcept.cpp
libnewmat_la_SOURCES += newmat/cholesky.cpp
libnewmat_la_SOURCES += newmat/evalue.cpp
libnewmat_la_SOURCES += newmat/fft.cpp
libnewmat_la_SOURCES += newmat/hholder.cpp
libnewmat_la_SOURCES += newmat/jacobi.cpp
libnewmat_la_SOURCES += newmat/newfft.cpp
libnewmat_la_SOURCES += newmat/sort.cpp
libnewmat_la_SOURCES += newmat/svd.cpp
libnewmat_la_SOURCES += newmat/newmatrm.cpp
libnewmat_la_SOURCES += newmat/newmat9.cpp

libnewmat_la_LIBADD = -lm

libqnx_la_LDFLAGS =  -no-undefined -version-info 0:0:0

libqnx_la_SOURCES =
libqnx_la_SOURCES += qnx-utils/AsciiFile.cc
libqnx_la_SOURCES += qnx-utils/AngleData.cc
libqnx_la_SOURCES += qnx-utils/CharData.cc
libqnx_la_SOURCES += qnx-utils/BinaryFile.cc
libqnx_la_SOURCES += qnx-utils/DataLog.cc
libqnx_la_SOURCES += qnx-utils/DataLogReader.cc
libqnx_la_SOURCES += qnx-utils/DataLogWriter.cc
libqnx_la_SOURCES += qnx-utils/FileData.cc
libqnx_la_SOURCES += qnx-utils/StringConverter.cc
libqnx_la_SOURCES += qnx-utils/StringData.cc
libqnx_la_SOURCES += qnx-utils/TimeP.cc
libqnx_la_SOURCES += qnx-utils/TimeTag.cc
libqnx_la_SOURCES += qnx-utils/ShortData.cc
libqnx_la_SOURCES += qnx-utils/Math.cc
libqnx_la_SOURCES += qnx-utils/NavUtils.cc
libqnx_la_SOURCES += qnx-utils/IntegerData.cc
libqnx_la_SOURCES += qnx-utils/FloatData.cc
libqnx_la_SOURCES += qnx-utils/DoubleData.cc
libqnx_la_SOURCES += qnx-utils/DataField.cc
libqnx_la_SOURCES += qnx-utils/DataFieldFactory.cc
libqnx_la_SOURCES += qnx-utils/LogFile.cc

libqnx_la_LIBADD = libgeolib.la -lm -lstdc++

libtnav_la_LDFLAGS =  -no-undefined -version-info 0:0:0

libtnav_la_SOURCES =
libtnav_la_SOURCES += terrain-nav/TerrainNav.cpp
libtnav_la_SOURCES += terrain-nav/TNavConfig.cpp
libtnav_la_SOURCES += terrain-nav/TNavFilter.cpp
libtnav_la_SOURCES += terrain-nav/TNavPointMassFilter.cpp
libtnav_la_SOURCES += terrain-nav/TNavParticleFilter.cpp
libtnav_la_SOURCES += terrain-nav/TNavBankFilter.cpp
libtnav_la_SOURCES += terrain-nav/TNavPFLog.cpp
libtnav_la_SOURCES += terrain-nav/TerrainMapOctree.cpp
libtnav_la_SOURCES += terrain-nav/PositionLog.cpp
libtnav_la_SOURCES += terrain-nav/TerrainNavLog.cpp
libtnav_la_SOURCES += terrain-nav/mapio.cpp
libtnav_la_SOURCES += terrain-nav/structDefs.cpp
libtnav_la_SOURCES += terrain-nav/trn_log.cpp
libtnav_la_SOURCES += terrain-nav/myOutput.cpp
libtnav_la_SOURCES += terrain-nav/matrixArrayCalcs.cpp
libtnav_la_SOURCES += terrain-nav/TerrainMapDEM.cpp
libtnav_la_SOURCES += terrain-nav/OctreeSupport.cpp
libtnav_la_SOURCES += terrain-nav/Octree.cpp
libtnav_la_SOURCES += terrain-nav/OctreeNode.cpp
libtnav_la_SOURCES += terrain-nav/TRNUtils.cpp

libtnav_la_LIBADD = libgeolib.la
libtnav_la_LIBADD += libnewmat.la
libtnav_la_LIBADD += libqnx.la
libtnav_la_LIBADD += -lm -lnetcdf -lpthread

libtrnw_la_LDFLAGS =  -no-undefined -version-info 0:0:0

libtrnw_la_SOURCES =
libtrnw_la_SOURCES += trnw/trnw.cpp
libtrnw_la_SOURCES += trnw/mb1_msg.c
libtrnw_la_SOURCES += trnw/trnif_msg.c

libtrnw_la_LIBADD = libtnav.la
libtrnw_la_LIBADD += libqnx.la
libtrnw_la_LIBADD += libnewmat.la
libtrnw_la_LIBADD += -lm -lpthread

libnetif_la_LDFLAGS =  -no-undefined -version-info 0:0:0

libnetif_la_SOURCES =
libnetif_la_SOURCES += trnw/netif.c
libnetif_la_SOURCES += trnw/trnif_proto.c

libnetif_la_LIBADD = libtrnw.la
libnetif_la_LIBADD += $(top_builddir)/src/mbtrn/libmframe.la
libnetif_la_LIBADD += -lm -lpthread

bin_PROGRAMS =   trn-server trn-cli netif-test trnif-test trnifsvr-test trnclient-test trnu-cli trnusvr-test mmcpub mmcsub trn-replay #  readlog writelog

trn_replay_SOURCES = opt/dorado/Replay.cpp opt/dorado/trn_replay.cpp utils/TerrainNavClient.cpp
trn_replay_LDADD = -ltnav  -lnewmat -lqnx -lnetcdf -lm -lpthread -lgeolib

mmcpub_SOURCES = trnw/mmcpub.c
mmcpub_LDADD = $(top_builddir)/src/mbtrn/libmframe.la

mmcsub_SOURCES = trnw/mmcsub.c
mmcsub_LDADD = $(top_builddir)/src/mbtrn/libmframe.la

trnu_cli_SOURCES = trnw/trnucli_test.c trnw/trnu_cli.c
trnu_cli_LDADD = $(top_builddir)/src/mbtrn/libmframe.la libtrnw.la

trn_cli_SOURCES = trnw/trncli_test.c trnw/trn_cli.c
trn_cli_LDADD = $(top_builddir)/src/mbtrn/libmframe.la libtrnw.la

trn_server_SOURCES = utils/trn_server.cpp
trn_server_LDADD = libtnav.la libqnx.la libnewmat.la libtnav.la libgeolib.la

netif_test_SOURCES = utils/netif-test.c
netif_test_LDADD = libnetif.la ${top_builddir}/src/mbtrn/libr7kr.la libtrnw.la

trnif_test_SOURCES =  trnw/trnif_test.c
trnif_test_LDADD = libnetif.la  ${top_builddir}/src/mbtrn/libr7kr.la libtrnw.la

trnifsvr_test_SOURCES = utils/trnifsvr-test.c
trnifsvr_test_LDADD = libnetif.la  ${top_builddir}/src/mbtrn/libr7kr.la libtrnw.la

trnclient_test_SOURCES =  utils/trnclient_test.cpp utils/TrnClient.cpp utils/TerrainNavClient.cpp
trnclient_test_LDADD = libtnav.la

trnusvr_test_SOURCES = trnw/trnusvr_test.c
trnusvr_test_LDADD = libnetif.la libtrnw.la

dist_bin_SCRIPTS =

CLEANFILES =
DISTCLEANFILES =
