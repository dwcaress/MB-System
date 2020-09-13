#!/usr/local/bin/bash
#/bin/bash

# Configuration for trnuctx log data plots

# Variable Naming Convention:
# QP_ prefix denotes environment/variables that qplot depends upon (don't modify)
# QU_ prefix for user environment prevents naming conflicts (may be imported/exported, i.e. shared between files)
# QX_ prefix denotes local/application-specific variables (file scope)

# import shared environment, variables
source ${QP_PLOT_HOME}/qp-shared.conf.sh

# session ID (used by local combiner job)
# QU_SESSION_ID="-`date +%s`

# define plot titles and image names

QU_STITLE=${QU_STITLE:-"\n${QU_DATA_SET_ID}"}

export QU_TRNUCLI_TIMING_OIMG_NAME="trnucli-timing"
QU_TRNUCLI_TIMING_PTITLE=${QU_PTITLE:-"TRNUCLI timing"}

export QU_TRNUCLI_EV_CYCLE_OIMG_NAME="trnucli-ev-cycle"
QU_TRNUCLI_EVE_CYCLE_PTITLE=${QU_PTITLE:-"TRNUCLI cycle events"}

export QU_TRNUCLI_EV_SESSION_OIMG_NAME="trnucli-ev-session"
QU_TRNUCLI_EV_SESSION_PTITLE=${QU_PTITLE:-"TRNUCLI session events"}

export QU_TRNUCLI_STATS_OIMG_NAME="trnucli-stats"
QU_TRNUCLI_STATS_PTITLE=${QU_PTITLE:-"TRNUCLI stats"}

export QU_TRNUCLI_ERR_OIMG_NAME="trnucli-err"
QU_TRNUCLI_ERR_PTITLE=${QU_PTITLE:-"TRNUCLI errors"}


# Define job names to use in the configuration
declare -a QU_KEYS=( "trnucli-timing" "trnucli-ev-cycle" "trnucli-ev-session" "trnucli-stats" "trnucli-err" "comb-all" )

# Set time formats for data and plots
# time format strings conform to gnuplot syntax
# (see man pages: strftime/strptime, date)

# input time format (must match data files)
# e.g. 2019-11-13T22:52:29Z
QU_ITIME="%Y-%m-%dT%H:%M:%SZ"
# output time format (gnuplot plot x-axis)
QU_OTIME="%Y-%m-%dT%H:%M:%SZ"


#### BEGIN QPLOT CONFIGURATION VALUES ####

# Define plot jobs
QP_JOB_DEFS["${QU_KEYS[0]}"]="plot,${QU_OTERM},${QU_KEYS[0]}"
QP_JOB_DEFS["${QU_KEYS[1]}"]="plot,${QU_OTERM},${QU_KEYS[1]}"
QP_JOB_DEFS["${QU_KEYS[2]}"]="plot,${QU_OTERM},${QU_KEYS[2]}"
QP_JOB_DEFS["${QU_KEYS[3]}"]="plot,${QU_OTERM},${QU_KEYS[3]}"
QP_JOB_DEFS["${QU_KEYS[4]}"]="plot,${QU_OTERM},${QU_KEYS[4]}"
# local combiner job (e.g. make PDF containing plots in this configuration)
#QP_JOB_DEFS["${QU_KEYS[4]}"]="combine,png,plot-output/${QU_OFILE_NAME}${QU_SESSION_ID}.pdf,./plot-output/*png"

# use QP_JOB_ORDER to enable plot jobs and set job order
# NOTE: plot combiner job is last, since plots must be completed first
QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[0]}"
QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[1]}"
QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[2]}"
QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[3]}"
QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[4]}"
#QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[4]}"

# Plot configuration parameters

