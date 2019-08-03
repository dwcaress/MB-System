bin_PROGRAMS = dump_gsf

include_HEADERS = gsf.h

AM_CFLAGS =

AM_CPPFLAGS = -DUSE_DEFAULT_FILE_FUNCTIONS

AM_LDFLAGS =

lib_LTLIBRARIES = libmbgsf.la

libmbgsf_la_LDFLAGS = -no-undefined -version-info 0:0:0

dump_gsf_SOURCES = dump_gsf.c
dump_gsf_LDADD = libmbgsf.la

libmbgsf_la_SOURCES =
libmbgsf_la_SOURCES += gsf.c
libmbgsf_la_SOURCES += gsf_compress.c
libmbgsf_la_SOURCES += gsf_dec.c
libmbgsf_la_SOURCES += gsf_enc.c
libmbgsf_la_SOURCES += gsf_indx.c
libmbgsf_la_SOURCES += gsf_info.c

