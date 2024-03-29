#!/usr/bin/env bash
##/bin/bash

# Configuration for mbtrnpp log data plots

# Variable Naming Convention:
# QP_ prefix denotes environment/variables that qplot depends upon (don't modify)
# QU_ prefix for user environment prevents naming conflicts (may be imported/exported, i.e. shared between files)
# QX_ prefix denotes local/application-specific variables (file scope)

# import shared environment, variables
source ${QP_PLOT_HOME}/qp-shared.conf.sh

# session ID (used by local combiner job)
# QU_SESSION_ID="-`date +%s`

# define plot titles and image names
QU_MBTRNPP_PROFILE_PTITLE=${QU_PTITLE:-"MBTRNPP operation profile"}
QU_MBTRNPP_PROFILE_STITLE=${QU_STITLE:-"\n${QU_DATA_SET_ID}"}
export QU_MBTRNPP_PROFILE_OIMG_NAME="mbtrnpp-prof"

QU_MBTRNPP_EVENTS_PTITLE=${QU_PTITLE:-"MBTRNPP Events"}
QU_MBTRNPP_EVENTS_STITLE=${QU_STITLE:-"\n${QU_DATA_SET_ID}"}
export QU_MBTRNPP_EVENTS_OIMG_NAME="mbtrnpp-events"

QU_MBTRNPP_REINITS_PTITLE=${QU_PTITLE:-"MBTRNPP TRN Re-inits"}
QU_MBTRNPP_REINITS_STITLE=${QU_STITLE:-"\n${QU_DATA_SET_ID}"}
export QU_MBTRNPP_REINITS_OIMG_NAME="mbtrnpp-reinits"

QU_MBTRNPP_ERRORS_PTITLE=${QU_PTITLE:-"MBTRNPP Errors"}
QU_MBTRNPP_ERRORS_STITLE=${QU_STITLE:-"\n${QU_DATA_SET_ID}"}
export QU_MBTRNPP_ERRORS_OIMG_NAME="mbtrnpp-errors"

QU_MBTRNPP_PUBERRORS_PTITLE=${QU_PTITLE:-"MBTRNPP Pub Errors"}
QU_MBTRNPP_PUBERRORS_STITLE=${QU_STITLE:-"\n${QU_DATA_SET_ID}"}
export QU_MBTRNPP_PUBERRORS_OIMG_NAME="mbtrnpp-puberrors"

QU_MBTRNPP_TRNUPUB_PTITLE=${QU_PTITLE:-"MBTRNPP TRNU Publishing"}
QU_MBTRNPP_TRNUPPUB_STITLE=${QU_STITLE:-"\n${QU_DATA_SET_ID}"}
export QU_MBTRNPP_TRNUPUB_OIMG_NAME="mbtrnpp-trnupub"

QU_MBTRNPP_NONSURVEY_PTITLE=${QU_PTITLE:-"MBTRNPP Non-survey events"}
QU_MBTRNPP_NONSURVEY_STITLE=${QU_STITLE:-"\n${QU_DATA_SET_ID}"}
export QU_MBTRNPP_NONSURVEY_OIMG_NAME="mbtrnpp-nonsurvey"

# Define job names to use in the configuration
declare -a QU_KEYS=( "mbtrpp-prof" "mbtrnpp-events" "mbtrnpp-reinits" "mbtrnpp-errors" "mbtrnpp-puberrors" "mbtrnpp-trnupub" "mbtrnpp-nonsurvey" "comb-all" )

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
QP_JOB_DEFS["${QU_KEYS[5]}"]="plot,${QU_OTERM},${QU_KEYS[5]}"
QP_JOB_DEFS["${QU_KEYS[6]}"]="plot,${QU_OTERM},${QU_KEYS[6]}"
# local combiner job (e.g. make PDF containing plots in this configuration)
#QP_JOB_DEFS["${QU_KEYS[3]}"]="combine,png,plot-output/${QU_OFILE_NAME}${QU_SESSION_ID}.pdf,./plot-output/*png"