QU_KEY=${QU_KEYS[0]}
QP_OFILE_NAME["$QU_KEY"]="${QU_TRNUCLI_TIMING_OIMG_NAME}"
QP_PTITLE["$QU_KEY"]="${QU_TRNUCLI_TIMING_PTITLE}${QU_STITLE}"
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
QP_XTITLE["$QU_KEY"]="time"
#QP_XRANGE_MIN["$QU_KEY"]=0
#QP_XRANGE_MAX["$QU_KEY"]=10
QP_YTITLE["$QU_KEY"]="Time in state"
#QP_YRANGE_MIN["$QU_KEY"]=6.5
#QP_YRANGE_MAX["$QU_KEY"]=8.5
#QP_X2TITLE["$QU_KEY"]=""
#QP_X2RANGE_MIN["$QU_KEY"]=0
#QP_X2RANGE_MAX["$QU_KEY"]=10
#QP_Y2TITLE["$QU_KEY"]="Depth"
#QP_Y2RANGE_MIN["$QU_KEY"]=6.5
#QP_Y2RANGE_MAX["$QU_KEY"]=8.8
#QP_XSCALE["$QU_KEY"]=-1.0
#QP_YSCALE["$QU_KEY"]=-1.0
#QP_XOFS["$QU_KEY"]=4.0618e6
#QP_YOFS["$QU_KEY"]=594500.
QP_PLOT_STYLE["$QU_KEY"]="linespoints" #lines
QP_POINTSIZE["$QU_KEY"]=${QU_POINT_SIZE}
QP_POINTTYPE["$QU_KEY"]=${QU_POINT_TYPE}
QP_POINTCOLOR["$QU_KEY"]=${QU_POINT_COLOR}
QP_INC_POINTTYPE["$QU_KEY"]="N"
QP_INC_POINTCOLOR["$QU_KEY"]="N"
QP_USE_LINETYPES["$QU_KEY"]="N"
QP_LINETYPE["$QU_KEY"]=1
QP_INC_LINETYPE["$QU_KEY"]="Y"
#QP_LINE_TYPES["$QU_KEY"]="${QU_LINE_TYPE_DFL}"
#QP_EXPR["$QU_KEY"]="Y"
QP_SPECDEL["$QU_KEY"]="|"
QP_PLOT_SPECS["$QU_KEY"]="${QU_TRNUCTX_T_SESSION_CSV},${QU_ORANGE},1,,1,4,,t-ses"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_TRNUCTX_T_LIST_CSV},${QU_YELLOW},1,,1,4,,t-list"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_TRNUCTX_T_CON_CSV},${QU_PURPLE},1,,1,4,,t-con"


QU_KEY=${QU_KEYS[1]}
QP_OFILE_NAME["$QU_KEY"]="${QU_TRNUCLI_EV_CYCLE_OIMG_NAME}"
QP_PTITLE["$QU_KEY"]="${QU_TRNUCLI_EV_CYCLE_PTITLE}${QU_STITLE}"
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
#QP_XRANGE_MIN["$QU_KEY"]=0
#QP_XRANGE_MAX["$QU_KEY"]=10
QP_YTITLE["$QU_KEY"]="Event count"
#QP_YRANGE_MIN["$QU_KEY"]=6.5
#QP_YRANGE_MAX["$QU_KEY"]=5.0
#QP_X2TITLE["$QU_KEY"]=""
#QP_X2RANGE_MIN["$QU_KEY"]=0
#QP_X2RANGE_MAX["$QU_KEY"]=10
#QP_Y2TITLE["$QU_KEY"]="Depth"
#QP_Y2RANGE_MIN["$QU_KEY"]=6.5
#QP_Y2RANGE_MAX["$QU_KEY"]=8.8
#QP_XSCALE["$QU_KEY"]=-1.0
#QP_YSCALE["$QU_KEY"]=-1.0
#QP_XOFS["$QU_KEY"]=4.0618e6
#QP_YOFS["$QU_KEY"]=594500.
QP_PLOT_STYLE["$QU_KEY"]="linespoints" #lines
QP_POINTSIZE["$QU_KEY"]=${QU_POINT_SIZE}
QP_POINTTYPE["$QU_KEY"]=${QU_POINT_TYPE}
QP_POINTCOLOR["$QU_KEY"]=${QU_POINT_COLOR}
QP_INC_POINTTYPE["$QU_KEY"]="N"
QP_INC_POINTCOLOR["$QU_KEY"]="N"
QP_USE_LINETYPES["$QU_KEY"]="N"
QP_LINETYPE["$QU_KEY"]=1
QP_INC_LINETYPE["$QU_KEY"]="Y"
#QP_LINE_TYPES["$QU_KEY"]="${QU_LINE_TYPE_DFL}"
QP_EXPR["$QU_KEY"]="Y"
QP_SPECDEL["$QU_KEY"]="|"
# add small offset to separate traces; read nearest integer below trace
QP_PLOT_SPECS["$QU_KEY"]="${QU_TRNUCTX_E_UPDATE_CSV},${QU_BLACK},1,,1,(\$4 + 0.04),,n.update"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_TRNUCTX_E_CYCLE_CSV},${QU_RED},1,,1,(\$4 + 0.05),,n.cycles"

