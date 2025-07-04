lib_LTLIBRARIES = libmbio.la

if BUILD_MBTRN
  MBTRNINCDIR = -I${top_srcdir}/src/mbtrn/r7kr -I${top_srcdir}/src/mbtrn/utils -I${top_srcdir}/src/mbtrnframe
  MBTRNLIB = ${top_builddir}/src/mbtrn/libr7kr.la
endif

include_HEADERS =
include_HEADERS += mb_config.h
include_HEADERS += mb_define.h
include_HEADERS += mb_format.h
include_HEADERS += mb_info.h
include_HEADERS += mb_io.h
include_HEADERS += mb_process.h
include_HEADERS += mb_segy.h
include_HEADERS += mb_status.h
include_HEADERS += mb_swap.h
include_HEADERS += mbf_bchrtunb.h
include_HEADERS += mbf_bchrxunb.h
include_HEADERS += mbf_cbat8101.h
include_HEADERS += mbf_cbat9001.h
include_HEADERS += mbf_dsl120pf.h
include_HEADERS += mbf_dsl120sf.h
include_HEADERS += mbf_elmk2unb.h
include_HEADERS += mbf_em12darw.h
include_HEADERS += mbf_em12ifrm.h
include_HEADERS += mbf_hsatlraw.h
include_HEADERS += mbf_hsldedmb.h
include_HEADERS += mbf_hsldeoih.h
include_HEADERS += mbf_hsmdaraw.h
include_HEADERS += mbf_hsmdldih.h
include_HEADERS += mbf_hsuricen.h
include_HEADERS += mbf_hypc8101.h
include_HEADERS += mbf_mbarirov.h
include_HEADERS += mbf_mbarrov2.h
include_HEADERS += mbf_mbpronav.h
include_HEADERS += mbf_mgd77dat.h
include_HEADERS += mbf_mr1aldeo.h
include_HEADERS += mbf_mr1bldeo.h
include_HEADERS += mbf_mr1prhig.h
include_HEADERS += mbf_mstiffss.h
include_HEADERS += mbf_oicgeoda.h
include_HEADERS += mbf_oicmbari.h
include_HEADERS += mbf_omghdcsj.h
include_HEADERS += mbf_sb2100rw.h
include_HEADERS += mbf_sbifremr.h
include_HEADERS += mbf_sbsiocen.h
include_HEADERS += mbf_sbsiolsi.h
include_HEADERS += mbf_sbsiomrg.h
include_HEADERS += mbf_sbsioswb.h
include_HEADERS += mbf_sburicen.h
include_HEADERS += mbf_xtfr8101.h
include_HEADERS += mbsys_3datdepthlidar.h
include_HEADERS += mbsys_3ddwissl1.h
include_HEADERS += mbsys_3ddwissl2.h
include_HEADERS += mbsys_atlas.h
include_HEADERS += mbsys_benthos.h
include_HEADERS += mbsys_dsl.h
include_HEADERS += mbsys_hdcs.h
include_HEADERS += mbsys_hs10.h
include_HEADERS += mbsys_hsds.h
include_HEADERS += mbsys_hsmd.h
include_HEADERS += mbsys_hysweep.h
include_HEADERS += mbsys_image83p.h
include_HEADERS += mbsys_jstar.h
include_HEADERS += mbsys_kmbes.h
include_HEADERS += mbsys_ldeoih.h
include_HEADERS += mbsys_mr1b.h
include_HEADERS += mbsys_mr1.h
include_HEADERS += mbsys_mr1v2001.h
include_HEADERS += mbsys_mstiff.h
include_HEADERS += mbsys_navnetcdf.h
include_HEADERS += mbsys_netcdf.h
include_HEADERS += mbsys_oic.h
include_HEADERS += mbsys_reson7k3.h
include_HEADERS += mbsys_reson7k.h
include_HEADERS += mbsys_reson8k.h
include_HEADERS += mbsys_reson.h
include_HEADERS += mbsys_sb2000.h
include_HEADERS += mbsys_sb2100.h
include_HEADERS += mbsys_sb.h
include_HEADERS += mbsys_simrad2.h
include_HEADERS += mbsys_simrad3.h
include_HEADERS += mbsys_simrad.h
include_HEADERS += mbsys_singlebeam.h
include_HEADERS += mbsys_stereopair.h
include_HEADERS += mbsys_surf.h
include_HEADERS += mbsys_swathplus.h
include_HEADERS += mbsys_wassp.h
include_HEADERS += mbsys_xse.h
if BUILD_GSF
include_HEADERS += mbf_gsfgenmb.h
include_HEADERS += mbsys_gsf.h
endif

