if BUILD_FFTW
 FFTW_PROG = mbsegypsd
endif


bin_PROGRAMS = hsdump \
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

AM_CPPFLAGS = -I@top_srcdir@/src/mbio -I@top_srcdir@/src/mbaux -I@top_srcdir@/src/gsf @CDF_INC_DIR@ @GMT_INC_DIR@ @FFTW_INC_DIR@
LIBS = ${libnetcdf_LIBS} ${libgmt_LIBS} -L@top_srcdir@/src/mbio -L@top_srcdir@/src/mbaux -lmbio
AM_CFLAGS = ${libnetcdf_CFLAGS} ${libgmt_CFLAGS}

hsdump_SOURCES = hsdump.c
hsdump_LDADD =

mbformat_SOURCES = mbformat.c
mbformat_LDADD =

mbdefaults_SOURCES = mbdefaults.c
mbdefaults_LDADD =

mbdatalist_SOURCES = mbdatalist.c
mbdatalist_LDADD =

mbinfo_SOURCES = mbinfo.c
mbinfo_LDADD =

mbset_SOURCES = mbset.c
mbset_LDADD =

mbprocess_SOURCES = mbprocess.c
mbprocess_LDADD = -lmbaux

mbcopy_SOURCES = mbcopy.c
mbcopy_LDADD =

mblist_SOURCES = mblist.c
mblist_LDADD =

mbnavlist_SOURCES = mbnavlist.c
mbnavlist_LDADD =

mbsvplist_SOURCES = mbsvplist.c
mbsvplist_LDADD =

mbbackangle_SOURCES = mbbackangle.c
mbbackangle_LDADD = -lmbaux

mbclean_SOURCES = mbclean.c
mbclean_LDADD =

mbareaclean_SOURCES = mbareaclean.c
mbareaclean_LDADD =

mbhistogram_SOURCES = mbhistogram.c
mbhistogram_LDADD =

mbgrid_SOURCES = mbgrid.c
mbgrid_LDADD = -lmbaux

mbmosaic_SOURCES = mbmosaic.c
mbmosaic_LDADD = -lmbaux

mblevitus_SOURCES = mblevitus.c
mblevitus_LDADD =

mbrollbias_SOURCES = mbrollbias.c
mbrollbias_LDADD =

mbextractsegy_SOURCES = mbextractsegy.c
mbextractsegy_LDADD =

mbsegylist_SOURCES = mbsegylist.c
mbsegylist_LDADD =

mbsegyinfo_SOURCES = mbsegyinfo.c
mbsegyinfo_LDADD =

mbsegygrid_SOURCES = mbsegygrid.c
mbsegygrid_LDADD = -lmbaux

mbauvnavusbl_SOURCES = mbauvnavusbl.c
mbauvnavusbl_LDADD = -lmbaux

mbgetesf_SOURCES = mbgetesf.c
mbgetesf_LDADD =

mbneptune2esf_SOURCES = mbneptune2esf.c
mbneptune2esf_LDADD =

mbps_SOURCES = mbps.c
mbps_LDADD = ${libgmt_LIBS} -lmbps -lpsl -lgmtps

mbstripnan_SOURCES = mbstripnan.c
mbstripnan_LDADD =

mb7k2jstar_SOURCES = mb7k2jstar.c
mb7k2jstar_LDADD =

mb7kpreprocess_SOURCES = mb7kpreprocess.c
mb7kpreprocess_LDADD = -lmbaux

mbrolltimelag_SOURCES = mbrolltimelag.c
mbrolltimelag_LDADD =

mbauvloglist_SOURCES = mbauvloglist.c
mbauvloglist_LDADD =

mb7k2ss_SOURCES = mb7k2ss.c
mb7k2ss_LDADD = -lmbaux

mbabsorption_SOURCES = mbabsorption.c
mbabsorption_LDADD =

mbdumpesf_SOURCES = mbdumpesf.c
mbdumpesf_LDADD =

mbtime_SOURCES = mbtime.c
mbtime_LDADD =

mbctdlist_SOURCES = mbctdlist.c
mbctdlist_LDADD = -lmbaux

mbfilter_SOURCES = mbfilter.c
mbfilter_LDADD =

mbroutetime_SOURCES = mbroutetime.c
mbroutetime_LDADD =

mbhysweeppreprocess_SOURCES = mbhysweeppreprocess.c
mbhysweeppreprocess_LDADD = -lmbaux

mbkongsbergpreprocess_SOURCES = mbkongsbergpreprocess.c
mbkongsbergpreprocess_LDADD = -lmbaux

if BUILD_FFTW
mbsegypsd_SOURCES = mbsegypsd.c
mbsegypsd_CFLAGS = ${libgmt_CFLAGS} ${libfftw_CFLAGS}
mbsegypsd_LDADD = ${libgmt_LIBS} ${libfftw_LIBS} -lmbaux
endif

BUILT_SOURCES = levitus.h
CLEANFILES = levitus.h
DISTCLEANFILES = levitus.h

levitus.h: Makefile
	echo 'char *ifile = "$(mbsystemdatadir)/LevitusAnnual82.dat";' >$@