# use QP_JOB_ORDER to enable plot jobs and set job order
# NOTE: plot combiner job is last, since plots must be completed first
QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[0]}"
QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[1]}"
QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[2]}"
QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[3]}"
QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[4]}"
QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[5]}"
QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[6]}"
#QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[6]}"

# Plot configuration parameters
QU_KEY=${QU_KEYS[0]}
QP_OFILE_NAME["$QU_KEY"]="${QU_MBTRNPP_PROFILE_OIMG_NAME}"
QP_PTITLE["$QU_KEY"]="${QU_MBTRNPP_PROFILE_PTITLE}${QU_MBTRNPP_PROFILE_STITLE}"
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
#QP_XRANGE_MAX["$QU_KEY"]=
QP_YTITLE["$QU_KEY"]="Time (s)"
#QP_YRANGE_MIN["$QU_KEY"]=6.5
QP_YRANGE_MAX["$QU_KEY"]=1. #0.8
#QP_X2TITLE["$QU_KEY"]=""
#QP_X2RANGE_MIN["$QU_KEY"]=0
#QP_X2RANGE_MAX["$QU_KEY"]=10
QP_Y2TITLE["$QU_KEY"]="Time (s)"
#QP_Y2RANGE_MIN["$QU_KEY"]=0
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
QP_PLOT_SPECS["$QU_KEY"]="${QU_MBTRNPP_XT_GETALL_CSV},${QU_BLUE},1,,1,8,x1y1,getall"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_XT_CYCLE_CSV},${QU_ORANGE},1,,1,8,x1y1,cycle"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_XT_STATS_CSV},${QU_YELLOW},1,,1,8,x1y2,stats(y2)"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_XT_FWRITE_CSV},${QU_PURPLE},1,,1,8,x1y2,fwrite(y2)"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_XT_PING_CSV},${QU_GREEN},1,,1,8,x1y2,ping(y2)"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_XT_TRNUPDATE_CSV},${QU_LTBLUE},1,,1,8,,trn-update"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_XT_TRNBIASEST_CSV},${QU_RED},1,,1,8,x1y2,trn-biasest(y2)"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_XT_TRNPROC_CSV},'#00ffff',1,,1,8,x1y2,trn-proc(y2)"
#QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_XT_TRNPROC_CSV},${QU_GOLDENROD},1,,1,8,x1y2,trn-proc(y2)"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_XT_PROCMB1_CSV},${QU_BLACK},1,,1,8,x1y2,proc-mb1(y2)"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_XT_TRNPROCTRN_CSV},${QU_GRAY},1,,1,8,x1y2,proc-trn(y2)"

QU_KEY=${QU_KEYS[1]}
QP_OFILE_NAME["$QU_KEY"]="${QU_MBTRNPP_EVENTS_OIMG_NAME}"
QP_PTITLE["$QU_KEY"]="${QU_MBTRNPP_EVENTS_PTITLE}${QU_MBTRNPP_EVENTS_STITLE}"
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
#QP_XRANGE_MAX["$QU_KEY"]=
QP_YTITLE["$QU_KEY"]="mbtrnpp Events"
#QP_YRANGE_MIN["$QU_KEY"]=6.5
QP_YRANGE_MAX["$QU_KEY"]=10. #0.8
#QP_X2TITLE["$QU_KEY"]=""
#QP_X2RANGE_MIN["$QU_KEY"]=0
#QP_X2RANGE_MAX["$QU_KEY"]=10
#QP_Y2TITLE["$QU_KEY"]="Time (s)"
#QP_Y2RANGE_MIN["$QU_KEY"]=0
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
QP_PLOT_SPECS["$QU_KEY"]="${QU_MBTRNPP_E_MBCON_CSV},${QU_ORANGE},1,,1,(\$5 + 0.00),,mbcon"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_MBTRNPP_E_MBDIS_CSV},${QU_YELLOW},1,,1,(\$5 + 0.01),,mbdis"

