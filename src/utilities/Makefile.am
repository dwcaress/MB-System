if BUILD_FFTW
 FFTW_PROG = mbsegypsd
endif

bin_PROGRAMS = mb7k2jstar \
        mb7k2ss \
        mb7kpreprocess \
        mbabsorption \
        mbareaclean \
        mbauvloglist \
        mbauvnavusbl \
        mbbackangle \
        mbclean \
        mbcopy \
        mbctdlist \
        mbdatalist \
        mbdefaults \
        mbdumpesf \
        mbextractsegy \
        mbfilter \
        mbformat \
        mbgetesf \
        mbgrid \
        mbhistogram \
        mbhsdump \
        mbhysweeppreprocess \
        mbinfo \
        mbinsreprocess \
        mbkongsbergpreprocess \
        mblevitus \
        mblist \
        mbmosaic \
        mbnavlist \
        mbneptune2esf \
        mbpreprocess \
        mbprocess \
        mbrollbias \
        mbrolltimelag \
        mbroutetime \
        mbrphsbias \
        mbsegygrid \
        mbsegyinfo \
        mbsegylist \
        mbset \
        mbsslayout \
        mbstripnan \
        mbsvplist \
        mbsvpselect \
        mbswplspreprocess \
        mbtime \
        $(FFTW_PROG)


include_HEADERS = levitus.h

AM_CFLAGS = ${libgmt_CFLAGS} ${libnetcdf_CFLAGS}

AM_CPPFLAGS = -I${top_srcdir}/src/mbio \
	      -I${top_srcdir}/src/mbaux \
	      -I${top_srcdir}/src/gsf \
	      ${libgmt_CPPFLAGS} \
	      ${libnetcdf_CPPFLAGS} \
	      ${libfftw_CPPFLAGS}

AM_LDFLAGS =

LIBS =  ${top_builddir}/src/mbio/libmbio.la \
       	${libgmt_LIBS} ${libnetcdf_LIBS} ${libproj_LIBS} \
        $(LIBM)

mb7k2jstar_SOURCES = mb7k2jstar.c
mb7k2ss_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mb7k2ss_SOURCES = mb7k2ss.c
mb7kpreprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mb7kpreprocess_SOURCES = mb7kpreprocess.c
mbabsorption_SOURCES = mbabsorption.c
mbareaclean_SOURCES = mbareaclean.c
mbauvloglist_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbauvloglist_SOURCES = mbauvloglist.c
mbauvnavusbl_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbauvnavusbl_SOURCES = mbauvnavusbl.c
mbbackangle_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbbackangle_SOURCES = mbbackangle.c
mbclean_SOURCES = mbclean.c
mbcopy_LDADD = ${top_builddir}/src/gsf/libmbgsf.la
mbcopy_SOURCES = mbcopy.c
mbctdlist_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbctdlist_SOURCES = mbctdlist.c
mbdatalist_SOURCES = mbdatalist.c
mbdefaults_SOURCES = mbdefaults.c
mbdumpesf_SOURCES = mbdumpesf.c
mbextractsegy_SOURCES = mbextractsegy.c
mbfilter_SOURCES = mbfilter.c
mbformat_SOURCES = mbformat.c
mbgetesf_SOURCES = mbgetesf.c
mbgrid_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbgrid_SOURCES = mbgrid.c
mbhistogram_SOURCES = mbhistogram.c
mbhsdump_SOURCES = mbhsdump.c
mbhysweeppreprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbhysweeppreprocess_SOURCES = mbhysweeppreprocess.c
mbinfo_SOURCES = mbinfo.c
mbinsreprocess_SOURCES = mbinsreprocess.c
mbkongsbergpreprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbkongsbergpreprocess_SOURCES = mbkongsbergpreprocess.c
mblevitus_SOURCES = mblevitus.c
mblist_SOURCES = mblist.c
mbmosaic_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbmosaic_SOURCES = mbmosaic.c
mbnavlist_SOURCES = mbnavlist.c
mbneptune2esf_SOURCES = mbneptune2esf.c
mbpreprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbpreprocess_SOURCES = mbpreprocess.c
mbprocess_LDADD = ${top_builddir}/src/gsf/libmbgsf.la ${top_builddir}/src/mbaux/libmbaux.la
mbprocess_SOURCES = mbprocess.c
mbrollbias_SOURCES = mbrollbias.c
mbrolltimelag_SOURCES = mbrolltimelag.c
mbroutetime_SOURCES = mbroutetime.c
mbrphsbias_SOURCES = mbrphsbias.c
mbsegygrid_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbsegygrid_SOURCES = mbsegygrid.c
mbsegyinfo_SOURCES = mbsegyinfo.c
mbsegylist_SOURCES = mbsegylist.c
mbset_SOURCES = mbset.c
mbsslayout_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbsslayout_SOURCES = mbsslayout.c
mbstripnan_SOURCES = mbstripnan.c
mbsvplist_SOURCES = mbsvplist.c
mbsvpselect_SOURCES = mbsvpselect.c geodesic.c
mbswplspreprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbswplspreprocess_SOURCES = mbswplspreprocess.c
mbtime_SOURCES = mbtime.c
if BUILD_FFTW
mbsegypsd_LDADD = ${top_builddir}/src/mbaux/libmbaux.la \
                  ${libgmt_LIBS} ${libnetcdf_LIBS} ${libproj_LIBS} ${libfftw_LIBS}
mbsegypsd_SOURCES = mbsegypsd.c
mbsegypsd_LDFLAGS =
endif

BUILT_SOURCES = levitus.h
CLEANFILES = levitus.h
DISTCLEANFILES = levitus.h

levitus.h: Makefile
	echo 'char *ifile = "$(mbsystemdatadir)/LevitusAnnual82.dat";' >$@
