lib_LTLIBRARIES = libmbtrn.la

include_HEADERS = \
				mframe/src/mframe.h \
				mframe/src/mcbuf.h \
                mframe/src/msocket.h \
                mframe/src/mthread.h \
				mframe/src/mfile.h \
				mframe/src/mtime.h\
                mframe/src/medebug.h \
                mframe/src/mmdebug.h \
				mframe/src/merror.h \
				mframe/src/mlog.h \
                mframe/src/mlist.h \
                mframe/src/mstats.h \
				mconfig.h \
                r7k-reader.h \
				r7kc.h

AM_CFLAGS = ${libgmt_CFLAGS} ${libnetcdf_CFLAGS}

# add -DMBTRN_VER="<version>" to override version defined in mbtrn.h
BUILD_DATE=`date +%Y/%m/%dT%H:%M:%S%z`

TRNC_VER='-DTRNC_VER=1.0'
FRAMES7K_VER='-DFRAMES7K_VER=1.0'
STREAM7K_VER='-DSTREAM7K_VER=1.0'
TBIN_VER='-DTBIN_VER=1.0'
EMU7K_VER='-DEMU_VER=1.0'
TEST_OPTS=-DWITH_TESTS -DWITH_R7KR_TEST
AM_CPPFLAGS = -g -O0 -I${top_srcdir}/src/mbtrn -I${top_srcdir}/src/mbtrn/mframe/src -DHAVE_CONFIG_H  ${TEST_OPTS} -DWITH_MST_MSTATS -DMST_STATS_EN -DWITH_PDEBUG -DWITH_MMDEBUG -D_GNU_SOURCE -DMFRAME_BUILD="${BUILD_DATE}" ${FRAMES7K_VER} ${TRNC_VER} ${STREAM7K_VER} ${TBIN_VER} ${EMU7K_VER}


    WITH_MEDEBUG_DEBUG     : enable medebug debug macros


libmbtrn_la_LDFLAGS =  -no-undefined -version-info 0:0:0

libmbtrn_la_SOURCES = \
				mframe/src/mframe.c \
                mframe/src/mcbuf.c \
                mframe/src/msocket.c \
                mframe/src/mthread.c \
                mframe/src/mfile.c \
                mframe/src/mtime.c\
                mframe/src/mmdebug.c \
                mframe/src/merror.c \
                mframe/src/mlog.c \
                mframe/src/mlist.c \
                mframe/src/mstats.c \
                mframe/src/mswap.c \
                mframe/src/mutils.c \
				mconfig.c \
                r7k-reader.c \
                r7kc.c


libmbtrn_la_LIBADD = -lpthread

bin_PROGRAMS =  stream7k frames7k emu7k trnc tbinx udps udpc mb1conv r7kr_test

stream7k_SOURCES = stream7k.c
stream7k_LDADD = libmbtrn.la

frames7k_SOURCES = frames7k.c
frames7k_LDADD = libmbtrn.la

trnc_SOURCES = trnc.c
trnc_LDADD = libmbtrn.la

emu7k_SOURCES = emu7k.c
emu7k_LDADD = libmbtrn.la

tbinx_SOURCES = tbinx.c
tbinx_LDADD = libmbtrn.la

udps_SOURCES = udps.c
udps_LDADD = libmbtrn.la

udpc_SOURCES = udpc.c
udpc_LDADD = libmbtrn.la

mb1conv_SOURCES = mb1conv.c mb1_msg.c mb71_msg.c
mb1conv_LDADD = libmbtrn.la

r7kr_test_SOURCES = r7kr-test.c
r7kr_test_LDADD = libmbtrn.la

dist_bin_SCRIPTS = mbtrnpp.sh emu7k.sh

CLEANFILES =
DISTCLEANFILES =
