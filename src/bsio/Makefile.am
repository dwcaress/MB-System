AM_CPPFLAGS = -I${top_srcdir}/src/mbio

include_HEADERS = mbbs.h mbbs_defines.h mbbs_mem.h

lib_LTLIBRARIES = libmbbsio.la

libmbbsio_la_LDFLAGS = -no-undefined -version-info 0:0:0

libmbbsio_la_LIBADD = ${XDR_LIB}

libmbbsio_la_SOURCES = mbbs_io.c mbbs_iomrv1.c \
                        mbbs_iomrv2.c mbbs_memalloc.c \
                        mbbs_misc.c mbbs_swapbytes.c \
                        mbbs_tm.c mbbs_utils.c \
                        mbbs_wrhdrfields.c mbbs_wrsampflags.c

