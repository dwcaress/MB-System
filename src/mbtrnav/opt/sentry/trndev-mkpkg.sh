#!/bin/bash

# Generate trndev development snapshot

#########################################
# Script configuration defaults
# casual users should not need to change
# anything below this section
#########################################
description="make trndev package"

#################################
# Script variable initialization
#################################
VERBOSE="N"
KEEP_FILES="N"
GIT=$(which git)
TAR=$(which tar)
MAKE=$(which make)

#################################
# name: printUsage
# description: print use message
# args: none
#################################
printUsage(){
    echo
    echo "Description: $description"
    echo
    echo "usage: `basename $0` [options]"
    echo "Options:"
    echo "-v             : verbose output    [$VERBOSE]"
    echo "-h             : print use message"
    echo "-k             : keep build files  [$KEEP_FILES]"
    echo ""
    echo
}

########################################
# name: vout
# description: print verbose message to stderr
# args:
#     msg: message
########################################
vout(){
if [ "${VERBOSE}" == "Y" ] || [ "${VERBOSE}" == "TRUE" ]
then
    echo "$1" >&2
fi
}

########################################
# name: processCmdLine
# description: do command line processsing
# args:
#     args:       positional paramters
#     returnCode: none
########################################
processCmdLine(){
    OPTIND=1
    vout "`basename $0` all args[$*]"

    while getopts hkv Option
    do
        vout "processing $Option[$OPTARG]"
        case $Option in
            v ) VERBOSE="TRUE"
            ;;
            h ) printUsage
                    exit 0
            ;;
            k ) KEEP_FILES="Y"
            ;;
           *) exit 0 # getopts outputs error message
            ;;
        esac
    done
}

showVars(){
    vout " GIT            $GIT"
    vout " MAKE           $MAKE"
    vout " TAR            $TAR"
    vout " DIST_DATE      $DIST_DATE"
    vout " TRNDEV_WD      $TRNDEV_WD"
    vout " TRNDEV_TGZ     $TRNDEV_TGZ"
    vout " TRNDEV_SDNAME  $TRNDEV_SDNAME"
    vout " TRNDEV_SD      $TRNDEV_SD"
    vout " MFRAME_REPO    $MFRAME_REPO"
    vout " MFRAME_TOP     $MFRAME_TOP"
    vout " MFRAME_TGZ     $MFRAME_TGZ"
    vout " LIBTRNAV_REPO  $LIBTRNAV_REPO"
    vout " LIBTRNAV_TOP   $LIBTRNAV_TOP"
    vout " LIBTRNAV_TGZ   $LIBTRNAV_TGZ"
}

#################d
# Main entry point
processCmdLine "$@"

# define variables
DIST_DATE=$(date +%Y%m%dT%H%M)
TRNDEV_WD=${PWD}/mktrndev
TRNDEV_TGZ=trndev-${DIST_DATE}.tar.gz
TRNDEV_SDNAME=trndev-${DIST_DATE}
TRNDEV_SD=${TRNDEV_WD}/${TRNDEV_SDNAME}
MFRAME_REPO=mframe
MFRAME_TGZ=mframe.tar.gz
MFRAME_TOP=${TRNDEV_WD}/${MFRAME_REPO}
LIBTRNAV_REPO=libtrnav
LIBTRNAV_TGZ=libtrnav.tar.gz
LIBTRNAV_TOP=${TRNDEV_WD}/${LIBTRNAV_REPO}

showVars

# create working directory
vout "creating working directory ${TRNDEV_WD}"
mkdir -p ${TRNDEV_WD}

cd ${TRNDEV_WD}

# clone repositories
vout "cloning mframe"
${GIT} clone git@bitbucket.org:mbari/mframe.git
vout "cloning libtrnav"
${GIT} clone git@bitbucket.org:mbari/libtrnav.git

# build mframe distribution tar.gz
vout "building mframe dist"
cd ${MFRAME_TOP}
${MAKE} dist

# build libtrnav distribution tar.gz
vout "building libtrnav dist"
cd ${LIBTRNAV_TOP}
${MAKE} dist

# create staging directory
vout "staging package in ${TRNDEV_SD}"
cd ${TRNDEV_WD}
mkdir -p ${TRNDEV_SD}
cd ${TRNDEV_SD}

# stage the distribution tar.gz
vout "staging mframe"
${TAR} xzvf ${MFRAME_TOP}/bin/${MFRAME_TGZ}
vout "staging libtrnav"
${TAR} xzvf ${LIBTRNAV_TOP}/bin/${LIBTRNAV_TGZ}

# copy the README to the top level staging directory
vout "staging doc, scripts"
cp ${LIBTRNAV_REPO}/opt/sentry/README*md ${TRNDEV_SD}
cp ${LIBTRNAV_REPO}/opt/sentry/trndev-build.sh ${TRNDEV_SD}

# generate trndev package tar.gz
vout "generating trndev package archive ${TRNDEV_WD}/${TRNDEV_TGZ}"
cd ${TRNDEV_WD}
${TAR} czvf ${TRNDEV_TGZ} ${TRNDEV_SDNAME}

if [ "${KEEP_FILES}" != "Y" ]
then
    vout "Cleaning up build files"
    cd ${TRNDEV_WD}
    rm -rf ${MFRAME_REPO}
    rm -rf ${LIBTRNAV_REPO}
    rm -rf ${TRNDEV_SDNAME}
fi
