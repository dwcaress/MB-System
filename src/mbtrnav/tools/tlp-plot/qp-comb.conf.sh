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
QU_COMB_IMG_NAME="mbtrnpp-comb"

export QU_TRNO_DVL_OFS_OIMG_NAME="trno-dvl-ofs"
export QU_TRNO_DVL_NOR_OIMG_NAME="trno-dvl-nor"
export QU_TRNO_DVL_EAS_OIMG_NAME="trno-dvl-eas"

export QU_TRNO_IDT_OFS_OIMG_NAME="trno-idt-ofs"
export QU_TRNO_IDT_NOR_OIMG_NAME="trno-idt-nor"
export QU_TRNO_IDT_EAS_OIMG_NAME="trno-idt-eas"

# define image paths (uses environment from qp-shared.conf)
TRNODVL_OFS="${QP_OUTPUT_DIR}/${QU_TRNO_DVL_OFS_OIMG_NAME}.${QU_IMG_TYPE}"
TRNODVL_NOR="${QP_OUTPUT_DIR}/${QU_TRNO_DVL_NOR_OIMG_NAME}.${QU_IMG_TYPE}"
TRNODVL_EAS="${QP_OUTPUT_DIR}/${QU_TRNO_DVL_EAS_OIMG_NAME}.${QU_IMG_TYPE}"

TRNOIDT_OFS="${QP_OUTPUT_DIR}/${QU_TRNO_IDT_OFS_OIMG_NAME}.${QU_IMG_TYPE}"
TRNOIDT_NOR="${QP_OUTPUT_DIR}/${QU_TRNO_IDT_NOR_OIMG_NAME}.${QU_IMG_TYPE}"
TRNOIDT_EAS="${QP_OUTPUT_DIR}/${QU_TRNO_IDT_EAS_OIMG_NAME}.${QU_IMG_TYPE}"


# list of images to combine, in specified order
QU_FILE_LIST="${TRNODVL_OFS}|${TRNODVL_NOR}|${TRNODVL_EAS}|${TRNOIDT_OFS}|${TRNOIDT_NOR}|${TRNOIDT_EAS}"

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