QU_KEY=${QU_KEYS[2]}
QP_OFILE_NAME["$QU_KEY"]="${QU_MBTRNPP_REINITS_OIMG_NAME}"
QP_PTITLE["$QU_KEY"]="${QU_MBTRNPP_REINITS_PTITLE}${QU_MBTRNPP_REINITS_STITLE}"
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
#QP_XRANGE_MAX["$QU_KEY"]=
QP_YTITLE["$QU_KEY"]="mbtrnpp Events"
#QP_YRANGE_MIN["$QU_KEY"]=6.5
#QP_YRANGE_MAX["$QU_KEY"]=1. #0.8
#QP_X2TITLE["$QU_KEY"]=""
#QP_X2RANGE_MIN["$QU_KEY"]=0
#QP_X2RANGE_MAX["$QU_KEY"]=10
#QP_Y2TITLE["$QU_KEY"]="Time (s)"
#QP_Y2RANGE_MIN["$QU_KEY"]=0
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
QP_PLOT_SPECS["$QU_KEY"]="${QU_MBTRNPP_E_TRNREINIT_CSV},${QU_BLUE},1,,1,(\$5 + 0.0),,reinit"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_MBTRNPP_E_GAINLO_CSV},${QU_GREEN},1,,1,(\$5 + 0.01),,gainlo"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_MBTRNPP_E_MBXYOFFSET_CSV},${QU_PURPLE},1,,1,(\$5 + 0.02),,xyoffset"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_MBTRNPP_E_MBZOFFSET_CSV},${QU_GOLDENROD},1,,1,(\$5 + 0.03),,zoffset"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_MBTRNPP_E_MBTRNUCLI_RESET_CSV},${QU_RED},1,,1,(\$5 + 0.04),,trnucli"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_MBTRNPP_E_MBEOF_CSV},${QU_LTBLUE},1,,1,(\$5 + 0.06),,eof"

QU_KEY=${QU_KEYS[3]}
QP_OFILE_NAME["$QU_KEY"]="${QU_MBTRNPP_ERRORS_OIMG_NAME}"
QP_PTITLE["$QU_KEY"]="${QU_MBTRNPP_ERRORS_PTITLE}${QU_MBTRNPP_ERRORS_STITLE}"
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
#QP_XRANGE_MAX["$QU_KEY"]=
QP_YTITLE["$QU_KEY"]="mbtrnpp Errors"
#QP_YRANGE_MIN["$QU_KEY"]=6.5
#QP_YRANGE_MAX["$QU_KEY"]=10. #0.8
#QP_X2TITLE["$QU_KEY"]=""
#QP_X2RANGE_MIN["$QU_KEY"]=0
#QP_X2RANGE_MAX["$QU_KEY"]=10
#QP_Y2TITLE["$QU_KEY"]="Time (s)"
#QP_Y2RANGE_MIN["$QU_KEY"]=0
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
# add small offset to separate traces; read nearest integer below trace
QP_PLOT_SPECS["$QU_KEY"]="${QU_MBTRNPP_E_EMBGETALL_CSV},${QU_BLUE},1,,1,(\$5 + 0.0),,embgetall"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_MBTRNPP_E_EMBFAIL_CSV},${QU_GREEN},1,,1,(\$5 + 0.01),,embfail"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_MBTRNPP_E_EMBSOCK_CSV},${QU_ORANGE},1,,1,(\$5 + 0.02),,embsock"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_MBTRNPP_E_EMBCON_CSV},${QU_YELLOW},1,,1,(\$5 + 0.03),,embcon"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_MBTRNPP_E_EMBFRAMERD_CSV},${QU_RED},1,,1,(\$5 + 0.04),,embframerd"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_MBTRNPP_E_EMBLOGWR_CSV},${QU_PURPLE},1,,1,(\$5 + 0.05),,emblogwr"

