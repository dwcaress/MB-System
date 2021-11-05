lib_LTLIBRARIES = libr7kr.la libmb1r.la

LIBMFRAME = ${top_srcdir}/src/mbtrnframe/libmframe.la

include_HEADERS = \
        utils/mconfig.h \
        utils/mb71_msg.h \
        r7kr/r7k-reader.h \
        r7kr/r7kc.h \
        mb1r/mb1-reader.h
        

AM_CFLAGS = ${libgmt_CFLAGS} ${libnetcdf_CFLAGS}

# configure mframe compilation flags
BUILD_DATE=`date +%Y/%m/%dT%H:%M:%S%z`
MFRAME_VERSIONS=-DMFRAME_BUILD="${BUILD_DATE}" -DTRNC_VER=1.0 -DFRAMES7K_VER=1.0 -DSTREAM7K_VER=1.0 -DTBINX_VER=1.0 -DEMU7K_VER=1.0
MFRAME_TESTS=-DWITH_TESTS -DWITH_R7KR_TEST -DWITH_MB1R_TEST
MFRAME_FEATURES=-DWITH_MST_MSTATS -DMST_STATS_EN -DWITH_MMDEBUG
MFRAME_OPTS=$(MFRAME_VERSIONS) $(MFRAME_FEATURES) $(MFRAME_TESTS)

AM_CPPFLAGS = -g -O0 -I${top_srcdir}/src/mbtrn/r7kr -I${top_srcdir}/src/mbtrn/mb1r -I${top_srcdir}/src/mbtrn/utils -I${top_srcdir}/src/mbtrnframe/mframe/src -I${top_srcdir}/src/mbtrnframe/utils -I${top_srcdir}/src/mbtrnav/trnw -I${top_srcdir}/src/mbtrnav/terrain-nav \
 -DHAVE_CONFIG_H  -D_GNU_SOURCE $(MFRAME_OPTS)

libr7kr_la_LDFLAGS =  -no-undefined -version-info 0:0:0


libr7kr_la_SOURCES =  utils/mconfig.c \
		utils/mb71_msg.c \
		r7kr/r7k-reader.c \
		r7kr/r7kc.c

libr7kr_la_LIBADD = ${LIBMFRAME} -lpthread


libmb1r_la_LDFLAGS =  -no-undefined -version-info 0:0:0

libmb1r_la_SOURCES =  \
		utils/mb71_msg.c \
		utils/mconfig.c \
		mb1r/mb1-reader.c

libmb1r_la_LIBADD = ${top_srcdir}/src/mbtrnav/libmb1.la ${LIBMFRAME} -lpthread

bin_PROGRAMS =  udps udpc mbtnav-cli stream7k frames7k emu7k r7kr_test trnc tbinx mb1conv mb12csv mb1r_test


udps_SOURCES = utils/udps.c
udps_LDADD = ${LIBMFRAME}

udpc_SOURCES = utils/udpc.c
udpc_LDADD = ${LIBMFRAME}

mbtnav_cli_SOURCES = utils/mbtnav_cli.c
mbtnav_cli_LDADD = ${LIBMFRAME}

stream7k_SOURCES = utils/stream7k.c
stream7k_LDADD = libr7kr.la

frames7k_SOURCES = utils/frames7k.c
frames7k_LDADD = libr7kr.la

emu7k_SOURCES = utils/emu7k.c
emu7k_LDADD = libr7kr.la

r7kr_test_SOURCES = r7kr/r7kr-test.c
r7kr_test_LDADD = libr7kr.la

trnc_SOURCES = utils/trnc.c
trnc_LDADD = libmb1r.la

tbinx_SOURCES = utils/tbinx.c
tbinx_LDADD = libmb1r.la libr7kr.la

mb1conv_SOURCES = utils/mb1conv.c
mb1conv_LDADD = libmb1r.la

mb12csv_SOURCES = utils/mb12csv.c
mb12csv_LDADD = libmb1r.la

mb1r_test_SOURCES = mb1r/mb1r-test.c
mb1r_test_LDADD = libmb1r.la

dist_bin_SCRIPTS = tools/bin/mbtrnpp.sh emu7k.sh

bin_SCRIPTS = tools/bin/mbtrnpp.sh

DIST_SUBDIRS = tools

CLEANFILES =
DISTCLEANFILES =
