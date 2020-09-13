#!/usr/local/bin/bash
#/bin/bash

# Configuration for trnusvr log data plots

# Variable Naming Convention:
# QP_ prefix denotes environment/variables that qplot depends upon (don't modify)
# QU_ prefix for user environment prevents naming conflicts (may be imported/exported, i.e. shared between files)
# QX_ prefix denotes local/application-specific variables (file scope)

# import shared environment, variables
source ${QP_PLOT_HOME}/qp-shared.conf.sh

# session ID (used by local combiner job)
# QU_SESSION_ID="-`date +%s`

# define plot titles and image names
QU_TRNUSVR_PROFILE_PTITLE=${QU_PTITLE:-"TRNUSVR operation profile"}
QU_TRNUSVR_PROFILE_STITLE=${QU_STITLE:-"\n${QU_DATA_SET_ID}"}
export QU_TRNUSVR_PROFILE_OIMG_NAME="trnusvr-prof"
QU_OFILE_NAME="trnusvr-prof"

QU_TRNUSVR_CLI_PTITLE=${QU_PTITLE:-"TRNUSVR client activity"}
QU_TRNUSVR_CLI_STITLE=${QU_STITLE:-"\n${QU_DATA_SET_ID}"}
export QU_TRNUSVR_CLI_OIMG_NAME="trnusvr-cli"
QU_OFILE_NAME="trnusvr-cli"

# Define job names to use in the configuration
declare -a QU_KEYS=( "trnusvr-prof" "trnusvr-cli" "comb-all" )

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
# local combiner job (e.g. make PDF containing plots in this configuration)
#QP_JOB_DEFS["${QU_KEYS[2]}"]="combine,png,plot-output/${QU_OFILE_NAME}${QU_SESSION_ID}.pdf,./plot-output/*png"

# use QP_JOB_ORDER to enable plot jobs and set job order
# NOTE: plot combiner job is last, since plots must be completed first
QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[0]}"
QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[1]}"
#QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[2]}"

# Plot configuration parameters
QU_KEY=${QU_KEYS[0]}
QP_OFILE_NAME["$QU_KEY"]="${QU_TRNUSVR_PROFILE_OIMG_NAME}"
QP_PTITLE["$QU_KEY"]="${QU_TRNUSVR_PROFILE_PTITLE}${QU_TRNUSVR_PROFILE_STITLE}"
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
QP_YTITLE["$QU_KEY"]="Time (s)"
#QP_YRANGE_MIN["$QU_KEY"]=6.5
#QP_YRANGE_MAX["$QU_KEY"]=8.5
#QP_X2TITLE["$QU_KEY"]=""
#QP_X2RANGE_MIN["$QU_KEY"]=0
#QP_X2RANGE_MAX["$QU_KEY"]=10
QP_Y2TITLE["$QU_KEY"]="Time (s)"
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
QP_PLOT_SPECS["$QU_KEY"]="${QU_TRNUSVR_XT_UDCON_CSV},${QU_BLUE},1,,1,8,x1y2,udcon(y2)"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_TRNUSVR_XT_CHKHB_CSV},${QU_ORANGE},1,,1,8,x1y2,chkhb(y2)"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_TRNUSVR_XT_READ_CSV},${QU_YELLOW},1,,1,8,x1y1,read"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_TRNUSVR_XT_HANDLE_CSV},${QU_PURPLE},1,,1,8,x1y2,handle(y2)"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_TRNUSVR_XT_REQRES_CSV},${QU_GREEN},1,,1,8,x1y1,reqres"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_TRNUSVR_XT_PUB_CSV},${QU_LTBLUE},1,,1,8,x1y2,pub(y2)"

QU_KEY=${QU_KEYS[1]}
QP_OFILE_NAME["$QU_KEY"]="${QU_TRNUSVR_CLI_OIMG_NAME}"
QP_PTITLE["$QU_KEY"]="${QU_TRNUSVR_CLI_PTITLE}${QU_TRNUSVR_CLI_STITLE}"
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
QP_YTITLE["$QU_KEY"]="Client activity"
#QP_YRANGE_MIN["$QU_KEY"]=6.5
#QP_YRANGE_MAX["$QU_KEY"]=8.5
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
QP_INC_LINETYPE["$QU_KEY"]="Y"
#QP_LINE_TYPES["$QU_KEY"]="${QU_LINE_TYPE_DFL}"
QP_EXPR["$QU_KEY"]="Y"
QP_SPECDEL["$QU_KEY"]="|"
QP_PLOT_SPECS["$QU_KEY"]="${QU_TRNUSVR_CLICON_CSV},${QU_BLUE},1,,1,(\$5 + 0.0),,con"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_TRNUSVR_CLIDIS_CSV},${QU_ORANGE},1,,1,(\$5 + 0.01),,dis"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_TRNUSVR_CLILL_CSV},${QU_GREEN},1,,1,(\$5 + 0.02),,llen"
