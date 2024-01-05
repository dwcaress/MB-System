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

message("In src/ps")

set(PS
    mb7k2jstar.ps
    mb7k2ss.ps
    mb7kpreprocess.ps
    mbabsorption.ps
    mbareaclean.ps
    mbauvloglist.ps
    mbauvnavusbl.ps
    mbbackangle.ps
    mbclean.ps
    mbcontour.ps
    mbcopy.ps
    mbctdlist.ps
    mbdatalist.ps
    mbdefaults.ps
    mbdumpesf.ps
    mbedit.ps
    mbeditviz.ps
    mbextractsegy.ps
    mbfilter.ps
    mbformat.ps
    mbgetesf.ps
    mbgrdtiff.ps
    mbgrdviz.ps
    mbgrid.ps
    mbhistogram.ps
    mbhsdump.ps
    mbhysweeppreprocess.ps
    mbinfo.ps
    mbio.ps
    mbkongsbergpreprocess.ps
    mblevitus.ps
    mblist.ps
    mbm_arc2grd.ps
    mbm_bpr.ps
    mbm_copy.ps
    mbm_fmtvel.ps
    mbm_grd2arc.ps
    mbm_grd2geovrml.ps
    mbm_grd3dplot.ps
    mbm_grdcut.ps
    mbm_grdinfo.ps
    mbm_grdplot.ps
    mbm_grdtiff.ps
    mbm_grid.ps
    mbm_histplot.ps
    mbm_makedatalist.ps
    mbm_multicopy.ps
    mbm_multidatalist.ps
    mbm_multiprocess.ps
    mbm_plot.ps
    mbm_route2mission.ps
    mbm_stat.ps
    mbm_utm.ps
    mbm_xbt.ps
    mbm_xyplot.ps
    mbmosaic.ps
    mbnavadjust.ps
    mbnavadjustmerge.ps
    mbnavedit.ps
    mbnavlist.ps
    mbneptune2esf.ps
    mbotps.ps
    mbprocess.ps
    mbps.ps
    mbrollbias.ps
    mbrolltimelag.ps
    mbroutetime.ps
    mbsegygrid.ps
    mbsegyinfo.ps
    mbsegylist.ps
    mbsegypsd.ps
    mbset.ps
    mbstripnan.ps
    mbsvplist.ps
    mbswath.ps
    mbsystem.ps
    mbtime.ps
    mbvelocitytool.ps)

install(FILES ${PS} DESTINATION ${CMAKE_INSTALL_DOC}/mbsystem/ps)
