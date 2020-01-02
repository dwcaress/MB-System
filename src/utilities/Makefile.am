if BUILD_FFTW
 FFTW_PROG = mbsegypsd
endif

if BUILD_MBTRN
  MBTRNINCDIR = -I${top_srcdir}/src/mbtrn/r7kr -I${top_srcdir}/src/mbtrn/utils -I${top_srcdir}/src/mbtrn/mframe/src
endif

bin_PROGRAMS =
bin_PROGRAMS += mbconfig
bin_PROGRAMS += mbabsorption
bin_PROGRAMS += mbareaclean
bin_PROGRAMS += mbauvloglist
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
bin_PROGRAMS += mbinfo
bin_PROGRAMS += mblevitus
bin_PROGRAMS += mblist
bin_PROGRAMS += mbmakeplatform
bin_PROGRAMS += mbminirovnav
bin_PROGRAMS += mbmosaic
bin_PROGRAMS += mbnavlist
bin_PROGRAMS += mbpreprocess
bin_PROGRAMS += mbprocess
bin_PROGRAMS += mbrolltimelag
bin_PROGRAMS += mbroutetime
bin_PROGRAMS += mbsegygrid
bin_PROGRAMS += mbsegyinfo
bin_PROGRAMS += mbsegylist
bin_PROGRAMS += mbset
bin_PROGRAMS += mbsslayout
bin_PROGRAMS += mbsvplist
bin_PROGRAMS += mbsvpselect
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

mbconfig_SOURCES = mbconfig.cc
# mb7k2jstar_SOURCES = mb7k2jstar.cc
# mb7k2ss_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
# mb7k2ss_SOURCES = mb7k2ss.cc
# mb7kpreprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
# mb7kpreprocess_SOURCES = mb7kpreprocess.cc
mbabsorption_SOURCES = mbabsorption.cc
mbareaclean_SOURCES = mbareaclean.cc
mbauvloglist_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbauvloglist_SOURCES = mbauvloglist.cc
# mbauvnavusbl_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
# mbauvnavusbl_SOURCES = mbauvnavusbl.cc
mbbackangle_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbbackangle_SOURCES = mbbackangle.cc
mbclean_SOURCES = mbclean.cc
if BUILD_GSF
mbcopy_LDADD = ${top_builddir}/src/gsf/libmbgsf.la
endif
mbcopy_SOURCES = mbcopy.cc
mbctdlist_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbctdlist_SOURCES = mbctdlist.cc
mbdatalist_SOURCES = mbdatalist.cc
mbdefaults_SOURCES = mbdefaults.cc
mbdumpesf_SOURCES = mbdumpesf.cc
mbextractsegy_SOURCES = mbextractsegy.cc
mbfilter_SOURCES = mbfilter.cc
mbformat_SOURCES = mbformat.cc
mbgetesf_SOURCES = mbgetesf.cc
mbgpstide_SOURCES = mbgpstide.cc
mbgrid_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbgrid_SOURCES = mbgrid.cc
mbhistogram_SOURCES = mbhistogram.cc
# mbhsdump_SOURCES = mbhsdump.cc
# mbhysweeppreprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
# mbhysweeppreprocess_SOURCES = mbhysweeppreprocess.cc
mbinfo_SOURCES = mbinfo.cc
# mbinsreprocess_SOURCES = mbinsreprocess.cc
# mbkongsbergpreprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
# mbkongsbergpreprocess_SOURCES = mbkongsbergpreprocess.cc
mblevitus_SOURCES = mblevitus.cc
mblist_SOURCES = mblist.cc
mbmakeplatform_SOURCES = mbmakeplatform.cc
mbminirovnav_SOURCES = mbminirovnav.cc
mbmosaic_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbmosaic_SOURCES = mbmosaic.cc
mbnavlist_SOURCES = mbnavlist.cc
# mbneptune2esf_SOURCES = mbneptune2esf.cc
mbpreprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbpreprocess_SOURCES = mbpreprocess.cc
mbprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbprocess_SOURCES = mbprocess.cc
# mbrollbias_SOURCES = mbrollbias.cc
mbrolltimelag_SOURCES = mbrolltimelag.cc
mbroutetime_SOURCES = mbroutetime.cc
# mbrphsbias_SOURCES = mbrphsbias.cc
mbsegygrid_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbsegygrid_SOURCES = mbsegygrid.cc
mbsegyinfo_SOURCES = mbsegyinfo.cc
mbsegylist_SOURCES = mbsegylist.cc
mbset_SOURCES = mbset.cc
mbsslayout_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbsslayout_SOURCES = mbsslayout.cc
# mbstripnan_SOURCES = mbstripnan.cc
mbsvplist_SOURCES = mbsvplist.cc
mbsvpselect_SOURCES = mbsvpselect.cc geodesic.c
# mbswplspreprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
# mbswplspreprocess_SOURCES = mbswplspreprocess.cc
mbtime_SOURCES = mbtime.cc
mbvoxelclean_SOURCES = mbvoxelclean.cc
if BUILD_FFTW
mbsegypsd_LDADD =
mbsegypsd_LDADD += ${top_builddir}/src/mbaux/libmbaux.la
mbsegypsd_LDADD += ${libgmt_LIBS}
mbsegypsd_LDADD += ${libnetcdf_LIBS}
mbsegypsd_LDADD += ${libproj_LIBS}
mbsegypsd_LDADD += ${libfftw_LIBS}
mbsegypsd_SOURCES = mbsegypsd.cc
mbsegypsd_LDFLAGS =
endif

BUILT_SOURCES = levitus.h
CLEANFILES = levitus.h
DISTCLEANFILES = levitus.h

levitus.h: Makefile
	echo 'char *levitusfile = "$(mbsystemdatadir)/LevitusAnnual82.dat";' >$@
	echo 'char *otps_location = "$(OTPS_DIR)";' >>$@
