#!/usr/local/bin/bash
##/bin/bash

# Combine multiple plot images into PDF

# Variable Naming Convention:
# QP_ prefix denotes environment/variables that qplot depends upon (don't modify)
# QU_ prefix for user environment prevents naming conflicts (may be imported/exported, i.e. shared between files)
# QX_ prefix denotes local/application-specific variables (file scope)

# import shared environment
source ${QP_PLOT_HOME}/qp-shared.conf.sh

# define variables
QU_SESSION_ID="" #"-`date +%s`
QU_COMB_IMG_NAME="trnucli-comb"

export QU_TRNUCLI_TIMING_OIMG_NAME="trnucli-timing"
export QU_TRNUCLI_EV_CYCLE_OIMG_NAME="trnucli-ev-cycle"
export QU_TRNUCLI_EV_SESSION_OIMG_NAME="trnucli-ev-session"
export QU_TRNUCLI_STATS_OIMG_NAME="trnucli-stats"
export QU_TRNUCLI_ERRORS_OIMG_NAME="trnucli-err"

# define image paths (uses environment from qp-shared.conf)
TRNUCLI_TIMING="${QP_OUTPUT_DIR}/${QU_TRNUCLI_TIMING_OIMG_NAME}.${QU_IMG_TYPE}"
TRNUCLI_EV_CYCLE="${QP_OUTPUT_DIR}/${QU_TRNUCLI_EV_CYCLE_OIMG_NAME}.${QU_IMG_TYPE}"
TRNUCLI_EV_SESSION="${QP_OUTPUT_DIR}/${QU_TRNUCLI_EV_SESSION_OIMG_NAME}.${QU_IMG_TYPE}"
TRNUCLI_STATS="${QP_OUTPUT_DIR}/${QU_TRNUCLI_STATS_OIMG_NAME}.${QU_IMG_TYPE}"
TRNUCLI_ERRORS="${QP_OUTPUT_DIR}/${QU_TRNUCLI_ERRORS_OIMG_NAME}.${QU_IMG_TYPE}"

# list of images to combine, in specified order
QU_FILE_LIST="${TRNUCLI_TIMING}|${TRNUCLI_EV_CYCLE}|${TRNUCLI_EV_SESSION}|${TRNUCLI_STATS}|${TRNUCLI_ERRORS}"

# Set some job name (key)
declare -a QU_KEYS=( "comb-all" )


#### BEGIN QPLOT CONFIGURATION VALUES ####

# Define the PDF combiner job
QP_JOB_DEFS["${QU_KEYS[0]}"]="combine,png,${QU_COMB_IMG_NAME}${QU_SESSION_ID}.pdf,${QU_FILE_LIST}"

# use QP_JOB_ORDER to set default job order
# [if not specified on command line w/ -j option]
# NOTE: plot combiner job is last, since plots must be completed first

# Set job order
QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[0]}"
