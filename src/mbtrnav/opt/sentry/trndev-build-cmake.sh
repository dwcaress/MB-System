#!/bin/bash

# Build the trndev packages as described in README-trndev-build.md

description="build/install trndev using cmake"
VERBOSE="N"
DO_INSTALL="N"
DO_UNINSTALL="N"
DO_BUILD="Y"

MAKE_CMD=${MAKE_CMD:-$(which cmake)}

TRNDEV_TOP=$PWD

DESTDIR=${DESTDIR:-""}
PREFIX=${PREFIX:-"/usr/local"}

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
    echo "  -d <s> : set DESTDIR for install/uninstall [${DESTDIR}]"
    echo "  -p <s> : set PREFIX for install/uninstall [${PREFIX}]"
    echo "  -i     : install"
    echo "  -u     : uninstall"
    echo "  -h     : help message"
    echo "  -v     : verbose output"
    echo
    echo "Examples:"
    echo
    echo " # build"
    echo "   `basename $0`"
    echo
    echo " # install to default directory ${DESTDIR}${PREFIX}"
    echo "   sudo `basename $0` -i"
    echo
    echo " # uninstall from default directory ${DESTDIR}${PREFIX}"
    echo "   sudo `basename $0` -u"
    echo
    echo " # install to staging directory w/ default PREFIX (as non-root)"
    echo "   `basename $0` -id \$PWD/stage"
    echo
    echo " # uninstall from staging directory w/ custom PREFIX (as non-root)"
    echo "   `basename $0` -ud \$PWD/stage -p /foo/bar"
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

    while getopts d:hip:uv Option
    do
        case $Option in
            d ) DESTDIR=${OPTARG}
            ;;
            i ) DO_INSTALL="Y"
                DO_BUILD="N"
            ;;
            p ) PREFIX=${OPTARG}
            ;;
            u ) DO_UNINSTALL="Y"
                DO_BUILD="N"
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
#   dest: DESTDIR
#   prefix: PREFIX
# returnCode: none
########################################
install_x(){
    src=$1
    destdir=$2
    prefix=$3
    cd $src
    ${MAKE_CMD} --install . --prefix ${destdir}/${prefix}
}

########################################
# name: uninstall_x
# description: install package files to dest
# args:
#   src: space-delimited list (quoted)
#   dest: DESTDIR
#   prefix: PREFIX
# returnCode: none
########################################
uninstall_x(){
    src=$1
    destdir=$2
    prefix=$3
    cd $src
    if [ -f "./install_manifest.txt" ]
    then
        xargs rm < ./install_manifest.txt
    else
        echo "install_manifest not found in ${PWD}"
    fi
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

if [ ${DO_BUILD} == "Y" ]
then
    vout "building mframe"
    build_pkg ${TRNDEV_TOP}/mframe/build
    vout "building libtrnav"
    build_pkg ${TRNDEV_TOP}/libtrnav/build
fi

if [ ${DO_INSTALL} == "Y" ]
then
    if [ ! -d ${DESTDIR} ]
    then
        mkdir -p ${DESTDIR}
    fi
    echo "installing to ${DESTDIR}/${PREFIX}"

    install_x ${TRNDEV_TOP}/mframe/build ${DESTDIR} ${PREFIX}
    install_x ${TRNDEV_TOP}/libtrnav/build ${DESTDIR} ${PREFIX}
fi

if [ ${DO_UNINSTALL} == "Y" ]
then
    vout "uninstalling from ${DESTDIR}/${PREFIX}"

    uninstall_x ${TRNDEV_TOP}/mframe/build ${DESTDIR} ${PREFIX}
    uninstall_x ${TRNDEV_TOP}/libtrnav/build ${DESTDIR} ${PREFIX}
fi
