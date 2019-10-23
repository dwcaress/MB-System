AM_CPPFLAGS = -I${top_srcdir}/src/mbio

include_HEADERS = mbbs.h mbbs_defines.h mbbs_mem.h

lib_LTLIBRARIES = libmbbsio.la

libmbbsio_la_LDFLAGS = -no-undefined -version-info 0:0:0

libmbbsio_la_LIBADD = ${XDR_LIB}

libmbbsio_la_SOURCES =
libmbbsio_la_SOURCES += mbbs_io.c
libmbbsio_la_SOURCES += mbbs_iomrv1.c
libmbbsio_la_SOURCES += mbbs_iomrv2.c
libmbbsio_la_SOURCES += mbbs_memalloc.c
libmbbsio_la_SOURCES += mbbs_misc.c
libmbbsio_la_SOURCES += mbbs_swapbytes.c
libmbbsio_la_SOURCES += mbbs_tm.c
libmbbsio_la_SOURCES += mbbs_wrhdrfields.c
libmbbsio_la_SOURCES += mbbs_wrsampflags.c
