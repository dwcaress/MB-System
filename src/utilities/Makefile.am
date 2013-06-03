if BUILD_FFTW
 FFTW_PROG = mbsegypsd
endif


bin_PROGRAMS = mbformat \
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

#Removed hsdump
#Remove mbps if WIN32?
#Add mbprofilepsd to FFTW_PROG in future?


include_HEADERS = levitus.h

AM_CPPFLAGS = -I@top_srcdir@/src -I@top_srcdir@/src/mbio -I@top_srcdir@/src/mbaux -I@top_srcdir@/src/surf -I@top_srcdir@/src/gsf @CDF_INC_DIR@ @GMT_INC_DIR@ @FFTW_INC_DIR@
LIBS = -L@top_srcdir@/src/mbio -L@top_srcdir@/src/gsf -L@top_srcdir@/src/proj -L@top_srcdir@/src/mr1pr -L@top_srcdir@/src/surf ${libnetcdf_LIBS} ${libgmt_LIBS}
AM_CFLAGS = ${libnetcdf_CFLAGS} ${libgmt_CFLAGS}

mbformat_SOURCES = mbformat.c
mbformat_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbdefaults_SOURCES = mbdefaults.c
mbdefaults_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbdatalist_SOURCES = mbdatalist.c
mbdatalist_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbinfo_SOURCES = mbinfo.c
mbinfo_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbset_SOURCES = mbset.c
mbset_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbprocess_SOURCES = mbprocess.c
mbprocess_LDADD = -L@top_srcdir@/src/mbaux \
	${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux

mbcopy_SOURCES = mbcopy.c
mbcopy_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mblist_SOURCES = mblist.c
mblist_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbnavlist_SOURCES = mbnavlist.c
mbnavlist_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbsvplist_SOURCES = mbsvplist.c
mbsvplist_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbbackangle_SOURCES = mbbackangle.c
mbbackangle_LDADD = -L@top_srcdir@/src/mbaux \
	${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux

mbclean_SOURCES = mbclean.c
mbclean_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbareaclean_SOURCES = mbareaclean.c
mbareaclean_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbhistogram_SOURCES = mbhistogram.c
mbhistogram_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbgrid_SOURCES = mbgrid.c
mbgrid_LDADD = -L@top_srcdir@/src/mbaux \
	${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux

mbmosaic_SOURCES = mbmosaic.c
mbmosaic_LDADD = -L@top_srcdir@/src/mbaux \
	${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux

mblevitus_SOURCES = mblevitus.c
mblevitus_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbrollbias_SOURCES = mbrollbias.c
mbrollbias_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbextractsegy_SOURCES = mbextractsegy.c
mbextractsegy_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbsegylist_SOURCES = mbsegylist.c
mbsegylist_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbsegyinfo_SOURCES = mbsegyinfo.c
mbsegyinfo_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbsegygrid_SOURCES = mbsegygrid.c
mbsegygrid_LDADD = -L@top_srcdir@/src/mbaux \
	${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux

mbauvnavusbl_SOURCES = mbauvnavusbl.c
mbauvnavusbl_LDADD = -L@top_srcdir@/src/mbaux \
	${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux

mbgetesf_SOURCES = mbgetesf.c
mbgetesf_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbneptune2esf_SOURCES = mbneptune2esf.c
mbneptune2esf_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbps_SOURCES = mbps.c
mbps_LDADD = -L@top_srcdir@/src/mbaux \
	${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbps -lpsl -lgmtps

mbstripnan_SOURCES = mbstripnan.c
mbstripnan_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mb7k2jstar_SOURCES = mb7k2jstar.c
mb7k2jstar_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mb7kpreprocess_SOURCES = mb7kpreprocess.c
mb7kpreprocess_LDADD = -L@top_srcdir@/src/mbaux \
	${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux

mbrolltimelag_SOURCES = mbrolltimelag.c
mbrolltimelag_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbauvloglist_SOURCES = mbauvloglist.c
mbauvloglist_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mb7k2ss_SOURCES = mb7k2ss.c
mb7k2ss_LDADD = -L@top_srcdir@/src/mbaux \
	${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux

mbabsorption_SOURCES = mbabsorption.c
mbabsorption_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbdumpesf_SOURCES = mbdumpesf.c
mbdumpesf_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbtime_SOURCES = mbtime.c
mbtime_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbctdlist_SOURCES = mbctdlist.c
mbctdlist_LDADD = -L@top_srcdir@/src/mbaux \
	${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux

mbfilter_SOURCES = mbfilter.c
mbfilter_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbroutetime_SOURCES = mbroutetime.c
mbroutetime_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbhysweeppreprocess_SOURCES = mbhysweeppreprocess.c
mbhysweeppreprocess_LDADD = -L@top_srcdir@/src/mbaux -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux

mbkongsbergpreprocess_SOURCES = mbkongsbergpreprocess.c
mbkongsbergpreprocess_LDADD = -L@top_srcdir@/src/mbaux -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux

if BUILD_FFTW
mbsegypsd_SOURCES = mbsegypsd.c
mbsegypsd_CFLAGS = ${libgmt_CFLAGS} ${libfftw_CFLAGS} -L@top_srcdir@/src/mbaux
mbsegypsd_LDADD = ${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi \
	-lmbaux ${libfftw_LIBS}

## Removed mbprofilepsd -- does not build??
# mbprofilepsd_SOURCES = mbprofilepsd.c
# mbprofilepsd_CFLAGS = ${libfftw_CFLAGS} -L@top_srcdir@/src/mbaux
# mbprofilepsd_LDADD = ${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi \
#	-lmbaux ${libfftw_LIBS}
endif

BUILT_SOURCES = levitus.h
CLEANFILES = levitus.h
DISTCLEANFILES = levitus.h

levitus.h: Makefile
	echo 'char *ifile = "$(mbsystemdatadir)/LevitusAnnual82.dat";' >$@
