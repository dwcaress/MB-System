lib_LTLIBRARIES = libgeolib.la libnewmat.la libqnx.la libtnav.la libtrnw.la libmframe.la

include_HEADERS =

AM_CFLAGS = ${libgmt_CFLAGS} ${libnetcdf_CFLAGS}

AM_CPPFLAGS = -g -O0 -I${top_srcdir}/src/mbtrnav/trnw -I${top_srcdir}/src/mbtrnav/terrain-nav -I${top_srcdir}/src/mbtrnav/qnx-utils -I${top_srcdir}/src/mbtrnav/newmat  -I${top_srcdir}/src/mbtrnav/gctp/source -I${top_srcdir}/src/mbtrnav/mframe/src -DHAVE_CONFIG_H  -DWITH_TESTS -DWITH_PDEBUG -DWITH_MMDEBUG -D_GNU_SOURCE


libgeolib_la_LDFLAGS =  -no-undefined -version-info 0:0:0

libgeolib_la_SOURCES = \
                gctp/source/gctp.c \
                gctp/source/alberfor.c \
                gctp/source/alberinv.c \
                gctp/source/alconfor.c \
                gctp/source/alconinv.c \
                gctp/source/azimfor.c \
                gctp/source/aziminv.c \
                gctp/source/eqconfor.c \
                gctp/source/eqconinv.c \
                gctp/source/equifor.c \
                gctp/source/equiinv.c \
                gctp/source/gnomfor.c \
                gctp/source/gnominv.c \
                gctp/source/goodfor.c \
                gctp/source/goodinv.c \
                gctp/source/gvnspfor.c \
                gctp/source/gvnspinv.c \
                gctp/source/hamfor.c \
                gctp/source/haminv.c \
                gctp/source/imolwfor.c \
                gctp/source/imolwinv.c \
                gctp/source/lamazfor.c \
                gctp/source/lamazinv.c \
                gctp/source/merfor.c \
                gctp/source/merinv.c \
                gctp/source/millfor.c \
                gctp/source/millinv.c \
                gctp/source/molwfor.c \
                gctp/source/molwinv.c \
                gctp/source/obleqfor.c  \
                gctp/source/obleqinv.c \
                gctp/source/omerfor.c \
                gctp/source/omerinv.c \
                gctp/source/orthfor.c \
                gctp/source/orthinv.c \
                gctp/source/polyfor.c \
                gctp/source/polyinv.c \
                gctp/source/psfor.c \
                gctp/source/psinv.c \
                gctp/source/robfor.c \
                gctp/source/robinv.c \
                gctp/source/sinfor.c \
                gctp/source/sininv.c \
                gctp/source/somfor.c \
                gctp/source/sominv.c \
                gctp/source/sterfor.c \
                gctp/source/sterinv.c \
                gctp/source/stplnfor.c \
                gctp/source/stplninv.c \
                gctp/source/tmfor.c \
                gctp/source/tminv.c \
                gctp/source/utmfor.c \
                gctp/source/utminv.c \
                gctp/source/vandgfor.c \
                gctp/source/vandginv.c \
                gctp/source/wivfor.c \
                gctp/source/wivinv.c \
                gctp/source/wviifor.c \
                gctp/source/wviiinv.c \
                gctp/source/for_init.c \
                gctp/source/inv_init.c \
                gctp/source/cproj.c \
                gctp/source/report.c \
                gctp/source/lamccfor.c \
                gctp/source/lamccinv.c \
                gctp/source/paksz.c \
                gctp/source/untfz.c \
                gctp/source/sphdz.c \
                gctp/source/br_gctp.c

libnewmat_la_LDFLAGS =  -no-undefined -version-info 0:0:0

libnewmat_la_SOURCES = \
                newmat/newmat1.cpp \
                newmat/newmat2.cpp \
                newmat/newmat3.cpp \
                newmat/newmat4.cpp \
                newmat/newmat5.cpp \
                newmat/newmat6.cpp \
                newmat/newmat7.cpp \
                newmat/newmat8.cpp \
                newmat/newmatex.cpp \
                newmat/bandmat.cpp \
                newmat/submat.cpp \
                newmat/myexcept.cpp \
                newmat/cholesky.cpp \
                newmat/evalue.cpp \
                newmat/fft.cpp \
                newmat/hholder.cpp \
                newmat/jacobi.cpp \
                newmat/newfft.cpp \
                newmat/sort.cpp \
                newmat/svd.cpp \
                newmat/newmatrm.cpp \
                newmat/newmat9.cpp

