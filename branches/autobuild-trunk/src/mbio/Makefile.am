AM_CPPFLAGS = -I.. -I../gsf -I../proj -I../mr1pr -I../surf @CDF_INC_DIR@ @GMT_INC_DIR@

AM_CFLAGS = ${libnetcdf_CFLAGS} ${libgmt_CFLAGS} -L../gsf -L../surf -L../mr1pr -L../proj


include_HEADERS = mb_format.h mb_status.h \
        mb_io.h mb_swap.h \
        mb_define.h mb_process.h \
        mb_segy.h mb_info.h \
        mbsys_sb.h mbsys_hsds.h \
        mbsys_sb2100.h mbsys_simrad.h \
        mbsys_simrad2.h mbsys_simrad3.h \
        mbsys_mr1.h mbsys_mr1b.h mbsys_mr1v2001.h \
        mbsys_ldeoih.h \
        mbsys_reson.h mbsys_sb2000.h \
        mbsys_hsmd.h mbsys_dsl.h \
        mbsys_gsf.h mbsys_mstiff.h \
        mbsys_oic.h mbsys_hdcs.h \
        mbsys_singlebeam.h mbsys_xse.h \
        mbsys_hs10.h \
        mbsys_atlas.h mbsys_surf.h \
        mbsys_reson8k.h \
        mbsys_netcdf.h mbsys_navnetcdf.h \
        mbsys_reson7k.h mbsys_jstar.h \
        mbsys_image83p.h mbsys_hysweep.h \
        mbsys_benthos.h \
        mbf_sbsiomrg.h mbf_sbsiocen.h \
        mbf_sbsiolsi.h mbf_sburicen.h \
        mbf_sbifremr.h mbf_hsatlraw.h \
        mbf_hsldedmb.h mbf_hsuricen.h \
        mbf_hsldeoih.h \
        mbf_sb2100rw.h \
        mbf_em12darw.h mbf_em12ifrm.h \
        mbf_mr1prhig.h mbf_mr1aldeo.h \
        mbf_mr1bldeo.h mbf_cbat9001.h \
        mbf_cbat8101.h mbf_hypc8101.h \
        mbf_bchrtunb.h mbf_elmk2unb.h \
        mbf_bchrxunb.h \
        mbf_sbsioswb.h \
        mbf_hsmdaraw.h mbf_hsmdldih.h \
        mbf_dsl120pf.h mbf_dsl120sf.h \
        mbf_gsfgenmb.h mbf_mstiffss.h \
        mbf_oicgeoda.h mbf_oicmbari.h \
        mbf_omghdcsj.h mbf_mgd77dat.h \
        mbf_mbarirov.h mbf_mbarrov2.h \
        mbf_mbpronav.h mbf_xtfr8101.h

lib_LTLIBRARIES = libmbio.la

libmbio_la_LDFLAGS = -no-undefined -version-info 0:0:0 

libmbio_la_SOURCES = \
		mb_format.c mb_error.c \
                mb_swap.c mb_compare.c mb_navint.c \
                mb_read_init.c mb_write_init.c mb_close.c \
                mb_fileio.c mb_mem.c \
                mb_coor_scale.c mb_check_info.c \
                mb_time.c mb_angle.c \
                mb_absorption.c mb_proj.c \
                mb_get_value.c mb_defaults.c \
                mb_process.c mb_esf.c \
                mb_get.c mb_get_all.c mb_read.c \
                mb_put_all.c mb_put_comment.c \
                mb_buffer.c \
                mb_read_ping.c mb_write_ping.c \
                mb_access.c mb_segy.c mb_rt.c \
                mbsys_sb.c mbsys_hsds.c \
                mbsys_sb2100.c mbsys_simrad.c \
                mbsys_simrad2.c mbsys_simrad3.c \
                mbsys_mr1.c mbsys_mr1b.c mbsys_mr1v2001.c \
                mbsys_ldeoih.c mbsys_reson.c \
                mbsys_elac.c mbsys_elacmk2.c \
                mbsys_sb2000.c mbsys_hsmd.c mbsys_dsl.c \
                mbsys_gsf.c mbsys_mstiff.c \
                mbsys_oic.c mbsys_hdcs.c \
                mbsys_singlebeam.c mbsys_xse.c \
                mbsys_hs10.c mbsys_atlas.c \
                mbsys_surf.c mbsys_reson8k.c \
                mbsys_netcdf.c mbsys_navnetcdf.c \
                mbsys_reson7k.c mbsys_jstar.c \
                mbsys_image83p.c mbsys_hysweep.c \
                mbsys_benthos.c \
                mbr_sbsiomrg.c mbr_sbsiocen.c \
                mbr_sbsiolsi.c mbr_sburicen.c \
                mbr_sburivax.c mbr_sbifremr.c \
                mbr_hsatlraw.c mbr_hsldedmb.c \
                mbr_hsuricen.c mbr_hsurivax.c \
                mbr_hsldeoih.c mbr_hsunknwn.c \
                mbr_sb2100rw.c mbr_sb2100bi.c \
                mbr_emoldraw.c mbr_em12darw.c \
                mbr_em12ifrm.c \
                mbr_em300raw.c mbr_em300mba.c \
                mbr_em710raw.c mbr_em710mba.c \
                mbr_mr1prhig.c mbr_mr1aldeo.c \
                mbr_mr1bldeo.c mbr_mr1prvr2.c \
                mbr_mbldeoih.c mbr_cbat9001.c \
                mbr_cbat8101.c mbr_hypc8101.c \
                mbr_bchrtunb.c mbr_elmk2unb.c \
                mbr_bchrxunb.c mbr_l3xseraw.c \
                mbr_sbsioswb.c \
                mbr_sb2000sb.c mbr_sb2000ss.c \
                mbr_hsmdaraw.c mbr_hsmdldih.c \
                mbr_dsl120pf.c mbr_dsl120sf.c \
                mbr_gsfgenmb.c mbr_mstiffss.c \
                mbr_oicgeoda.c mbr_oicmbari.c \
                mbr_omghdcsj.c mbr_mgd77dat.c \
                mbr_asciixyz.c mbr_hydrob93.c \
                mbr_mbarirov.c mbr_mbarrov2.c \
                mbr_mbpronav.c mbr_hs10jams.c \
                mbr_hsds2raw.c mbr_hsds2lam.c \
                mbr_xtfr8101.c \
                mbr_mbnetcdf.c mbr_nvnetcdf.c \
                mbr_samesurf.c mbr_reson7kr.c \
                mbr_edgjstar.c mbr_segysegy.c \
                mbr_image83p.c mbr_imagemba.c \
                mbr_hir2rnav.c mbr_hysweep1.c \
                mbr_xtfb1624.c


libmbio_la_LIBADD = ${libnetcdf_LIBS} ${libgmt_LIBS} @XDR_LIB@ \
		   -lmbgsf -lmbmr1pr -lmbsapi -lmbproj 

nodist_libmbio_la_SOURCES = projections.h

BUILT_SOURCES = projections.h

CLEANFILES = projections.h
DISTCLEANFILES = projections.h

projections.h: Makefile
	echo 'char *projectionfile = "$(mbsystemdatadir)/Projections.dat";' >$@
#	echo 'char *projectionfile = "$(datadir)/mbsystem/Projections.dat";' >$@


