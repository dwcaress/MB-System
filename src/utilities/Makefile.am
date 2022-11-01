if BUILD_FFTW
  FFTW_PROG = mbsegypsd
endif
if BUILD_MBSVPSELECT
  MBSVPSELECT_PROG = mbsvpselect
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
bin_PROGRAMS += $(MBSVPSELECT_PROG)
bin_PROGRAMS += mbswath2las
bin_PROGRAMS += mbtime
bin_PROGRAMS += mbtransmitpattern
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
AM_CPPFLAGS += ${libproj_CPPFLAGS}
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
mbabsorption_SOURCES = mbabsorption.cc
mbareaclean_SOURCES = mbareaclean.cc
mbauvloglist_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbauvloglist_SOURCES = mbauvloglist.cc
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
mbinfo_SOURCES = mbinfo.cc
mblevitus_SOURCES = mblevitus.cc
mblist_SOURCES = mblist.cc
mbmakeplatform_SOURCES = mbmakeplatform.cc
mbminirovnav_SOURCES = mbminirovnav.cc
mbmosaic_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbmosaic_SOURCES = mbmosaic.cc
mbnavlist_SOURCES = mbnavlist.cc
mbpreprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbpreprocess_SOURCES = mbpreprocess.cc
mbprocess_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbprocess_SOURCES = mbprocess.cc
mbrolltimelag_SOURCES = mbrolltimelag.cc
mbroutetime_SOURCES = mbroutetime.cc
mbsegygrid_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbsegygrid_SOURCES = mbsegygrid.cc
mbsegyinfo_SOURCES = mbsegyinfo.cc
mbsegylist_SOURCES = mbsegylist.cc
mbset_SOURCES = mbset.cc
mbsslayout_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbsslayout_SOURCES = mbsslayout.cc
mbsvplist_SOURCES = mbsvplist.cc
if BUILD_MBSVPSELECT
mbsvpselect_SOURCES = mbsvpselect.cc
endif
mbswath2las_SOURCES = mbswath2las.cc
mbtime_SOURCES = mbtime.cc
mbtransmitpattern_LDADD = ${top_builddir}/src/mbaux/libmbaux.la
mbtransmitpattern_SOURCES = mbtransmitpattern.cc
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
	echo 'const char *levitusfile = "$(mbsystemdatadir)/LevitusAnnual82.dat";' >$@
	echo 'const char *otps_location = "$(OTPS_DIR)";' >>$@
