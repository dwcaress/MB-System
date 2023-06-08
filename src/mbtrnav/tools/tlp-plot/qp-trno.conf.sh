#!/usr/local/bin/bash
#/bin/bash

# Configuration for TRNIO data plots

# Variable Naming Convention:
# QP_ prefix denotes environment/variables that qplot depends upon (don't modify)
# QU_ prefix for user environment prevents naming conflicts (may be imported/exported, i.e. shared between files)
# QX_ prefix denotes local/application-specific variables (file scope)

# import shared environment, variables
source ${QP_PLOT_HOME}/qp-shared.conf.sh
QX_TRNO_TARGET=${QX_TRNO_TARGET:-"tbd"}

# session ID (used by local combiner job)
# QU_SESSION_ID="-`date +%s`

# define plot titles and image names
QU_TRNO_OFFSETS_PTITLE=${QU_PTITLE:-"TRN offsets"}
QU_TRNO_OFFSETS_STITLE=${QU_STITLE:-"\n${QX_TRNO_TARGET} : ${QU_DATA_SET_ID}"}
export QU_TRNO_OFFSETS_OIMG_NAME="trno-${QX_TRNO_TARGET}-ofs"
QU_OFILE_NAME="trno-${QX_TRNO_TARGET}-ofs"

QU_TRNO_NORTHINGS_PTITLE=${QU_PTITLE:-"TRN Northings (mmse vs nav)"}
QU_TRNO_NORTHINGS_STITLE=${QU_STITLE:-"\n${QX_TRNO_TARGET} : ${QU_DATA_SET_ID}"}
export QU_TRNO_NORTHINGS_OIMG_NAME="trno-${QX_TRNO_TARGET}-nor"
QU_OFILE_NAME="trno-${QX_TRNO_TARGET}-nor"

QU_TRNO_EASTINGS_PTITLE=${QU_PTITLE:-"TRN Eastings (mmse vs nav)"}
QU_TRNO_EASTINGS_STITLE=${QU_STITLE:-"\n${QX_TRNO_TARGET} : ${QU_DATA_SET_ID}"}
export QU_TRNO_EASTINGS_OIMG_NAME="trno-${QX_TRNO_TARGET}-eas"
QU_OFILE_NAME="trno-${QX_TRNO_TARGET}-eas"

# Define job names to use in the configuration
declare -a QU_KEYS=( "trno-${QX_TRNO_TARGET}-ofs" "trno-${QX_TRNO_TARGET}-nor" "trno-${QX_TRNO_TARGET}-eas" "comb-all" )

# Set time formats for data and plots
# time format strings conform to gnuplot syntax
# (see man pages: strftime/strptime, date)

# input time format (must match data files)
# e.g. 2019-11-13T22:52:29Z
QU_ITIME="%s" #%Y-%m-%dT%H:%M:%SZ"
# output time format (gnuplot plot x-axis)
QU_OTIME="%Y-%m-%dT%H:%M:%SZ"

#### BEGIN QPLOT CONFIGURATION VALUES ####

# Define plot jobs
QP_JOB_DEFS["${QU_KEYS[0]}"]="plot,${QU_OTERM},${QU_KEYS[0]}"
QP_JOB_DEFS["${QU_KEYS[1]}"]="plot,${QU_OTERM},${QU_KEYS[1]}"
QP_JOB_DEFS["${QU_KEYS[2]}"]="plot,${QU_OTERM},${QU_KEYS[2]}"
# local combiner job (e.g. make PDF containing plots in this configuration)
#QP_JOB_DEFS["${QU_KEYS[2]}"]="combine,png,plot-output/${QU_OFILE_NAME}${QU_SESSION_ID}.pdf,./plot-output/*png"

# use QP_JOB_ORDER to enable plot jobs and set job order
# NOTE: plot combiner job is last, since plots must be completed first
QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[0]}"
QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[1]}"
QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[2]}"
#QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[2]}"

