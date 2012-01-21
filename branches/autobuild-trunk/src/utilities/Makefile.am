if BUILD_FFTW
 FFTW_PROG = mbsegypsd
endif
bin_PROGRAMS = mb7k2jstar mb7k2ss mb7kpreprocess mbabsorption mbareaclean \
mbauvloglist mbauvnavusbl mbbackangle mbclean mbcopy mbctdlist mbdatalist \
mbdefaults mbdumpesf mbextractsegy mbfilter mbformat mbgetesf mbgrid \
mbhistogram mbps mbinfo mblevitus mblist mbmosaic mbnavlist mbneptune2esf \
mbprocess mbrollbias mbrolltimelag mbroutetime mbsegygrid mbsegyinfo \
mbsegylist mbset mbsm20002gsf mbstripnan mbsvplist mbtime $(FFTW_PROG)

#Removed hsdump
#Remove mbps if WIN32?
#Add mbprofilepsd to FFTW_PROG in future?


include_HEADERS = levitus.h

INCLUDES = -I.. -I../mbio -I../mbaux -I../surf -I../gsf @CDF_INC_DIR@ @GMT_INC_DIR@ @FFTW_INC_DIR@
LIBS = -L../mbio -L../gsf -L../proj -L../mr1pr -L../surf ${libnetcdf_LIBS} ${libgmt_LIBS}
AM_CFLAGS = ${libnetcdf_CFLAGS} ${libgmt_CFLAGS}  

mb7k2jstar_SOURCES = mb7k2jstar.c
mb7k2jstar_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mb7k2ss_SOURCES = mb7k2ss.c
mb7k2ss_LDADD = -L../mbaux \
	${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux

mb7kpreprocess_SOURCES = mb7kpreprocess.c
mb7kpreprocess_LDADD = -L../mbaux \
	${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux 

mbabsorption_SOURCES = mbabsorption.c
mbabsorption_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbareaclean_SOURCES = mbareaclean.c
mbareaclean_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbauvloglist_SOURCES = mbauvloglist.c
mbauvloglist_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbauvnavusbl_SOURCES = mbauvnavusbl.c
mbauvnavusbl_LDADD = -L../mbaux \
	${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux

mbbackangle_SOURCES = mbbackangle.c
mbbackangle_LDADD = -L../mbaux \
	${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux

mbclean_SOURCES = mbclean.c
mbclean_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbcopy_SOURCES = mbcopy.c
mbcopy_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbctdlist_SOURCES = mbctdlist.c
mbctdlist_LDADD = -L../mbaux \
	${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux

mbdatalist_SOURCES = mbdatalist.c
mbdatalist_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbdefaults_SOURCES = mbdefaults.c
mbdefaults_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbdumpesf_SOURCES = mbdumpesf.c
mbdumpesf_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbextractsegy_SOURCES = mbextractsegy.c
mbextractsegy_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbfilter_SOURCES = mbfilter.c
mbfilter_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbformat_SOURCES = mbformat.c
mbformat_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbgetesf_SOURCES = mbgetesf.c
mbgetesf_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbgrid_SOURCES = mbgrid.c
mbgrid_LDADD = -L../mbaux \
	${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux

mbhistogram_SOURCES = mbhistogram.c
mbhistogram_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbinfo_SOURCES = mbinfo.c
mbinfo_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mblevitus_SOURCES = mblevitus.c
mblevitus_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mblist_SOURCES = mblist.c
mblist_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbmosaic_SOURCES = mbmosaic.c
mbmosaic_LDADD = -L../mbaux \
	${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux

mbnavlist_SOURCES = mbnavlist.c
mbnavlist_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbneptune2esf_SOURCES = mbneptune2esf.c
mbneptune2esf_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbprocess_SOURCES = mbprocess.c
mbprocess_LDADD = -L../mbaux \
	${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux

mbps_SOURCES = mbps.c
mbps_LDADD = -L../mbaux \
	${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbps

mbrollbias_SOURCES = mbrollbias.c
mbrollbias_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbrolltimelag_SOURCES = mbrolltimelag.c
mbrolltimelag_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbroutetime_SOURCES = mbroutetime.c
mbroutetime_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbsegygrid_SOURCES = mbsegygrid.c
mbsegygrid_LDADD = -L../mbaux \
	${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux

mbsegyinfo_SOURCES = mbsegyinfo.c
mbsegyinfo_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbsegylist_SOURCES = mbsegylist.c
mbsegylist_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbset_SOURCES = mbset.c
mbset_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbsm20002gsf_SOURCES = mbsm20002gsf.c
mbsm20002gsf_LDADD = -L../mbaux \
	${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux

mbstripnan_SOURCES = mbstripnan.c
mbstripnan_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbsvplist_SOURCES = mbsvplist.c
mbsvplist_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

mbtime_SOURCES = mbtime.c
mbtime_LDADD = -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi

if BUILD_FFTW
mbsegypsd_SOURCES = mbsegypsd.c
mbsegypsd_CFLAGS = ${libfftw_CFLAGS} -L../mbaux
mbsegypsd_LDADD = ${libgmt_LIBS} -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi \
	-lmbaux ${libfftw_LIBS}

#mbprofilepsd_SOURCES = mbprofilepsd.c
#mbprofilepsd_LDADD = -L../mbaux \
#		     ${libgmt_LIBS} @FFTW_LIB_DIR@ -lmbio -lmbproj -lmbgsf -lmbmr1pr \
#		     -lmbsapi -lmbaux @FFTW_LIB@
endif

