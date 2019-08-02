
bin_PROGRAMS = dump_gsf

include_HEADERS = gsf.h

AM_CFLAGS =

AM_CPPFLAGS = -DUSE_DEFAULT_FILE_FUNCTIONS

AM_LDFLAGS =

lib_LTLIBRARIES = libmbgsf.la

libmbgsf_la_LDFLAGS = -no-undefined -version-info 0:0:0

dump_gsf_SOURCES = dump_gsf.c
dump_gsf_LDADD = libmbgsf.la

libmbgsf_la_SOURCES = \
        gsf.c gsf_compress.c gsf_dec.c gsf_enc.c gsf_indx.c gsf_info.c