# Plot configuration parameters
QU_KEY=${QU_KEYS[0]}
QP_OFILE_NAME["$QU_KEY"]="${QU_TRNO_OFFSETS_OIMG_NAME}"
QP_PTITLE["$QU_KEY"]="${QU_TRNO_OFFSETS_PTITLE}${QU_TRNO_OFFSETS_STITLE}"
QP_TFMT["$QU_KEY"]=${QU_ITIME}
QP_XTFMT["$QU_KEY"]=${QU_OTIME}
QP_ISTIME["$QU_KEY"]="Y"
QP_YFMT["$QU_KEY"]="%g" #${QU_OTIME}
QP_DSEP["$QU_KEY"]=","
#QP_KEY_FONT["$QU_KEY"]="arial"
#QP_KEY_SIZE["$QU_KEY"]="9"
#QP_KEY_MAX_COL["$QU_KEY"]="8"
#QP_TERM_FONT["$QU_KEY"]="arial"
#QP_TERM_SIZE["$QU_KEY"]="10"
QP_TERM_OSIZE["$QU_KEY"]=${QU_TERM_OSIZE[${QU_OTERM}]}
QP_XTITLE["$QU_KEY"]="Time (h:m:s)"
#QP_XRANGE_MIN["$QU_KEY"]="\"2019-11-10T00:00:00Z\""
#QP_XRANGE_MAX["$QU_KEY"]="\"2019-11-15T00:00:00Z\""
QP_YTITLE["$QU_KEY"]="Offset (m)"
#QP_YRANGE_MIN["$QU_KEY"]=6.5
#QP_YRANGE_MAX["$QU_KEY"]=8.5
#QP_X2TITLE["$QU_KEY"]=""
#QP_X2RANGE_MIN["$QU_KEY"]=0
#QP_X2RANGE_MAX["$QU_KEY"]=10
QP_Y2TITLE["$QU_KEY"]="covariance"
#QP_Y2RANGE_MIN["$QU_KEY"]=6.5
#QP_Y2RANGE_MAX["$QU_KEY"]=8.8
QP_XSCALE["$QU_KEY"]=1.0
QP_YSCALE["$QU_KEY"]=1.0
QP_XOFS["$QU_KEY"]=0.0
QP_YOFS["$QU_KEY"]=0.0
QP_PLOT_STYLE["$QU_KEY"]="linespoints" #lines
QP_POINTSIZE["$QU_KEY"]=${QU_POINT_SIZE}
QP_POINTTYPE["$QU_KEY"]=${QU_POINT_TYPE}
QP_POINTCOLOR["$QU_KEY"]=${QU_POINT_COLOR}
QP_INC_POINTTYPE["$QU_KEY"]="N"
QP_INC_POINTCOLOR["$QU_KEY"]="N"
QP_USE_LINETYPES["$QU_KEY"]="N"
QP_LINETYPE["$QU_KEY"]=1
QP_INC_LINETYPE["$QU_KEY"]="N"
#QP_LINE_TYPES["$QU_KEY"]="${QU_LINE_TYPE_DFL}"
QP_PLOT_SPECS["$QU_KEY"]="${QU_TRNO_CSV},${QU_BLUE},6,,1,7,x1y1,ofs.N"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_TRNO_CSV},${QU_ORANGE},6,,1,8,x1y1,ofs.E"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_TRNO_CSV},${QU_GRAY},6,,1,9,x1y1,ofs.Z"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_TRNO_CSV},${QU_YELLOW},6,,1,10,x1y2,cov.N(y2)"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_TRNO_CSV},${QU_GREEN},6,,1,11,x1y2,cov.E(y2)"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_TRNO_CSV},${QU_LTBLUE},6,,1,12,x1y2,cov.Z(y2)"

QU_KEY=${QU_KEYS[1]}
QP_OFILE_NAME["$QU_KEY"]="${QU_TRNO_NORTHINGS_OIMG_NAME}"
QP_PTITLE["$QU_KEY"]="${QU_TRNO_NORTHINGS_PTITLE}${QU_TRNO_NORTHINGS_STITLE}"
QP_TFMT["$QU_KEY"]=${QU_ITIME}
QP_XTFMT["$QU_KEY"]=${QU_OTIME}
QP_ISTIME["$QU_KEY"]="Y"
QP_YFMT["$QU_KEY"]="%g" #${QU_OTIME}
QP_DSEP["$QU_KEY"]=","
#QP_KEY_FONT["$QU_KEY"]="arial"
#QP_KEY_SIZE["$QU_KEY"]="9"
##QP_KEY_MAX_COL["$QU_KEY"]="8"
QP_TERM_FONT["$QU_KEY"]="arial"
#QP_TERM_SIZE["$QU_KEY"]="10"
QP_TERM_OSIZE["$QU_KEY"]=${QU_TERM_OSIZE[${QU_OTERM}]}
QP_XTITLE["$QU_KEY"]="Time (h:m:s)"
#QP_XRANGE_MIN["$QU_KEY"]="\"2019-11-10T00:00:00Z\""
#QP_XRANGE_MAX["$QU_KEY"]="\"2019-11-15T00:00:00Z\""
QP_YTITLE["$QU_KEY"]="Northings (m)"
#QP_YRANGE_MIN["$QU_KEY"]=6.5
#QP_YRANGE_MAX["$QU_KEY"]=10
#QP_X2TITLE["$QU_KEY"]=""
#QP_X2RANGE_MIN["$QU_KEY"]=0
#QP_X2RANGE_MAX["$QU_KEY"]=10
#QP_Y2TITLE["$QU_KEY"]="Time (s)"
#QP_Y2RANGE_MIN["$QU_KEY"]=6.5
#QP_Y2RANGE_MAX["$QU_KEY"]=8.8
#QP_XSCALE["$QU_KEY"]=1.0
#QP_YSCALE["$QU_KEY"]=1.0
#QP_XOFS["$QU_KEY"]=0.0
#QP_YOFS["$QU_KEY"]=0.0
QP_PLOT_STYLE["$QU_KEY"]="linespoints" #lines
QP_POINTSIZE["$QU_KEY"]=${QU_POINT_SIZE}
QP_POINTTYPE["$QU_KEY"]=${QU_POINT_TYPE}
QP_POINTCOLOR["$QU_KEY"]=${QU_POINT_COLOR}
QP_INC_POINTTYPE["$QU_KEY"]="N"
QP_INC_POINTCOLOR["$QU_KEY"]="N"
QP_USE_LINETYPES["$QU_KEY"]="N"
QP_LINETYPE["$QU_KEY"]=1
QP_INC_LINETYPE["$QU_KEY"]="N"
#QP_LINE_TYPES["$QU_KEY"]="${QU_LINE_TYPE_DFL}"
QP_EXPR["$QU_KEY"]="Y"
QP_SPECDEL["$QU_KEY"]="|"
QP_PLOT_SPECS["$QU_KEY"]="${QU_TRNO_CSV},${QU_BLUE},2,,1,3,,mmse.N"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_TRNO_CSV},${QU_ORANGE},2,,1,14,,nav.N"

