lib_LTLIBRARIES = libmbtrn.la

include_HEADERS = cbuffer.h \
				iowrap.h \
				iowrap-posix.h \
				iowrap-win.h\
				mbtrn.h \
				mbtrn-server.h \
				mbtrn-test.h \
				mconfig.h \
				mdebug.h \
				merror.h \
				mlog.h \
				mlist.h \
				r7kc.h

AM_CFLAGS = ${libgmt_CFLAGS} ${libnetcdf_CFLAGS}

AM_CPPFLAGS = -I${top_srcdir}/src/mbtrn -I${top_srcdir}/src/trn -DHAVE_CONFIG_H  -DWITH_TEST -DMD_OLEVEL=MD_DEBUG

AM_LDFLAGS = -L$(top_builddir)/src/mbtrn

libmbtrn_la_LDFLAGS =  -no-undefined -version-info 0:0:0

libmbtrn_la_SOURCES = \
				cbuffer.c \
				iowrap.c \
				iowrap-posix.c \
				mbtrn-server.c \
				mconfig.c \
				mdebug.c \
				merror.c \
				mlog.c \
				mlist.c \
				mbtrn.c \
				r7kc.c

libmbtrn_la_LIBADD = -lpthread

bin_PROGRAMS = tmbtrn stream7k frames7k trnc emu7k

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


CLEANFILES =
DISTCLEANFILES =
