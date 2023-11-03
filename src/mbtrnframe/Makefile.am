lib_LTLIBRARIES = libmbtrnframe.la

include_HEADERS = \
        mconfig.h \
        mframe.h \
        mbbuf.h \
        mcbuf.h \
        msocket.h \
        mthread.h \
        mfile.h \
        mtime.h\
        medebug.h \
        mmdebug.h \
        merror.h \
        mlog.h \
        mlist.h \
        mstats.h \
        mkvconf.h \
        mutils.h

AM_CFLAGS =

# configure mframe compilation flags
BUILD_DATE=`date +%Y/%m/%dT%H:%M:%S%z`
MFRAME_VERSIONS=-DMFRAME_BUILD="${BUILD_DATE}" -DTRNC_VER=1.0 -DFRAMES7K_VER=1.0 -DSTREAM7K_VER=1.0 -DTBINX_VER=1.0 -DEMU7K_VER=1.0
MFRAME_TESTS=-DWITH_TESTS -DWITH_R7KR_TEST -DWITH_MB1R_TEST
MFRAME_FEATURES=-DWITH_MST_MSTATS -DMST_STATS_EN -DMXDEBUG_C_API 
MFRAME_OPTS=$(MFRAME_VERSIONS) $(MFRAME_FEATURES) $(MFRAME_TESTS)

AM_CPPFLAGS = -g -O0 -DHAVE_CONFIG_H  -D_GNU_SOURCE $(MFRAME_OPTS)

libmbtrnframe_la_LDFLAGS =  -no-undefined -version-info 0:0:0

libmbtrnframe_la_SOURCES = mframe.c \
        mbbuf.c \
        mcbuf.c \
        mconfig.c \
        merror.c \
        mfile.c \
        mkvconf.c \
        mlist.c \
        mlog.c \
        mmdebug.c \
        msocket.c \
        mstats.c \
        mswap.c \
        mthread.c \
        mtime.c\
        mutils.c \
        mxdebug.cpp

libmbtrnframe_la_LIBADD =

bin_PROGRAMS =

dist_bin_SCRIPTS =

bin_SCRIPTS =

DIST_SUBDIRS =

CLEANFILES =
DISTCLEANFILES =
