lib_LTLIBRARIES = libmframe.la

include_HEADERS = \
        usr/mconfig.h \
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
        mframe/src/mkvconf.h

AM_CFLAGS =

# configure mframe compilation flags
BUILD_DATE=`date +%Y/%m/%dT%H:%M:%S%z`
MFRAME_VERSIONS=-DMFRAME_BUILD="${BUILD_DATE}" -DTRNC_VER=1.0 -DFRAMES7K_VER=1.0 -DSTREAM7K_VER=1.0 -DTBINX_VER=1.0 -DEMU7K_VER=1.0
MFRAME_TESTS=-DWITH_TESTS -DWITH_R7KR_TEST -DWITH_MB1R_TEST
MFRAME_FEATURES=-DWITH_MST_MSTATS -DMST_STATS_EN -DWITH_MMDEBUG
MFRAME_OPTS=$(MFRAME_VERSIONS) $(MFRAME_FEATURES) $(MFRAME_TESTS)

AM_CPPFLAGS = -g -O0 -I./usr -I./mframe/src -DHAVE_CONFIG_H  -D_GNU_SOURCE $(MFRAME_OPTS)

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
        usr/mconfig.c

libmframe_la_LIBADD =

bin_PROGRAMS =

dist_bin_SCRIPTS =

bin_SCRIPTS =

DIST_SUBDIRS =

CLEANFILES =
DISTCLEANFILES =
