AM_CPPFLAGS = -I@top_srcdir@/src

include_HEADERS = mr1pr.h mr1pr_defines.h

lib_LTLIBRARIES = libmbmr1pr.la

libmbmr1pr_la_LDFLAGS = -no-undefined -version-info 0:0:0

libmbmr1pr_la_LIBADD = @XDR_LIB@

libmbmr1pr_la_SOURCES = io.c iov1.c memalloc.c misc.c swapbytes.c
