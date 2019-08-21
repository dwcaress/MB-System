lib_LTLIBRARIES = libmbtrn.la

include_HEADERS = mframe.h \
				mcbuf.h \
                msocket.h \
                mthread.h \
				mfile.h \
				mtime.h\
				mconfig.h \
                medebug.h \
                mmdebug.h \
				merror.h \
				mlog.h \
                mlist.h \
                mstats.h \
                mbtrn.h \
                mbtrn-server.h \
                mbtrn-test.h \
				r7kc.h

AM_CFLAGS = ${libgmt_CFLAGS} ${libnetcdf_CFLAGS}

# add -DMBTRN_VER="<version>" to override version defined in mbtrn.h

AM_CPPFLAGS = -g -O0 -I${top_srcdir}/src/mbtrn -DHAVE_CONFIG_H  -DWITH_TESTS -DWITH_MST_MSTATS -DMST_STATS_EN -DWITH_PDEBUG -DWITH_MMDEBUG -D_GNU_SOURCE -DMBTRN_BUILD="`date +%Y/%m/%dT%H:%M:%S%z`"


    WITH_MEDEBUG_DEBUG     : enable medebug debug macros

libmbtrn_la_LDFLAGS =  -no-undefined -version-info 0:0:0

libmbtrn_la_SOURCES = \
				mframe.c \
                mcbuf.c \
                msocket.c \
                mthread.c \
                mfile.c \
                mtime.c\
                mmdebug.c \
                mconfig.c \
                merror.c \
                mlog.c \
                mlist.c \
                mstats.c \
				mswap.c \
				mbtrn.c \
                mbtrn-server.c \
				r7kc.c


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
