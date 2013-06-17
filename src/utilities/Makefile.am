if BUILD_FFTW
 FFTW_PROG = mbsegypsd
endif


bin_PROGRAMS = mbhsdump \
        mbformat \
        mbdefaults \
        mbdatalist \
        mbinfo \
        mbset \
        mbprocess \
        mbcopy \
        mblist \
        mbnavlist \
        mbsvplist \
        mbbackangle \
        mbclean \
        mbareaclean \
        mbhistogram \
        mbgrid \
        mbmosaic \
        mblevitus \
        mbrollbias \
        mbextractsegy \
        mbsegylist \
        mbsegyinfo \
        mbsegygrid \
        mbauvnavusbl \
        mbgetesf \
        mbneptune2esf \
        mbps \
        mbstripnan \
        mbrollbias \
        mb7k2jstar \
        mb7kpreprocess \
        mbrolltimelag \
        mbauvloglist \
        mb7k2ss \
        mbabsorption \
        mbdumpesf \
        mbtime \
        mbctdlist \
        mbfilter \
        mbroutetime \
        mbhysweeppreprocess \
        mbkongsbergpreprocess \
        $(FFTW_PROG)

#Remove mbps if WIN32?

include_HEADERS = levitus.h

AM_CFLAGS = ${libnetcdf_CFLAGS} ${libgmt_CFLAGS}

AM_CPPFLAGS = -I@top_srcdir@/src/mbio -I@top_srcdir@/src/mbaux @CDF_INC_DIR@ @GMT_INC_DIR@ @FFTW_INC_DIR@
if BUILD_GSF
  AM_CPPFLAGS += -I@top_srcdir@/src/gsf
endif

AM_LDFLAGS = -L$(top_builddir)/src/mbio -L$(top_builddir)/src/mbaux -L$(top_builddir)/src/gsf -L$(top_builddir)/src/mr1pr -L$(top_builddir)/src/proj -L$(top_builddir)/src/surf
if BUILD_GSF
  AM_LDFLAGS += -L$(top_builddir)/src/gsf
endif

LIBS = ${libnetcdf_LIBS} ${libgmt_LIBS} $(top_builddir)/src/mbio/libmbio.la

mbhsdump_SOURCES = mbhsdump.c

mbformat_SOURCES = mbformat.c

mbdefaults_SOURCES = mbdefaults.c

mbdatalist_SOURCES = mbdatalist.c

mbinfo_SOURCES = mbinfo.c

mbset_SOURCES = mbset.c

mbprocess_SOURCES = mbprocess.c
mbprocess_LDADD = $(top_builddir)/src/mbaux/libmbaux.la

mbcopy_SOURCES = mbcopy.c
# libmbiola.la already links against gsf if gsf is being built
#if BUILD_GSF
#  mbcopy_LDADD = $(top_builddir)/src/gsf/libmbgsf.la
#endif

mblist_SOURCES = mblist.c

mbnavlist_SOURCES = mbnavlist.c

mbsvplist_SOURCES = mbsvplist.c

mbbackangle_SOURCES = mbbackangle.c
mbbackangle_LDADD = $(top_builddir)/src/mbaux/libmbaux.la

mbclean_SOURCES = mbclean.c

mbareaclean_SOURCES = mbareaclean.c

mbhistogram_SOURCES = mbhistogram.c

mbgrid_SOURCES = mbgrid.c
mbgrid_LDADD = $(top_builddir)/src/mbaux/libmbaux.la

mbmosaic_SOURCES = mbmosaic.c
mbmosaic_LDADD = $(top_builddir)/src/mbaux/libmbaux.la

mblevitus_SOURCES = mblevitus.c

mbrollbias_SOURCES = mbrollbias.c

mbextractsegy_SOURCES = mbextractsegy.c

mbsegylist_SOURCES = mbsegylist.c

mbsegyinfo_SOURCES = mbsegyinfo.c

mbsegygrid_SOURCES = mbsegygrid.c
mbsegygrid_LDADD = $(top_builddir)/src/mbaux/libmbaux.la

mbauvnavusbl_SOURCES = mbauvnavusbl.c
mbauvnavusbl_LDADD = $(top_builddir)/src/mbaux/libmbaux.la

mbgetesf_SOURCES = mbgetesf.c

mbneptune2esf_SOURCES = mbneptune2esf.c

mbps_SOURCES = mbps.c
mbps_LDADD = ${libgmt_LIBS} -lmbps -lpsl -lgmtps

mbstripnan_SOURCES = mbstripnan.c

mb7k2jstar_SOURCES = mb7k2jstar.c

mb7kpreprocess_SOURCES = mb7kpreprocess.c
mb7kpreprocess_LDADD = $(top_builddir)/src/mbaux/libmbaux.la

mbrolltimelag_SOURCES = mbrolltimelag.c

mbauvloglist_SOURCES = mbauvloglist.c

mb7k2ss_SOURCES = mb7k2ss.c
mb7k2ss_LDADD = $(top_builddir)/src/mbaux/libmbaux.la

mbabsorption_SOURCES = mbabsorption.c

mbdumpesf_SOURCES = mbdumpesf.c

mbtime_SOURCES = mbtime.c

mbctdlist_SOURCES = mbctdlist.c
mbctdlist_LDADD = $(top_builddir)/src/mbaux/libmbaux.la

mbfilter_SOURCES = mbfilter.c

mbroutetime_SOURCES = mbroutetime.c

mbhysweeppreprocess_SOURCES = mbhysweeppreprocess.c
mbhysweeppreprocess_LDADD = $(top_builddir)/src/mbaux/libmbaux.la

mbkongsbergpreprocess_SOURCES = mbkongsbergpreprocess.c
mbkongsbergpreprocess_LDADD = $(top_builddir)/src/mbaux/libmbaux.la

if BUILD_FFTW
mbsegypsd_SOURCES = mbsegypsd.c
mbsegypsd_CFLAGS = ${libgmt_CFLAGS} ${libfftw_CFLAGS}
mbsegypsd_LDADD = ${libgmt_LIBS} ${libfftw_LIBS} $(top_builddir)/src/mbaux/libmbaux.la
endif

BUILT_SOURCES = levitus.h
CLEANFILES = levitus.h
DISTCLEANFILES = levitus.h

levitus.h: Makefile
	echo 'char *ifile = "$(mbsystemdatadir)/LevitusAnnual82.dat";' >$@
