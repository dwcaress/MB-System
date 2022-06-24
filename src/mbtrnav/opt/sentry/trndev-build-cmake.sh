#!/bin/bash

# Build the trndev packages as described in README-trndev-build.md

description="build/install trndev using cmake"
VERBOSE="N"
DO_INSTALL="N"

MAKE_CMD=$(which cmake)

TRNDEV_TOP=$PWD

TRNDEV_INSTALL=${TRNDEV_INSTALL:-${TRNDEV_TOP}/install}

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

#################################
# name: printUsage
# description: print use message
# args: none
#################################
printUsage(){
    echo
    echo " Description: $description"
    echo
    echo " usage: `basename $0` [options]"
    echo " Options:"
    echo "  -I     : install to default directory [${TRNDEV_INSTALL}]"
    echo "  -i <s> : install to specifiec directory"
    echo "  -h     : help message"
    echo "  -v     : verbose output"
    echo
    echo
}

########################################
# name: processCmdLine
# description: do command line processing
# args:
#     args:       positional parameters
#     returnCode: none
########################################
processCmdLine(){
    OPTIND=1
    vout "`basename $0` all args[$*]"

    while getopts Ii:hv Option
    do
        case $Option in
            I ) DO_INSTALL="Y"
            ;;
            i ) TRNDEV_INSTALL=${OPTARG}
                DO_INSTALL="Y"
            ;;
            v ) VERBOSE="Y"
            ;;
            h ) printUsage
                exit 0
            ;;
            * ) exit 0 # getopts outputs error message
            ;;
        esac
    done
}

########################################
# name: build_pkg
# description: build a package
# args:
#  loc    : build directory
#  targets: list of targets to build
# returnCode: none
########################################
build_pkg(){
    loc=$1

    if [ ! -d ${loc} ]
    then
        mkdir -p ${loc}
    fi
    cd $loc
    ${MAKE_CMD} ..
    ${MAKE_CMD} --build .
    # install a copy in the build directory
    ${MAKE_CMD} --install . --prefix `pwd`/pkg
}

########################################
# name: install_x
# description: install package files to dest
# args:
#   src: build location
#   dest: destination path
# returnCode: none
########################################
install_x(){
    src=$1
    dest=$2
    cd $src
    ${MAKE_CMD} --install . --prefix ${dest}
}

# Script main entry point

# command line option processing
processCmdLine "$@"

if [ ! -d mframe ]
then
echo " missing mframe directory"
exit -1
fi

if [ ! -d libtrnav ]
then
echo " missing libtrnav directory"
exit -1
fi

build_pkg ${TRNDEV_TOP}/mframe/build
build_pkg ${TRNDEV_TOP}/libtrnav/build

if [ ${DO_INSTALL} == "Y" ]
then
    if [ ! -d ${TRNDEV_INSTALL} ]
    then
        mkdir -p ${TRNDEV_INSTALL}
    fi
    echo "installing to ${TRNDEV_INSTALL}"

    install_x ${TRNDEV_TOP}/mframe/build ${TRNDEV_INSTALL}
    install_x ${TRNDEV_TOP}/libtrnav/build ${TRNDEV_INSTALL}
fi