libnewmat_la_LIBADD = -lm

libqnx_la_LDFLAGS =  -no-undefined -version-info 0:0:0

libqnx_la_SOURCES = \
                qnx-utils/AsciiFile.cc \
                qnx-utils/AngleData.cc \
                qnx-utils/CharData.cc \
                qnx-utils/BinaryFile.cc \
                qnx-utils/DataLog.cc \
                qnx-utils/DataLogReader.cc \
                qnx-utils/DataLogWriter.cc \
                qnx-utils/FileData.cc \
                qnx-utils/StringConverter.cc \
                qnx-utils/StringData.cc \
                qnx-utils/TimeP.cc \
                qnx-utils/TimeTag.cc \
                qnx-utils/ShortData.cc \
                qnx-utils/Math.cc \
                qnx-utils/NavUtils.cc \
                qnx-utils/IntegerData.cc \
                qnx-utils/FloatData.cc \
                qnx-utils/DoubleData.cc \
                qnx-utils/DataField.cc \
                qnx-utils/DataFieldFactory.cc \
                qnx-utils/LogFile.cc

libqnx_la_LIBADD = libgeolib.la \
				-lm -lstdc++

libtnav_la_LDFLAGS =  -no-undefined -version-info 0:0:0

libtnav_la_SOURCES = \
                terrain-nav/TerrainNav.cpp \
                terrain-nav/TNavConfig.cpp \
                terrain-nav/TNavFilter.cpp \
                terrain-nav/TNavPointMassFilter.cpp \
                terrain-nav/TNavParticleFilter.cpp \
				terrain-nav/TNavBankFilter.cpp \
				terrain-nav/TNavPFLog.cpp \
				terrain-nav/TerrainMapOctree.cpp \
                terrain-nav/PositionLog.cpp \
                terrain-nav/TerrainNavLog.cpp \
                terrain-nav/mapio.cpp \
                terrain-nav/structDefs.cpp \
                terrain-nav/trn_log.cpp \
                terrain-nav/myOutput.cpp \
                terrain-nav/matrixArrayCalcs.cpp \
                terrain-nav/TerrainMapDEM.cpp \
                terrain-nav/OctreeSupport.cpp \
                terrain-nav/Octree.cpp \
                terrain-nav/OctreeNode.cpp \
                terrain-nav/TRNUtils.cpp

libmframe_la_LDFLAGS =  -no-undefined -version-info 0:0:0

libmframe_la_SOURCES = \
                mframe/src/mframe.c \
                mframe/src/mfile.c \
                mframe/src/msocket.c \
                mframe/src/mlist.c \
                mframe/src/mmdebug.c \
                mframe/src/mutils.c \
                mframe/src/mthread.c \
                mframe/src/merror.c \
                mframe/src/mtime.c \
                mframe/src/mstats.c \
                mframe/src/mlog.c

libmframe_la_LIBADD =


libtnav_la_LIBADD = libgeolib.la \
					libnewmat.la \
					libqnx.la \
					-lm -lnetcdf -lrt -lpthread

libtrnw_la_LDFLAGS =  -no-undefined -version-info 0:0:0

libtrnw_la_SOURCES = \
                trnw/trnw.cpp \
                trnw/mb1_msg.c

libtrnw_la_LIBADD = libtnav.la \
					libqnx.la \
					libnewmat.la \
					-lm -lpthread


bin_PROGRAMS =   trn-server trn-cli #  readlog writelog


trn_cli_SOURCES = trnw/trncli_test.c trnw/trn_cli.c
trn_cli_LDADD = libmframe.la libtrnw.la libqnx.la libnewmat.la  libtnav.la  libgeolib.la

trn_server_SOURCES = utils/trn_server.cpp
trn_server_LDADD = libtnav.la libqnx.la libnewmat.la libtnav.la libgeolib.la

dist_bin_SCRIPTS = 

CLEANFILES =
DISTCLEANFILES =