QU_KEY=${QU_KEYS[4]}
QP_OFILE_NAME["$QU_KEY"]="${QU_MBTRNPP_PUBERRORS_OIMG_NAME}"
QP_PTITLE["$QU_KEY"]="${QU_MBTRNPP_PUBERRORS_PTITLE}${QU_MBTRNPP_PUBERRORS_STITLE}"
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
#QP_XRANGE_MAX["$QU_KEY"]=
QP_YTITLE["$QU_KEY"]="mbtrnpp Pub Errors"
#QP_YRANGE_MIN["$QU_KEY"]=6.5
#QP_YRANGE_MAX["$QU_KEY"]=10. #0.8
#QP_X2TITLE["$QU_KEY"]=""
#QP_X2RANGE_MIN["$QU_KEY"]=0
#QP_X2RANGE_MAX["$QU_KEY"]=10
#QP_Y2TITLE["$QU_KEY"]="Time (s)"
#QP_Y2RANGE_MIN["$QU_KEY"]=0
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
# add small offset to separate traces; read nearest integer below trace
QP_PLOT_SPECS["$QU_KEY"]+="${QU_MBTRNPP_E_ETRNUPUB_CSV},${QU_GOLDENROD},1,,1,(\$5 + 0.05),,etrnupub"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_MBTRNPP_E_ETRNUPUBEMPTY_CSV},${QU_LTBLUE},1,,1,(\$5 + 0.05),,etrnupubempty"

# Plot configuration parameters
QU_KEY=${QU_KEYS[5]}
QP_OFILE_NAME["$QU_KEY"]="${QU_MBTRNPP_TRNUPUB_OIMG_NAME}"
QP_PTITLE["$QU_KEY"]="${QU_MBTRNPP_TRNUPUB_PTITLE}${QU_MBTRNPP_TRNUPUB_STITLE}"
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
#QP_XRANGE_MAX["$QU_KEY"]=
QP_YTITLE["$QU_KEY"]="TRNU pub events"
#QP_YRANGE_MIN["$QU_KEY"]=6.5
#QP_YRANGE_MAX["$QU_KEY"]=1. #0.8
#QP_X2TITLE["$QU_KEY"]=""
#QP_X2RANGE_MIN["$QU_KEY"]=0
#QP_X2RANGE_MAX["$QU_KEY"]=10
#QP_Y2TITLE["$QU_KEY"]="Time (s)"
#QP_Y2RANGE_MIN["$QU_KEY"]=0
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
QP_EXPR["$QU_KEY"]="Y"
QP_SPECDEL["$QU_KEY"]="|"
#QP_LINE_TYPES["$QU_KEY"]="${QU_LINE_TYPE_DFL}"
QP_PLOT_SPECS["$QU_KEY"]="${QU_MBTRNPP_E_TRNUPUB_CSV},${QU_BLUE},1,,1,(\$5 + 0.0),,trnu.pub"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_MBTRNPP_E_TRNUPUBEMPTY_CSV},${QU_GREEN},1,,1,(\$5 + 0.01),,trnu.pub.empty"

# Plot configuration parameters
QU_KEY=${QU_KEYS[6]}
QP_OFILE_NAME["$QU_KEY"]="${QU_MBTRNPP_NONSURVEY_OIMG_NAME}"
QP_PTITLE["$QU_KEY"]="${QU_MBTRNPP_NONSURVEY_PTITLE}${QU_MBTRNPP_NONSURVEY_STITLE}"
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
#QP_XRANGE_MAX["$QU_KEY"]=
QP_YTITLE["$QU_KEY"]="TRNU Non-survey events"
#QP_YRANGE_MIN["$QU_KEY"]=6.5
#QP_YRANGE_MAX["$QU_KEY"]=1. #0.8
#QP_X2TITLE["$QU_KEY"]=""
#QP_X2RANGE_MIN["$QU_KEY"]=0
#QP_X2RANGE_MAX["$QU_KEY"]=10
#QP_Y2TITLE["$QU_KEY"]="Time (s)"
#QP_Y2RANGE_MIN["$QU_KEY"]=0
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
QP_EXPR["$QU_KEY"]="Y"
QP_SPECDEL["$QU_KEY"]="|"
#QP_LINE_TYPES["$QU_KEY"]="${QU_LINE_TYPE_DFL}"
QP_PLOT_SPECS["$QU_KEY"]="${QU_MBTRNPP_E_MBNONSURVEY_CSV},${QU_BLUE},1,,1,(\$5 + 0.0),,mb.nonsurvey"
