
bin_PROGRAMS =
bin_PROGRAMS += mb7k2jstar
bin_PROGRAMS += mb7k2ss
bin_PROGRAMS += mb7kpreprocess
bin_PROGRAMS += mbauvnavusbl
bin_PROGRAMS += mbhsdump
bin_PROGRAMS += mbhysweeppreprocess
bin_PROGRAMS += mbinsreprocess
bin_PROGRAMS += mbkongsbergpreprocess
bin_PROGRAMS += mbneptune2esf
bin_PROGRAMS += mbrollbias
bin_PROGRAMS += mbrphsbias
bin_PROGRAMS += mbstripnan
bin_PROGRAMS += mbswplspreprocess

AM_CFLAGS = ${libgmt_CFLAGS} ${libnetcdf_CFLAGS}

AM_CPPFLAGS =
AM_CPPFLAGS += -I${top_srcdir}/src/mbio
AM_CPPFLAGS += -I${top_srcdir}/src/mbaux
AM_CPPFLAGS += ${libgmt_CPPFLAGS}
AM_CPPFLAGS += ${libnetcdf_CPPFLAGS}
AM_CPPFLAGS += ${libfftw_CPPFLAGS}

AM_LDFLAGS =

LIBS =
LIBS += ${top_builddir}/src/mbio/libmbio.la
LIBS += ${libgmt_LIBS}
LIBS += ${libnetcdf_LIBS}
LIBS += ${libproj_LIBS}
LIBS += $(LIBM)

mb7k2jstar_SOURCES = mb7k2jstar.cc
mb7k2ss_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mb7k2ss_SOURCES = mb7k2ss.cc
mb7kpreprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mb7kpreprocess_SOURCES = mb7kpreprocess.cc
mbauvnavusbl_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbauvnavusbl_SOURCES = mbauvnavusbl.cc
mbhsdump_SOURCES = mbhsdump.cc
mbhysweeppreprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbhysweeppreprocess_SOURCES = mbhysweeppreprocess.cc
mbinsreprocess_SOURCES = mbinsreprocess.cc
mbkongsbergpreprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbkongsbergpreprocess_SOURCES = mbkongsbergpreprocess.cc
mbneptune2esf_SOURCES = mbneptune2esf.cc
mbrollbias_SOURCES = mbrollbias.cc
mbrphsbias_SOURCES = mbrphsbias.cc
mbstripnan_SOURCES = mbstripnan.cc
mbswplspreprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbswplspreprocess_SOURCES = mbswplspreprocess.cc

bin_SCRIPTS = \
	mbm_fmtvel \
	mbm_grid \
	mbm_grd2geovrml