QU_KEY=${QU_KEYS[2]}
QP_OFILE_NAME["$QU_KEY"]="${QU_TRNUCLI_EV_SESSION_OIMG_NAME}"
QP_PTITLE["$QU_KEY"]="${QU_TRNUCLI_EV_SESSION_PTITLE}${QU_STITLE}"
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
#QP_XRANGE_MIN["$QU_KEY"]=0
#QP_XRANGE_MAX["$QU_KEY"]=10
QP_YTITLE["$QU_KEY"]="Event count"
#QP_YRANGE_MIN["$QU_KEY"]=6.5
#QP_YRANGE_MAX["$QU_KEY"]=5.0
#QP_X2TITLE["$QU_KEY"]=""
#QP_X2RANGE_MIN["$QU_KEY"]=0
#QP_X2RANGE_MAX["$QU_KEY"]=10
#QP_Y2TITLE["$QU_KEY"]="Depth"
#QP_Y2RANGE_MIN["$QU_KEY"]=6.5
#QP_Y2RANGE_MAX["$QU_KEY"]=8.8
#QP_XSCALE["$QU_KEY"]=-1.0
#QP_YSCALE["$QU_KEY"]=-1.0
#QP_XOFS["$QU_KEY"]=4.0618e6
#QP_YOFS["$QU_KEY"]=594500.
QP_PLOT_STYLE["$QU_KEY"]="linespoints" #lines
QP_POINTSIZE["$QU_KEY"]=${QU_POINT_SIZE}
QP_POINTTYPE["$QU_KEY"]=${QU_POINT_TYPE}
QP_POINTCOLOR["$QU_KEY"]=${QU_POINT_COLOR}
QP_INC_POINTTYPE["$QU_KEY"]="N"
QP_INC_POINTCOLOR["$QU_KEY"]="N"
QP_USE_LINETYPES["$QU_KEY"]="N"
QP_LINETYPE["$QU_KEY"]=1
QP_INC_LINETYPE["$QU_KEY"]="Y"
#QP_LINE_TYPES["$QU_KEY"]="${QU_LINE_TYPE_DFL}"
QP_EXPR["$QU_KEY"]="Y"
QP_SPECDEL["$QU_KEY"]="|"
# add small offset to separate traces; read nearest integer below trace
QP_PLOT_SPECS["$QU_KEY"]="${QU_TRNUCTX_E_RCTEX_CSV},${QU_ORANGE},1,,1,(\$3 + 0.01),,rctex"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_TRNUCTX_E_HBTEX_CSV},${QU_YELLOW},1,,1,(\$3 + 0.02),,hbtex"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_TRNUCTX_E_CON_CSV},${QU_RED},1,,1,(\$4 + 0.05),,n.connect"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_TRNUCTX_E_DIS_CSV},${QU_GOLDENROD},1,,1,(\$4 + 0.06),,n.disconnect"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_TRNUCTX_E_HBEAT_CSV},${QU_LTBLUE},1,,1,(\$4 + 0.07),,n.hbeat"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_TRNUCTX_E_RCTO_CSV},${QU_GRAY},1,,1,(\$4 + 0.08),,n.rcto"

