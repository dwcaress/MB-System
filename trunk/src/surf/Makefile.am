AM_CPPFLAGS = -I@top_srcdir@/src

include_HEADERS = sapi.h

lib_LTLIBRARIES = libmbsapi.la

libmbsapi_la_LDFLAGS = -no-undefined -version-info 0:0:0

libmbsapi_la_LIBADD = @XDR_LIB@

libmbsapi_la_SOURCES = \
		mem_surf.c pb_math.c sapi_build.c sapi_data.c sapi_files.c \
		sapi_globals.c sapi_simple.c util_surf.c xdr_surf.c

#xdr_win32.c for win32??
