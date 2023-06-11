#
# Gets variables from root CMakeLists
#

message("in macros")

set(executables mbm_arc2grd mbm_bpr mbm_copy
    mbm_fmtvel mbm_grd2arc mbm_grd2geovrml mbm_grd3dplot
    mbm_grdcut mbm_grdinfo mbm_grdplot mbm_grdtiff
    mbm_grid mbm_histplot mbm_makedatalist mbm_makesvp
    mbm_multicopy mbm_multidatalist mbm_multiprocess
    mbm_plot mbm_rollerror mbm_route2mission mbm_stat
    mbm_trnplot mbm_utm mbm_xbt mbm_xyplot)


## foreach(executable ${executables})
##  add_executable(${executable}
##                 ${executable})

install(PROGRAMS ${executables}
        DESTINATION ${CMAKE_INSTALL_BINDIR})
	
