lib_LTLIBRARIES = libmbtrn.la

include_HEADERS =
include_HEADERS += mframe.h
include_HEADERS += mcbuf.h
include_HEADERS += msocket.h
include_HEADERS += mthread.h
include_HEADERS += mfile.h
include_HEADERS += mtime.h
include_HEADERS += mconfig.h
include_HEADERS += medebug.h
include_HEADERS += mmdebug.h
include_HEADERS += merror.h
include_HEADERS += mlog.h
include_HEADERS += mlist.h
include_HEADERS += mstats.h
include_HEADERS += mbtrn.h
include_HEADERS += mbtrn-server.h
include_HEADERS += mbtrn-test.h
include_HEADERS += r7kc.h

AM_CFLAGS = ${libgmt_CFLAGS} ${libnetcdf_CFLAGS}

# add -DMBTRN_VER="<version>" to override version defined in mbtrn.h

AM_CPPFLAGS =
AM_CPPFLAGS += -g -O0
AM_CPPFLAGS += -I${top_srcdir}/src/mbtrn
AM_CPPFLAGS += -DHAVE_CONFIG_H
AM_CPPFLAGS += -DWITH_TESTS
AM_CPPFLAGS += -DWITH_MST_MSTATS
AM_CPPFLAGS += -DMST_STATS_EN
AM_CPPFLAGS += -DWITH_PDEBUG
AM_CPPFLAGS += -DWITH_MMDEBUG
AM_CPPFLAGS += -D_GNU_SOURCE
AM_CPPFLAGS += -DMBTRN_BUILD="`date +%Y/%m/%dT%H:%M:%S%z`"


    WITH_MEDEBUG_DEBUG     : enable medebug debug macros

libmbtrn_la_LDFLAGS =  -no-undefined -version-info 0:0:0

libmbtrn_la_SOURCES =
libmbtrn_la_SOURCES += mframe.c
libmbtrn_la_SOURCES += mcbuf.c
libmbtrn_la_SOURCES += msocket.c
libmbtrn_la_SOURCES += mthread.c
libmbtrn_la_SOURCES += mfile.c
libmbtrn_la_SOURCES += mtime.c
libmbtrn_la_SOURCES += mmdebug.c
libmbtrn_la_SOURCES += mconfig.c
libmbtrn_la_SOURCES += merror.c
libmbtrn_la_SOURCES += mlog.c
libmbtrn_la_SOURCES += mlist.c
libmbtrn_la_SOURCES += mstats.c
libmbtrn_la_SOURCES += mswap.c
libmbtrn_la_SOURCES += mbtrn.c
libmbtrn_la_SOURCES += mbtrn-server.c
libmbtrn_la_SOURCES += r7kc.c

libmbtrn_la_LIBADD = -lpthread

bin_PROGRAMS = tmbtrn stream7k frames7k trnc emu7k tbinx udps udpc mb1conv

tmbtrn_SOURCES = mbtrn-test.c
tmbtrn_LDADD = libmbtrn.la

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

dist_bin_SCRIPTS = mbtrnpp.sh emu7k.sh

CLEANFILES =
DISTCLEANFILES =
