#------------------------------------------------------------------------------
#    The MB-system:  CMakeLists.txt   27 June 2023
#
#    Copyright (c) 2023-2024 by
#    David W. Caress (caress@mbari.org)
#      Monterey Bay Aquarium Research Institute
#      Moss Landing, California, USA
#    Dale N. Chayes
#      Center for Coastal and Ocean Mapping
#      University of New Hampshire
#      Durham, New Hampshire, USA
#    Christian dos Santos Ferreira
#      MARUM
#      University of Bremen
#      Bremen Germany
#
#    MB-System was created by Caress and Chayes in 1992 at the
#      Lamont-Doherty Earth Observatory
#      Columbia University
#      Palisades, NY 10964
#
#    See README.md file for copying and redistribution conditions.
#------------------------------------------------------------------------------

message("In src/html")

set(HTML
    ChangeLog.html
    COPYING.html
    GPL.html
    index.html
    mbsystem_logo_small.gif
    mbsystem_logo.gif
    mbsystem_version.txt
    mbsystem_man_list.html
    mbsystem_formats.html
    mbsystem.html
    mbio.html
    mb7k2jstar.html
    mb7k2ss.html
    mb7kpreprocess.html
    mbabsorption.html
    mbareaclean.html
    mbauvloglist.html
    mbauvnavusbl.html
    mbbackangle.html
    mbclean.html
    mbconfig.html
    mbcontour.html
    mbcopy.html
    mbctdlist.html
    mbdatalist.html
    mbdefaults.html
    mbdumpesf.html
    mbedit.html
    mbeditviz.html
    mbextractsegy.html
    mbfilter.html
    mbformat.html
    mbgetesf.html
    mbgpstide.html
	mbgrd2gltf.html
    mbgrd2obj.html
    mbgrdtiff.html
    mbgrdviz.html
    mbgrid.html
    mbhistogram.html
    mbhsdump.html
    mbhysweeppreprocess.html
    mbinfo.html
    mbkongsbergpreprocess.html
    mblevitus.html
    mblist.html
    mbm_arc2grd.html
    mbm_bpr.html
    mbm_copy.html
    mbm_fmtvel.html
    mbm_grd2arc.html
    mbm_grd2geovrml.html
    mbm_grd3dplot.html
    mbm_grdcut.html
    mbm_grdinfo.html
    mbm_grdplot.html
    mbm_grdtiff.html
    mbm_grid.html
    mbm_histplot.html
    mbm_makedatalist.html
    mbmakeplatform.html
    mbm_makesvp.html
    mbm_multicopy.html
    mbm_multidatalist.html
    mbm_multiprocess.html
    mbm_plot.html
    mbm_route2mission.html
    mbm_stat.html
    mbm_utm.html
    mbm_xbt.html
    mbm_xyplot.html
    mbmosaic.html
    mbnavadjust.html
    mbnavadjustmerge.html
    mbnavedit.html
    mbnavlist.html
    mbneptune2esf.html
    mbotps.html
    mbpreprocess.html
    mbprocess.html
    mbps.html
    mbrollbias.html
    mbrolltimelag.html
    mbroutetime.html
    mbsegygrid.html
    mbsegyinfo.html
    mbsegylist.html
    mbsegypsd.html
    mbset.html
    mbsslayout.html
    mbstripnan.html
    mbsvplist.html
    mbsvpselect.html
    mbswath.html
    mbswplspreprocess.html
    mbtime.html
    mbvelocitytool.html
    mbvoxelclean.html)

set(PDF
    ChangeLog.pdf
    GPL.pdf
    COPYING.pdf
    mbsystem.pdf
    mbio.pdf
    mb7k2jstar.pdf
    mb7k2ss.pdf
    mb7kpreprocess.pdf
    mbabsorption.pdf
    mbareaclean.pdf
    mbauvloglist.pdf
    mbauvnavusbl.pdf
    mbbackangle.pdf
    mbclean.pdf
    mbconfig.pdf
    mbcontour.pdf
    mbcopy.pdf
    mbctdlist.pdf
    mbdatalist.pdf
    mbdefaults.pdf
    mbdumpesf.pdf
    mbedit.pdf
    mbeditviz.pdf
    mbextractsegy.pdf
    mbfilter.pdf
    mbformat.pdf
    mbgetesf.pdf
    mbgpstide.pdf
	mbgrd2gltf.pdf
    mbgrd2obj.pdf
    mbgrdtiff.pdf
    mbgrdviz.pdf
    mbgrid.pdf
    mbhistogram.pdf
    mbhsdump.pdf
    mbhysweeppreprocess.pdf
    mbinfo.pdf
    mbkongsbergpreprocess.pdf
    mblevitus.pdf
    mblist.pdf
    mbmakeplatform.pdf
    mbm_arc2grd.pdf
    mbm_bpr.pdf
    mbm_copy.pdf
    mbm_fmtvel.pdf
    mbm_grd2arc.pdf
    mbm_grd2geovrml.pdf
    mbm_grd3dplot.pdf
    mbm_grdcut.pdf
    mbm_grdinfo.pdf
    mbm_grdplot.pdf
    mbm_grdtiff.pdf
    mbm_grid.pdf
    mbm_histplot.pdf
    mbm_makedatalist.pdf
    mbm_makesvp.pdf
    mbm_multicopy.pdf
    mbm_multidatalist.pdf
    mbm_multiprocess.pdf
    mbm_plot.pdf
    mbm_route2mission.pdf
    mbm_stat.pdf
    mbm_utm.pdf
    mbm_xbt.pdf
    mbm_xyplot.pdf
    mbmosaic.pdf
    mbnavadjust.pdf
    mbnavadjustmerge.pdf
    mbnavedit.pdf
    mbnavlist.pdf
    mbneptune2esf.pdf
    mbotps.pdf
    mbpreprocess.pdf
    mbprocess.pdf
    mbps.pdf
    mbrollbias.pdf
    mbrolltimelag.pdf
    mbroutetime.pdf
    mbsegygrid.pdf
    mbsegyinfo.pdf
    mbsegylist.pdf
    mbsegypsd.pdf
    mbset.pdf
    mbsslayout.pdf
    mbstripnan.pdf
    mbsvplist.pdf
    mbsvpselect.pdf
    mbswath.pdf
    mbswplspreprocess.pdf
    mbtime.pdf
    mbvelocitytool.pdf
    mbvoxelclean.pdf)

install(FILES ${HTML} ${PDF} DESTINATION ${CMAKE_INSTALL_DOC}/mbsystem/html)
