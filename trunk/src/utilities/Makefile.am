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
        mbrphsbias \
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
        mbsxppreprocess \
        mbpreprocess \
        $(FFTW_PROG)

#Remove mbps if WIN32?

include_HEADERS = levitus.h

AM_CFLAGS = ${libgmt_CFLAGS} ${libnetcdf_CFLAGS}

AM_CPPFLAGS = -I${top_srcdir}/src/mbio \
	      -I${top_srcdir}/src/mbaux \
	      ${libgmt_CPPFLAGS} \
	      ${libnetcdf_CPPFLAGS} \
	      ${libfftw_CPPFLAGS}
if BUILD_GSF
  AM_CPPFLAGS += -I${top_srcdir}/src/gsf
endif

AM_LDFLAGS =

LIBS =  ${top_builddir}/src/mbio/libmbio.la \
       	${libgmt_LIBS} ${libnetcdf_LIBS} ${libproj_LIBS} \
        $(LIBM)

mbhsdump_SOURCES = mbhsdump.c

mbformat_SOURCES = mbformat.c

mbdefaults_SOURCES = mbdefaults.c

mbdatalist_SOURCES = mbdatalist.c

mbinfo_SOURCES = mbinfo.c

mbset_SOURCES = mbset.c

mbprocess_SOURCES = mbprocess.c
mbprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la

mbcopy_SOURCES = mbcopy.c
if BUILD_GSF
  mbcopy_LDADD = ${top_builddir}/src/gsf/libmbgsf.la
endif

mblist_SOURCES = mblist.c

mbnavlist_SOURCES = mbnavlist.c

mbsvplist_SOURCES = mbsvplist.c

mbbackangle_SOURCES = mbbackangle.c
mbbackangle_LDADD = ${top_builddir}/src/mbaux/libmbaux.la

mbclean_SOURCES = mbclean.c

mbareaclean_SOURCES = mbareaclean.c

mbhistogram_SOURCES = mbhistogram.c

mbgrid_SOURCES = mbgrid.c
mbgrid_LDADD = ${top_builddir}/src/mbaux/libmbaux.la

mbmosaic_SOURCES = mbmosaic.c
mbmosaic_LDADD = ${top_builddir}/src/mbaux/libmbaux.la

mblevitus_SOURCES = mblevitus.c

mbrphsbias_SOURCES = mbrphsbias.c

mbrollbias_SOURCES = mbrollbias.c

mbextractsegy_SOURCES = mbextractsegy.c

mbsegylist_SOURCES = mbsegylist.c

mbsegyinfo_SOURCES = mbsegyinfo.c

mbsegygrid_SOURCES = mbsegygrid.c
mbsegygrid_LDADD = ${top_builddir}/src/mbaux/libmbaux.la

mbauvnavusbl_SOURCES = mbauvnavusbl.c
mbauvnavusbl_LDADD = ${top_builddir}/src/mbaux/libmbaux.la

mbgetesf_SOURCES = mbgetesf.c

mbneptune2esf_SOURCES = mbneptune2esf.c

mbps_SOURCES = mbps.c
mbps_LDADD = ${top_builddir}/src/mbaux/libmbps.la ${libgmt_LIBS} -lpsl -lgmtps

mbstripnan_SOURCES = mbstripnan.c

mb7k2jstar_SOURCES = mb7k2jstar.c

mb7kpreprocess_SOURCES = mb7kpreprocess.c
mb7kpreprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la

mbrolltimelag_SOURCES = mbrolltimelag.c

mbauvloglist_SOURCES = mbauvloglist.c

mb7k2ss_SOURCES = mb7k2ss.c
mb7k2ss_LDADD = ${top_builddir}/src/mbaux/libmbaux.la

mbabsorption_SOURCES = mbabsorption.c

mbdumpesf_SOURCES = mbdumpesf.c

mbtime_SOURCES = mbtime.c

mbctdlist_SOURCES = mbctdlist.c
mbctdlist_LDADD = ${top_builddir}/src/mbaux/libmbaux.la

mbfilter_SOURCES = mbfilter.c

mbroutetime_SOURCES = mbroutetime.c

mbhysweeppreprocess_SOURCES = mbhysweeppreprocess.c
mbhysweeppreprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la

mbkongsbergpreprocess_SOURCES = mbkongsbergpreprocess.c
mbkongsbergpreprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la

mbsxppreprocess_SOURCES = mbsxppreprocess.c
mbsxppreprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la

mbpreprocess_SOURCES = mbpreprocess.c
mbpreprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la

if BUILD_FFTW
mbsegypsd_SOURCES = mbsegypsd.c
mbsegypsd_LDFLAGS =

mbsegypsd_LDADD = ${top_builddir}/src/mbaux/libmbaux.la \
                  ${libgmt_LIBS} ${libnetcdf_LIBS} ${libproj_LIBS} ${libfftw_LIBS}
endif

BUILT_SOURCES = levitus.h
CLEANFILES = levitus.h
DISTCLEANFILES = levitus.h

levitus.h: Makefile
	echo 'char *ifile = "$(mbsystemdatadir)/LevitusAnnual82.dat";' >$@