AM_CFLAGS = ${libgmt_CFLAGS} ${libnetcdf_CFLAGS}

AM_CPPFLAGS =
AM_CPPFLAGS += -I${top_srcdir}/src/mbaux
AM_CPPFLAGS += -I@top_srcdir@/src/bsio
AM_CPPFLAGS += -I@top_srcdir@/src/surf
AM_CPPFLAGS += -I@top_srcdir@/src/mbbitpack
AM_CPPFLAGS += $(MBTRNINCDIR)
if BUILD_GSF
AM_CPPFLAGS += -I@top_srcdir@/src/gsf
endif
AM_CPPFLAGS += ${libgmt_CPPFLAGS}
AM_CPPFLAGS += ${libnetcdf_CPPFLAGS}
AM_CPPFLAGS += ${libproj_CPPFLAGS}

libmbio_la_LDFLAGS = -no-undefined -version-info 0:0:0

libmbio_la_SOURCES =
libmbio_la_SOURCES += mb_absorption.c
libmbio_la_SOURCES += mb_access.c
libmbio_la_SOURCES += mb_angle.c
libmbio_la_SOURCES += mb_buffer.c
libmbio_la_SOURCES += mb_check_info.c
libmbio_la_SOURCES += mb_close.c
libmbio_la_SOURCES += mb_compare.c
libmbio_la_SOURCES += mb_coor_scale.c
libmbio_la_SOURCES += mb_defaults.c
libmbio_la_SOURCES += mb_error.c
libmbio_la_SOURCES += mb_esf.c
libmbio_la_SOURCES += mb_fileio.c
libmbio_la_SOURCES += mb_format.c
libmbio_la_SOURCES += mb_get_all.c
libmbio_la_SOURCES += mb_get.c
libmbio_la_SOURCES += mb_get_value.c
libmbio_la_SOURCES += mb_mem.c
libmbio_la_SOURCES += mb_navint.c
libmbio_la_SOURCES += mb_platform.c
libmbio_la_SOURCES += mb_platform_math.c
libmbio_la_SOURCES += mb_process.c
libmbio_la_SOURCES += mb_proj.c
libmbio_la_SOURCES += mb_put_all.c
libmbio_la_SOURCES += mb_put_comment.c
libmbio_la_SOURCES += mb_read.c
libmbio_la_SOURCES += mb_read_init.c
libmbio_la_SOURCES += mb_read_ping.c
libmbio_la_SOURCES += mb_rt.c
libmbio_la_SOURCES += mb_segy.c
libmbio_la_SOURCES += mb_spline.c
libmbio_la_SOURCES += mb_swap.c
libmbio_la_SOURCES += mb_time.c
libmbio_la_SOURCES += mb_write_init.c
libmbio_la_SOURCES += mb_write_ping.c
libmbio_la_SOURCES += mbr_3ddepthp.c
libmbio_la_SOURCES += mbr_3dwisslp.c
libmbio_la_SOURCES += mbr_3dwisslr.c
libmbio_la_SOURCES += mbr_3dwissl2.c
libmbio_la_SOURCES += mbr_asciixyz.c
libmbio_la_SOURCES += mbr_bchrtunb.c
libmbio_la_SOURCES += mbr_bchrxunb.c
libmbio_la_SOURCES += mbr_cbat8101.c
libmbio_la_SOURCES += mbr_cbat9001.c
libmbio_la_SOURCES += mbr_dsl120pf.c
libmbio_la_SOURCES += mbr_dsl120sf.c
libmbio_la_SOURCES += mbr_edgjstar.c
libmbio_la_SOURCES += mbr_elmk2unb.c
libmbio_la_SOURCES += mbr_em12darw.c
libmbio_la_SOURCES += mbr_em12ifrm.c
libmbio_la_SOURCES += mbr_em300mba.c
libmbio_la_SOURCES += mbr_em300raw.c
libmbio_la_SOURCES += mbr_em710mba.c
libmbio_la_SOURCES += mbr_em710raw.c
libmbio_la_SOURCES += mbr_emoldraw.c
libmbio_la_SOURCES += mbr_hir2rnav.c
libmbio_la_SOURCES += mbr_hs10jams.c
libmbio_la_SOURCES += mbr_hsatlraw.c
libmbio_la_SOURCES += mbr_hsds2lam.c
libmbio_la_SOURCES += mbr_hsds2raw.c
libmbio_la_SOURCES += mbr_hsldedmb.c
libmbio_la_SOURCES += mbr_hsldeoih.c
libmbio_la_SOURCES += mbr_hsmdaraw.c
libmbio_la_SOURCES += mbr_hsmdldih.c
libmbio_la_SOURCES += mbr_hsunknwn.c
libmbio_la_SOURCES += mbr_hsuricen.c
libmbio_la_SOURCES += mbr_hsurivax.c
libmbio_la_SOURCES += mbr_hydrob93.c
libmbio_la_SOURCES += mbr_hypc8101.c
libmbio_la_SOURCES += mbr_hysweep1.c
libmbio_la_SOURCES += mbr_image83p.c
libmbio_la_SOURCES += mbr_imagemba.c
libmbio_la_SOURCES += mbr_kemkmall.c
libmbio_la_SOURCES += mbr_l3xseraw.c
libmbio_la_SOURCES += mbr_mbarirov.c
libmbio_la_SOURCES += mbr_mbarrov2.c
libmbio_la_SOURCES += mbr_mbldeoih.c
libmbio_la_SOURCES += mbr_mbarimb1.c
libmbio_la_SOURCES += mbr_mbnetcdf.c
libmbio_la_SOURCES += mbr_mbpronav.c
libmbio_la_SOURCES += mbr_mgd77dat.c
libmbio_la_SOURCES += mbr_mgd77tab.c
libmbio_la_SOURCES += mbr_mgd77txt.c
libmbio_la_SOURCES += mbr_mr1aldeo.c
libmbio_la_SOURCES += mbr_mr1bldeo.c
libmbio_la_SOURCES += mbr_mr1prhig.c
libmbio_la_SOURCES += mbr_mr1prvr2.c
libmbio_la_SOURCES += mbr_mstiffss.c
libmbio_la_SOURCES += mbr_nvnetcdf.c
libmbio_la_SOURCES += mbr_oicgeoda.c
libmbio_la_SOURCES += mbr_oicmbari.c
libmbio_la_SOURCES += mbr_omghdcsj.c
libmbio_la_SOURCES += mbr_photgram.c
libmbio_la_SOURCES += mbr_reson7k3.c
libmbio_la_SOURCES += mbr_reson7kr.c
libmbio_la_SOURCES += mbr_samesurf.c
libmbio_la_SOURCES += mbr_sb2000sb.c
libmbio_la_SOURCES += mbr_sb2000ss.c
libmbio_la_SOURCES += mbr_sb2100bi.c
libmbio_la_SOURCES += mbr_sb2100rw.c
libmbio_la_SOURCES += mbr_sbifremr.c
libmbio_la_SOURCES += mbr_sbsiocen.c
libmbio_la_SOURCES += mbr_sbsiolsi.c
libmbio_la_SOURCES += mbr_sbsiomrg.c
libmbio_la_SOURCES += mbr_sbsioswb.c
libmbio_la_SOURCES += mbr_sburicen.c
libmbio_la_SOURCES += mbr_sburivax.c
libmbio_la_SOURCES += mbr_segysegy.c
libmbio_la_SOURCES += mbr_soirovnv.c
libmbio_la_SOURCES += mbr_soiusbln.c
libmbio_la_SOURCES += mbr_swplssxi.c
libmbio_la_SOURCES += mbr_swplssxp.c
libmbio_la_SOURCES += mbr_wasspenl.c
libmbio_la_SOURCES += mbr_xtfb1624.c
libmbio_la_SOURCES += mbr_xtfr8101.c
libmbio_la_SOURCES += mbsys_3datdepthlidar.c
libmbio_la_SOURCES += mbsys_3ddwissl1.c
libmbio_la_SOURCES += mbsys_3ddwissl2.c
libmbio_la_SOURCES += mbsys_atlas.c
libmbio_la_SOURCES += mbsys_benthos.c
libmbio_la_SOURCES += mbsys_dsl.c
libmbio_la_SOURCES += mbsys_elac.c
libmbio_la_SOURCES += mbsys_elacmk2.c
libmbio_la_SOURCES += mbsys_hdcs.c
libmbio_la_SOURCES += mbsys_hs10.c
libmbio_la_SOURCES += mbsys_hsds.c
libmbio_la_SOURCES += mbsys_hsmd.c
libmbio_la_SOURCES += mbsys_hysweep.c
libmbio_la_SOURCES += mbsys_image83p.c
libmbio_la_SOURCES += mbsys_jstar.c
libmbio_la_SOURCES += mbsys_kmbes.c
libmbio_la_SOURCES += mbsys_ldeoih.c
libmbio_la_SOURCES += mbsys_mr1b.c
libmbio_la_SOURCES += mbsys_mr1.c
libmbio_la_SOURCES += mbsys_mr1v2001.c
libmbio_la_SOURCES += mbsys_mstiff.c
libmbio_la_SOURCES += mbsys_navnetcdf.c
libmbio_la_SOURCES += mbsys_netcdf.c
libmbio_la_SOURCES += mbsys_oic.c
libmbio_la_SOURCES += mbsys_reson7k3.c
libmbio_la_SOURCES += mbsys_reson7k.c
libmbio_la_SOURCES += mbsys_reson8k.c
libmbio_la_SOURCES += mbsys_reson.c
libmbio_la_SOURCES += mbsys_sb2000.c
libmbio_la_SOURCES += mbsys_sb2100.c
libmbio_la_SOURCES += mbsys_sb.c
libmbio_la_SOURCES += mbsys_simrad2.c
libmbio_la_SOURCES += mbsys_simrad3.c
libmbio_la_SOURCES += mbsys_simrad.c
libmbio_la_SOURCES += mbsys_singlebeam.c
libmbio_la_SOURCES += mbsys_stereopair.c
libmbio_la_SOURCES += mbsys_surf.c
libmbio_la_SOURCES += mbsys_swathplus.c
libmbio_la_SOURCES += mbsys_wassp.c
libmbio_la_SOURCES += mbsys_xse.c
if BUILD_GSF
libmbio_la_SOURCES += mbr_gsfgenmb.c
libmbio_la_SOURCES += mbsys_gsf.c
endif

libmbio_la_LIBADD =
libmbio_la_LIBADD += $(top_builddir)/src/bsio/libmbbsio.la
libmbio_la_LIBADD += $(top_builddir)/src/surf/libmbsapi.la
libmbio_la_LIBADD += $(top_builddir)/src/mbbitpack/libmbbitpack.la
if BUILD_GSF
libmbio_la_LIBADD += $(top_builddir)/src/gsf/libmbgsf.la
endif
libmbio_la_LIBADD += ${libgmt_LIBS}
libmbio_la_LIBADD += ${libnetcdf_LIBS}
libmbio_la_LIBADD += ${libproj_LIBS}
libmbio_la_LIBADD += ${XDR_LIB}
libmbio_la_LIBADD += $(MBTRNLIB)
nodist_libmbio_la_SOURCES = projections.h

BUILT_SOURCES = projections.h

CLEANFILES = projections.h
DISTCLEANFILES = projections.h

projections.h: Makefile
	echo 'char *projectionfile = "$(mbsystemdatadir)/Projections.dat";' >$@