QU_KEY=${QU_KEYS[3]}
QP_OFILE_NAME["$QU_KEY"]="${QU_TRNUCLI_STATS_OIMG_NAME}"
QP_PTITLE["$QU_KEY"]="${QU_TRNUCLI_STATS_PTITLE}${QU_STITLE}"
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
#QP_XRANGE_MIN["$QU_KEY"]=0
#QP_XRANGE_MAX["$QU_KEY"]=10
QP_YTITLE["$QU_KEY"]="Event value"
#QP_YRANGE_MIN["$QU_KEY"]=6.5
#QP_YRANGE_MAX["$QU_KEY"]=8.5
#QP_X2TITLE["$QU_KEY"]=""
#QP_X2RANGE_MIN["$QU_KEY"]=0
#QP_X2RANGE_MAX["$QU_KEY"]=10
#QP_Y2TITLE["$QU_KEY"]="Depth"
#QP_Y2RANGE_MIN["$QU_KEY"]=6.5
#QP_Y2RANGE_MAX["$QU_KEY"]=8.8
#QP_XSCALE["$QU_KEY"]=-1.0
#QP_YSCALE["$QU_KEY"]=-1.0
#QP_XOFS["$QU_KEY"]=4.0618e6
#QP_YOFS["$QU_KEY"]=594500.
QP_PLOT_STYLE["$QU_KEY"]="points" #lines
QP_POINTSIZE["$QU_KEY"]=${QU_POINT_SIZE}
QP_POINTTYPE["$QU_KEY"]=${QU_POINT_TYPE}
QP_POINTCOLOR["$QU_KEY"]=${QU_POINT_COLOR}
QP_INC_POINTTYPE["$QU_KEY"]="N"
QP_INC_POINTCOLOR["$QU_KEY"]="N"
QP_USE_LINETYPES["$QU_KEY"]="N"
QP_LINETYPE["$QU_KEY"]=1
QP_INC_LINETYPE["$QU_KEY"]="Y"
#QP_LINE_TYPES["$QU_KEY"]="${QU_LINE_TYPE_DFL}"
QP_PLOT_SPECS["$QU_KEY"]="${QU_TRNUCTX_E_UPDATE_STAT_CSV},${QU_GREEN},1,,1,3,,update"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_TRNUCTX_E_UPDATE_STAT_CSV},${QU_BLUE},1,,1,4,,cycle"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_TRNUCTX_E_UPDATE_STAT_CSV},${QU_RED},1,,1,5,,elist"
#QP_PLOT_SPECS["$QU_KEY"]+="+${QU_TRNUCTX_E_UPDATE_STAT_CSV},${QU_GOLDENROD},1,,1,6,,tsvr"

QU_KEY=${QU_KEYS[4]}
QP_OFILE_NAME["$QU_KEY"]="${QU_TRNUCLI_ERR_OIMG_NAME}"
QP_PTITLE["$QU_KEY"]="${QU_TRNUCLI_ERR_PTITLE}${QU_STITLE}"
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
#QP_XRANGE_MIN["$QU_KEY"]=0
#QP_XRANGE_MAX["$QU_KEY"]=10
QP_YTITLE["$QU_KEY"]="Errors"
#QP_YRANGE_MIN["$QU_KEY"]=6.5
#QP_YRANGE_MAX["$QU_KEY"]=5.0
#QP_X2TITLE["$QU_KEY"]=""
#QP_X2RANGE_MIN["$QU_KEY"]=0
#QP_X2RANGE_MAX["$QU_KEY"]=10
#QP_Y2TITLE["$QU_KEY"]="Depth"
#QP_Y2RANGE_MIN["$QU_KEY"]=6.5
#QP_Y2RANGE_MAX["$QU_KEY"]=8.8
#QP_XSCALE["$QU_KEY"]=-1.0
#QP_YSCALE["$QU_KEY"]=-1.0
#QP_XOFS["$QU_KEY"]=4.0618e6
#QP_YOFS["$QU_KEY"]=594500.
QP_PLOT_STYLE["$QU_KEY"]="linespoints" #lines
QP_POINTSIZE["$QU_KEY"]=${QU_POINT_SIZE}
QP_POINTTYPE["$QU_KEY"]=${QU_POINT_TYPE}
QP_POINTCOLOR["$QU_KEY"]=${QU_POINT_COLOR}
QP_INC_POINTTYPE["$QU_KEY"]="N"
QP_INC_POINTCOLOR["$QU_KEY"]="N"
QP_USE_LINETYPES["$QU_KEY"]="N"
QP_LINETYPE["$QU_KEY"]=1
QP_INC_LINETYPE["$QU_KEY"]="Y"
#QP_LINE_TYPES["$QU_KEY"]="${QU_LINE_TYPE_DFL}"
QP_EXPR["$QU_KEY"]="Y"
QP_SPECDEL["$QU_KEY"]="|"
# add small offset to separate traces; read nearest integer below trace
QP_PLOT_SPECS["$QU_KEY"]="${QU_TRNUCTX_E_ELIST_CSV},${QU_BLUE},1,,1,(\$4 + 0.0),,elisten"
QP_PLOT_SPECS["$QU_KEY"]="${QU_TRNUCTX_E_ECON_CSV},${QU_GREEN},1,,1,(\$4 + 0.1),,econnect"
