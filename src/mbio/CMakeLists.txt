if(CMAKE_SYSTEM_NAME STREQUAL Linux)
  find_package(TIRPC REQUIRED)
else()
  add_library(TIRPC::TIRPC INTERFACE IMPORTED)
endif()

find_package(LibPROJ REQUIRED)
find_package(NetCDF REQUIRED)

set(SRC
    mb_absorption.c
    mb_access.c
    mb_angle.c
    mb_buffer.c
    mb_check_info.c
    mb_close.c
    mb_compare.c
    mb_coor_scale.c
    mb_defaults.c
    mb_error.c
    mb_esf.c
    mb_fileio.c
    mb_format.c
    mb_get.c
    mb_get_all.c
    mb_get_value.c
    mb_mem.c
    mb_navint.c
    mb_platform.c
    mb_platform_math.c
    mb_process.c
    mb_proj.c
    mb_put_all.c
    mb_put_comment.c
    mb_read.c
    mb_read_init.c
    mb_read_ping.c
    mb_rt.c
    mb_segy.c
    mb_spline.c
    mb_swap.c
    mb_time.c
    mb_write_init.c
    mb_write_ping.c
    mbr_3ddepthp.c
    mbr_3dwisslp.c
    mbr_3dwisslr.c
    mbr_asciixyz.c
    mbr_bchrtunb.c
    mbr_bchrxunb.c
    mbr_cbat8101.c
    mbr_cbat9001.c
    mbr_dsl120pf.c
    mbr_dsl120sf.c
    mbr_edgjstar.c
    mbr_elmk2unb.c
    mbr_em12darw.c
    mbr_em12ifrm.c
    mbr_em300mba.c
    mbr_em300raw.c
    mbr_em710mba.c
    mbr_em710raw.c
    mbr_emoldraw.c
    mbr_hir2rnav.c
    mbr_hs10jams.c
    mbr_hsatlraw.c
    mbr_hsds2lam.c
    mbr_hsds2raw.c
    mbr_hsldedmb.c
    mbr_hsldeoih.c
    mbr_hsmdaraw.c
    mbr_hsmdldih.c
    mbr_hsunknwn.c
    mbr_hsuricen.c
    mbr_hsurivax.c
    mbr_hydrob93.c
    mbr_hypc8101.c
    mbr_hysweep1.c
    mbr_image83p.c
    mbr_imagemba.c
    mbr_kemkmall.c
    mbr_l3xseraw.c
    mbr_mbarimb1.c
    mbr_mbarirov.c
    mbr_mbarrov2.c
    mbr_mbldeoih.c
    mbr_mbnetcdf.c
    mbr_mbpronav.c
    mbr_mgd77dat.c
    mbr_mgd77tab.c
    mbr_mgd77txt.c
    mbr_mr1aldeo.c
    mbr_mr1bldeo.c
    mbr_mr1prhig.c
    mbr_mr1prvr2.c
    mbr_mstiffss.c
    mbr_nvnetcdf.c
    mbr_oicgeoda.c
    mbr_oicmbari.c
    mbr_omghdcsj.c
    mbr_photgram.c
    mbr_reson7k3.c
    mbr_reson7kr.c
    mbr_samesurf.c
    mbr_sb2000sb.c
    mbr_sb2000ss.c
    mbr_sb2100bi.c
    mbr_sb2100rw.c
    mbr_sbifremr.c
    mbr_sbsiocen.c
    mbr_sbsiolsi.c
    mbr_sbsiomrg.c
    mbr_sbsioswb.c
    mbr_sburicen.c
    mbr_sburivax.c
    mbr_segysegy.c
    mbr_soirovnv.c
    mbr_soiusbln.c
    mbr_swplssxi.c
    mbr_swplssxp.c
    mbr_wasspenl.c
    mbr_xtfb1624.c
    mbr_xtfr8101.c
    mbsys_3datdepthlidar.c
    mbsys_3ddwissl.c
    mbsys_atlas.c
    mbsys_benthos.c
    mbsys_dsl.c
    mbsys_elac.c
    mbsys_elacmk2.c
    mbsys_hdcs.c
    mbsys_hs10.c
    mbsys_hsds.c
    mbsys_hsmd.c
    mbsys_hysweep.c
    mbsys_image83p.c
    mbsys_jstar.c
    mbsys_kmbes.c
    mbsys_ldeoih.c
    mbsys_mr1.c
    mbsys_mr1b.c
    mbsys_mr1v2001.c
    mbsys_mstiff.c
    mbsys_navnetcdf.c
    mbsys_netcdf.c
    mbsys_oic.c
    mbsys_reson.c
    mbsys_reson7k.c
    mbsys_reson7k3.c
    mbsys_reson8k.c
    mbsys_sb.c
    mbsys_sb2000.c
    mbsys_sb2100.c
    mbsys_simrad.c
    mbsys_simrad2.c
    mbsys_simrad3.c
    mbsys_singlebeam.c
    mbsys_stereopair.c
    mbsys_surf.c
    mbsys_swathplus.c
    mbsys_wassp.c
    mbsys_xse.c)

if(buildGSF)
  set(SRC ${SRC} mbsys_gsf.c mbr_gsfgenmb.c)
endif()

add_library(mbio ${SRC})
set_target_properties(mbio PROPERTIES VERSION "0" SOVERSION "0")
if(LibPROJ_VERSION VERSION_LESS 5)
  target_compile_definitions(
    mbio PRIVATE USE_PROJ4_API=1
                 PROJECTIONS_DAT="${CMAKE_INSTALL_DATADIR}/Projections.dat")
endif()
target_compile_definitions(mbio PUBLIC MB_PACKAGE_VERSION="${PROJECT_VERSION}"
                                       MB_PACKAGE_DATE="${PROJECT_DATE}")
target_include_directories(mbio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if(buildGSF)
  target_link_libraries(mbio PRIVATE mbgsf)
endif()
target_link_libraries(
  mbio
  PRIVATE NetCDF::NetCDF mbbsio mbsapi LibPROJ::LibPROJ NetCDF::NetCDF
  PUBLIC TIRPC::TIRPC m)

install(TARGETS mbio DESTINATION ${CMAKE_INSTALL_LIBDIR})