QU_KEY=${QU_KEYS[2]}
QP_OFILE_NAME["$QU_KEY"]="${QU_TRNO_EASTINGS_OIMG_NAME}"
QP_PTITLE["$QU_KEY"]="${QU_TRNO_EASTINGS_PTITLE}${QU_TRNO_EASTINGS_STITLE}"
QP_TFMT["$QU_KEY"]=${QU_ITIME}
QP_XTFMT["$QU_KEY"]=${QU_OTIME}
QP_ISTIME["$QU_KEY"]="Y"
QP_YFMT["$QU_KEY"]="%g" #${QU_OTIME}
QP_DSEP["$QU_KEY"]=","
#QP_KEY_FONT["$QU_KEY"]="arial"
#QP_KEY_SIZE["$QU_KEY"]="9"
##QP_KEY_MAX_COL["$QU_KEY"]="8"
QP_TERM_FONT["$QU_KEY"]="arial"
#QP_TERM_SIZE["$QU_KEY"]="10"
QP_TERM_OSIZE["$QU_KEY"]=${QU_TERM_OSIZE[${QU_OTERM}]}
QP_XTITLE["$QU_KEY"]="Time (h:m:s)"
#QP_XRANGE_MIN["$QU_KEY"]="\"2019-11-10T00:00:00Z\""
#QP_XRANGE_MAX["$QU_KEY"]="\"2019-11-15T00:00:00Z\""
QP_YTITLE["$QU_KEY"]="Eastings (m)"
#QP_YRANGE_MIN["$QU_KEY"]=6.5
#QP_YRANGE_MAX["$QU_KEY"]=10
#QP_X2TITLE["$QU_KEY"]=""
#QP_X2RANGE_MIN["$QU_KEY"]=0
#QP_X2RANGE_MAX["$QU_KEY"]=10
#QP_Y2TITLE["$QU_KEY"]="Time (s)"
#QP_Y2RANGE_MIN["$QU_KEY"]=6.5
#QP_Y2RANGE_MAX["$QU_KEY"]=8.8
#QP_XSCALE["$QU_KEY"]=1.0
#QP_YSCALE["$QU_KEY"]=1.0
#QP_XOFS["$QU_KEY"]=0.0
#QP_YOFS["$QU_KEY"]=0.0
QP_PLOT_STYLE["$QU_KEY"]="linespoints" #lines
QP_POINTSIZE["$QU_KEY"]=${QU_POINT_SIZE}
QP_POINTTYPE["$QU_KEY"]=${QU_POINT_TYPE}
QP_POINTCOLOR["$QU_KEY"]=${QU_POINT_COLOR}
QP_INC_POINTTYPE["$QU_KEY"]="N"
QP_INC_POINTCOLOR["$QU_KEY"]="N"
QP_USE_LINETYPES["$QU_KEY"]="N"
QP_LINETYPE["$QU_KEY"]=1
QP_INC_LINETYPE["$QU_KEY"]="N"
#QP_LINE_TYPES["$QU_KEY"]="${QU_LINE_TYPE_DFL}"
QP_EXPR["$QU_KEY"]="Y"
QP_SPECDEL["$QU_KEY"]="|"
QP_PLOT_SPECS["$QU_KEY"]="${QU_TRNO_CSV},${QU_BLUE},2,,1,4,,mmse.E"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_TRNO_CSV},${QU_ORANGE},2,,1,15,,nav.E"
