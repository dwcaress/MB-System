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
QU_SESSION_ID=${QU_SESSION_ID:-""} #"-`date +%s`
QU_COMB_IMG_NAME="tlp-comb"

export QU_TRNO_OFS_OIMG_NAME="trno-ofs-${QU_SESSION_ID}"
export QU_TRNO_NOR_OIMG_NAME="trno-nor-${QU_SESSION_ID}"
export QU_TRNO_EAS_OIMG_NAME="trno-eas-${QU_SESSION_ID}"
export QU_TRNO_DEP_OIMG_NAME="trno-dep-${QU_SESSION_ID}"
export QU_TRNO_NERR_OIMG_NAME="trno-nerr-${QU_SESSION_ID}"
export QU_TRNO_EERR_OIMG_NAME="trno-eerr-${QU_SESSION_ID}"
export QU_TRNO_ZERR_OIMG_NAME="trno-zerr-${QU_SESSION_ID}"
export QU_TRNO_NAV_OIMG_NAME="trno-nav-${QU_SESSION_ID}"

# define image paths (uses environment from qp-shared.conf)
TRNO_OFS="${QP_OUTPUT_DIR}/${QU_TRNO_OFS_OIMG_NAME}.${QU_IMG_TYPE}"
TRNO_NOR="${QP_OUTPUT_DIR}/${QU_TRNO_NOR_OIMG_NAME}.${QU_IMG_TYPE}"
TRNO_EAS="${QP_OUTPUT_DIR}/${QU_TRNO_EAS_OIMG_NAME}.${QU_IMG_TYPE}"
TRNO_DEP="${QP_OUTPUT_DIR}/${QU_TRNO_DEP_OIMG_NAME}.${QU_IMG_TYPE}"
TRNO_NERR="${QP_OUTPUT_DIR}/${QU_TRNO_NERR_OIMG_NAME}.${QU_IMG_TYPE}"
TRNO_EERR="${QP_OUTPUT_DIR}/${QU_TRNO_EERR_OIMG_NAME}.${QU_IMG_TYPE}"
TRNO_ZERR="${QP_OUTPUT_DIR}/${QU_TRNO_ZERR_OIMG_NAME}.${QU_IMG_TYPE}"
TRNO_NAV="${QP_OUTPUT_DIR}/${QU_TRNO_NAV_OIMG_NAME}.${QU_IMG_TYPE}"

# list of images to combine, in specified order
QU_FILE_LIST="${TRNO_OFS}|${TRNO_NERR}|${TRNO_EERR}|${TRNO_ZERR}|${TRNO_NOR}|${TRNO_EAS}|${TRNO_DEP}|${TRNO_NAV}"

# Set some job name (key)
declare -a QU_KEYS=( "comb-all" )


#### BEGIN QPLOT CONFIGURATION VALUES ####

# Define the PDF combiner job
QP_JOB_DEFS["${QU_KEYS[0]}"]="combine,png,${QU_COMB_IMG_NAME}-${QU_SESSION_ID}.pdf,${QU_FILE_LIST}"

# use QP_JOB_ORDER to set default job order
# [if not specified on command line w/ -j option]
# NOTE: plot combiner job is last, since plots must be completed first

# Set job order
QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[0]}"
