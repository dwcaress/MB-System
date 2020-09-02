lib_LTLIBRARIES = libmframe.la libr7kr.la

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
        mframe/src/mkvconf.h \
        utils/mconfig.h \
        utils/mb1_msg.h \
        utils/mb71_msg.h \
        r7kr/r7k-reader.h \
        r7kr/r7kc.h

AM_CFLAGS = ${libgmt_CFLAGS} ${libnetcdf_CFLAGS}

# configure mframe compilation flags
BUILD_DATE=`date +%Y/%m/%dT%H:%M:%S%z`
MFRAME_VERSIONS=-DMFRAME_BUILD="${BUILD_DATE}" -DTRNC_VER=1.0 -DFRAMES7K_VER=1.0 -DSTREAM7K_VER=1.0 -DTBINX_VER=1.0 -DEMU7K_VER=1.0
MFRAME_TESTS=-DWITH_TESTS -DWITH_R7KR_TEST
MFRAME_FEATURES=-DWITH_MST_MSTATS -DMST_STATS_EN -DWITH_PDEBUG -DWITH_MMDEBUG
MFRAME_OPTS=$(MFRAME_VERSIONS) $(MFRAME_FEATURES) $(MFRAME_TESTS)

AM_CPPFLAGS = -g -O0 -I${top_srcdir}/src/mbtrn/r7kr -I${top_srcdir}/src/mbtrn/utils -I${top_srcdir}/src/mbtrn/mframe/src -I${top_srcdir}/src/mbtrnav/trnw \
 -DHAVE_CONFIG_H  -D_GNU_SOURCE $(MFRAME_OPTS)

libmframe_la_LDFLAGS =  -no-undefined -version-info 0:0:0

libmframe_la_SOURCES = mframe/src/mframe.c \
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
        mframe/src/mkvconf.c \
        utils/mconfig.c

libmframe_la_LIBADD =

libr7kr_la_LDFLAGS =  -no-undefined -version-info 0:0:0

libr7kr_la_SOURCES =  utils/mconfig.c \
        utils/mb1_msg.c \
        utils/mb71_msg.c \
        r7kr/r7k-reader.c \
        r7kr/r7kc.c

libr7kr_la_LIBADD = libmframe.la -lpthread

bin_PROGRAMS =  stream7k frames7k emu7k trnc tbinx udps udpc mb1conv mb12csv r7kr_test mbtnav-cli

stream7k_SOURCES = utils/stream7k.c
stream7k_LDADD = libr7kr.la

frames7k_SOURCES = utils/frames7k.c
frames7k_LDADD = libr7kr.la

trnc_SOURCES = utils/trnc.c
trnc_LDADD = libr7kr.la

emu7k_SOURCES = utils/emu7k.c
emu7k_LDADD = libr7kr.la

tbinx_SOURCES = utils/tbinx.c
tbinx_LDADD = libr7kr.la

udps_SOURCES = utils/udps.c
udps_LDADD = libr7kr.la

udpc_SOURCES = utils/udpc.c
udpc_LDADD = libr7kr.la

mb1conv_SOURCES = utils/mb1conv.c
mb1conv_LDADD = libr7kr.la

mb12csv_SOURCES = utils/mb12csv.c
mb12csv_LDADD = libr7kr.la

r7kr_test_SOURCES = r7kr/r7kr-test.c
r7kr_test_LDADD = libr7kr.la

mbtnav_cli_SOURCES = utils/mbtnav_cli.c
mbtnav_cli_LDADD = libr7kr.la

dist_bin_SCRIPTS = tools/bin/mbtrnpp.sh emu7k.sh

bin_SCRIPTS = mbtrnpp.sh

DIST_SUBDIRS = tools

CLEANFILES =
DISTCLEANFILES =
