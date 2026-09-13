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

message("In src/macros")

set(scripts
    mbm_arc2grd
    mbm_bpr
    mbm_copy
    mbm_grd2arc
    mbm_grd3dplot
    mbm_grdcut
    mbm_grdinfo
    mbm_grdplot
    mbm_grdtiff
    mbm_histplot
    mbm_makedatalist
    mbm_makesvp
    mbm_multicopy
    mbm_multidatalist
    mbm_multiprocess
    mbm_phins2fnv
    mbm_plot
    mbm_rollerror
    mbm_route2mission
    mbm_stat
    mbm_tmap
    mbm_trnplot
    mbm_utm
    mbm_xbt
    mbm_xyplot
    mbpy_grdplot
    mbpy_grd3dplot
    mbpy_histplot
    mbpy_xyplot)

install(PROGRAMS ${scripts} DESTINATION ${CMAKE_INSTALL_BINDIR})

# The mbpy_* macros (Python/PyGMT-era ports of mbm_grdplot, mbm_grd3dplot,
# and mbm_histplot) share a common set of support modules (page-layout,
# projection, and color-palette logic; misc-option modeling) plus one
# script-generator module per tool -- see src/macros/python/*.py.
# Installed as plain files (not executables) alongside the mbpy_*
# scripts themselves, matching the flat, no-packaging layout the Perl
# scripts above already use; each mbpy_* tool locates its sibling
# modules at runtime relative to its own installed location.
set(mbpy_modules
    python/mbpy_layout.py
    python/mbpy_color.py
    python/mbpy_misc.py
    python/mbpy_grdplot_script.py
    python/mbpy_grd3dplot_script.py
    python/mbpy_histplot_script.py
    python/mbpy_xyplot_script.py)

install(FILES ${mbpy_modules} DESTINATION ${CMAKE_INSTALL_BINDIR})
