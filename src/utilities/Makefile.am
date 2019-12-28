if BUILD_FFTW
 FFTW_PROG = mbsegypsd
endif

if BUILD_MBTRN
  MBTRNINCDIR = -I${top_srcdir}/src/mbtrn/r7kr -I${top_srcdir}/src/mbtrn/utils -I${top_srcdir}/src/mbtrn/mframe/src
endif

bin_PROGRAMS =
bin_PROGRAMS += mbconfig
bin_PROGRAMS += mb7k2jstar
bin_PROGRAMS += mb7k2ss
bin_PROGRAMS += mb7kpreprocess
bin_PROGRAMS += mbabsorption
bin_PROGRAMS += mbareaclean
bin_PROGRAMS += mbauvloglist
bin_PROGRAMS += mbauvnavusbl
bin_PROGRAMS += mbbackangle
bin_PROGRAMS += mbclean
bin_PROGRAMS += mbcopy
bin_PROGRAMS += mbctdlist
bin_PROGRAMS += mbdatalist
bin_PROGRAMS += mbdefaults
bin_PROGRAMS += mbdumpesf
bin_PROGRAMS += mbextractsegy
bin_PROGRAMS += mbfilter
bin_PROGRAMS += mbformat
bin_PROGRAMS += mbgetesf
bin_PROGRAMS += mbgpstide
bin_PROGRAMS += mbgrid
bin_PROGRAMS += mbhistogram
bin_PROGRAMS += mbhsdump
bin_PROGRAMS += mbhysweeppreprocess
bin_PROGRAMS += mbinfo
bin_PROGRAMS += mbinsreprocess
bin_PROGRAMS += mbkongsbergpreprocess
bin_PROGRAMS += mblevitus
bin_PROGRAMS += mblist
bin_PROGRAMS += mbmakeplatform
bin_PROGRAMS += mbminirovnav
bin_PROGRAMS += mbmosaic
bin_PROGRAMS += mbnavlist
bin_PROGRAMS += mbneptune2esf
bin_PROGRAMS += mbpreprocess
bin_PROGRAMS += mbprocess
bin_PROGRAMS += mbrollbias
bin_PROGRAMS += mbrolltimelag
bin_PROGRAMS += mbroutetime
bin_PROGRAMS += mbrphsbias
bin_PROGRAMS += mbsegygrid
bin_PROGRAMS += mbsegyinfo
bin_PROGRAMS += mbsegylist
bin_PROGRAMS += mbset
bin_PROGRAMS += mbsslayout
bin_PROGRAMS += mbstripnan
bin_PROGRAMS += mbsvplist
bin_PROGRAMS += mbsvpselect
bin_PROGRAMS += mbswplspreprocess
bin_PROGRAMS += mbtime
bin_PROGRAMS += mbvoxelclean
bin_PROGRAMS += $(FFTW_PROG)
bin_PROGRAMS += $(MBTRN_PROG)

include_HEADERS = levitus.h

AM_CFLAGS = ${libgmt_CFLAGS} ${libnetcdf_CFLAGS}

AM_CPPFLAGS =
AM_CPPFLAGS += -I${top_srcdir}/src/mbio
AM_CPPFLAGS += -I${top_srcdir}/src/mbaux
if BUILD_GSF
AM_CPPFLAGS += -I${top_srcdir}/src/gsf
endif
AM_CPPFLAGS += $(MBTRNINCDIR)
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

mbconfig_SOURCES = mbconfig.c
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
if BUILD_GSF
mbcopy_LDADD = ${top_builddir}/src/gsf/libmbgsf.la
endif
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
mbgpstide_SOURCES = mbgpstide.c
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
mbmakeplatform_SOURCES = mbmakeplatform.c
mbminirovnav_SOURCES = mbminirovnav.c
mbmosaic_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbmosaic_SOURCES = mbmosaic.c
mbnavlist_SOURCES = mbnavlist.c
mbneptune2esf_SOURCES = mbneptune2esf.c
mbpreprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbpreprocess_SOURCES = mbpreprocess.c
mbprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
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
mbvoxelclean_SOURCES = mbvoxelclean.c
if BUILD_FFTW
mbsegypsd_LDADD =
mbsegypsd_LDADD += ${top_builddir}/src/mbaux/libmbaux.la
mbsegypsd_LDADD += ${libgmt_LIBS}
mbsegypsd_LDADD += ${libnetcdf_LIBS}
mbsegypsd_LDADD += ${libproj_LIBS}
mbsegypsd_LDADD += ${libfftw_LIBS}
mbsegypsd_SOURCES = mbsegypsd.c
mbsegypsd_LDFLAGS =
endif

BUILT_SOURCES = levitus.h
CLEANFILES = levitus.h
DISTCLEANFILES = levitus.h

levitus.h: Makefile
	echo 'char *levitusfile = "$(mbsystemdatadir)/LevitusAnnual82.dat";' >$@
	echo 'char *otps_location = "$(OTPS_DIR)";' >>$@
