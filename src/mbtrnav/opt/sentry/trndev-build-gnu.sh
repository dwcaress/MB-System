#!/bin/bash

# Build the trndev packages as described in README-trndev-build.md

description="build/install trndev using gnu make"
VERBOSE="N"
DO_INSTALL="N"
DO_UNINSTALL="N"
DO_BUILD="Y"

MAKE_CMD=${MAKE_CMD:-$(which make)}

#TRNDEV_TOP=${TRNDEV_TOP:-"${PWD}"}
TRNDEV_TOP=$PWD

DESTDIR=${DESTDIR:-""}
PREFIX=${PREFIX:-"/usr/local"}

declare -a TRN_CMAKE_OPTS
declare -a MF_CMAKE_OPTS

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
    echo "  -m <s> : libmframe cmake options"
    echo "  -t <s> : libtrnav cmake options"
    echo "  -h     : help message"
    echo "  -v     : verbose output"
    echo
    echo "Use Notes:"
    echo " This script is typically run twice:"
    echo "   build (optionally using -t, -m)"
    echo "   then"
    echo "   install (using -i, optionally -p, -d)"
    echo "   or"
    echo "   uninstall (using -u)"
    echo
    echo "   When uninstalling, use the same -p, -d options"
    echo "   used to install"
    echo
    echo "Examples:"
    echo
    echo " # build"
    echo "   `basename $0`"
    echo
    echo " # build using cmake TRN options"
    echo " # environment options should precede targets"
    echo "   `basename $0` -t \"buildUseProj=1\" -t \"libs\" -t \"trnc\" -t \"rov\""
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

#./trndev-build-gnu.sh -v -t "PROJ_LIB_DIR=-L/usr/lib/x86_64-linux-gnu" -t "PROJ_INC_DIR=-L/usr/include" -t "buildUseProj=1" -t "clean" -t "libs" -t "trnc" -t "rov"

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

    while getopts d:him:p:t:uv Option
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
            t ) TRN_CMAKE_OPTS[${#TRN_CMAKE_OPTS[@]}]=${OPTARG}
            ;;
            m ) MF_CMAKE_OPTS[${#MF_CMAKE_OPTS[@]}]=${OPTARG}
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
    shift
    vout "make targets: [$#] $*"
    cd $loc
    ${MAKE_CMD} $*
}

########################################
# name: install_x
# description: install package files to dest
# args:
#   src: space-delimited list (quoted)
#   dest: DESTDIR
#   prefix: PREFIX
# returnCode: none
########################################
install_x(){
    src=$1
    destdir=$2
    prefix=$3

    OPT_DEST=
    if [ ! -z destdir ]
    then
        OPT_DEST="DESTDIR=${destdir}"
    fi

    OPT_PREFIX=
    if [ ! -z prefix ]
    then
        OPT_PREFIX="PREFIX=${prefix}"
    fi

    cd $src
    ${MAKE_CMD} ${OPT_DEST} ${OPT_PREFIX} install
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

    OPT_DEST=
    if [ ! -z destdir ]
    then
        OPT_DEST="DESTDIR=${destdir}"
    fi

    OPT_PREFIX=
    if [ ! -z prefix ]
    then
        OPT_PREFIX="PREFIX=${prefix}"
    fi

    cd $src
    ${MAKE_CMD} ${OPT_DEST} ${OPT_PREFIX} uninstall
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
    build_pkg ${TRNDEV_TOP}/mframe ${MF_CMAKE_OPTS[@]} all

    vout "building libtrnav"
    build_pkg ${TRNDEV_TOP}/libtrnav ${TRN_CMAKE_OPTS[@]}
fi

if [ ${DO_INSTALL} == "Y" ]
then
    if [ ! -d ${DESTDIR} ]
    then
        mkdir -p ${DESTDIR}
    fi
    vout "installing to ${DESTDIR}/${PREFIX}"

    install_x ${TRNDEV_TOP}/mframe ${DESTDIR} ${PREFIX}
    install_x ${TRNDEV_TOP}/libtrnav ${DESTDIR} ${PREFIX}
fi

if [ ${DO_UNINSTALL} == "Y" ]
then
    vout "uninstalling from ${DESTDIR}/${PREFIX}"

    uninstall_x ${TRNDEV_TOP}/mframe ${DESTDIR} ${PREFIX}
    uninstall_x ${TRNDEV_TOP}/libtrnav ${DESTDIR} ${PREFIX}
fi
