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
# Let app set session ID
#QU_SESSION_ID="" #"-`date +%s`
QU_COMB_IMG_NAME="mbtrnpp-comb"

export QU_TRNO_OFS_OIMG_NAME="trno-$QU_SESSION_ID-ofs"
export QU_TRNO_NOR_OIMG_NAME="trno-$QU_SESSION_ID-nor"
export QU_TRNO_EAS_OIMG_NAME="trno-$QU_SESSION_ID-eas"
export QU_TRNO_DEP_OIMG_NAME="trno-$QU_SESSION_ID-dep"
export QU_TRNO_POS_OIMG_NAME="trno-$QU_SESSION_ID-pos"

# define image paths (uses environment from qp-shared.conf)
TRNO_OFS="${QP_OUTPUT_DIR}/${QU_TRNO_OFS_OIMG_NAME}.${QU_IMG_TYPE}"
TRNO_NOR="${QP_OUTPUT_DIR}/${QU_TRNO_NOR_OIMG_NAME}.${QU_IMG_TYPE}"
TRNO_EAS="${QP_OUTPUT_DIR}/${QU_TRNO_EAS_OIMG_NAME}.${QU_IMG_TYPE}"
TRNO_DEP="${QP_OUTPUT_DIR}/${QU_TRNO_DEP_OIMG_NAME}.${QU_IMG_TYPE}"
TRNO_POS="${QP_OUTPUT_DIR}/${QU_TRNO_POS_OIMG_NAME}.${QU_IMG_TYPE}"

# list of images to combine, in specified order
QU_FILE_LIST="${TRNO_OFS}|${TRNO_NOR}|${TRNO_EAS}|${TRNO_DEP}|${TRNO_POS}"

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
