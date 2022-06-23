#!/bin/bash

# Build the trndev packages as described in README-trndev-build.md

description="build/install trndev using gnu make"
VERBOSE="N"
DO_INSTALL="N"

MAKE_CMD=$(which make)

TRNDEV_TOP=${TRNDEV_TOP:-"${PWD}"}

TRNDEV_INSTALL=${TRNDEV_INSTALL:-${TRNDEV_TOP}/install}

MF_VERSION=${MF_VERSION:-"0.1.0"}
LT_VERSION=${LT_VERSION:-"0.1.0"}

MF_SRC_LIB=${TRNDEV_TOP}/mframe/bin
MF_DES_LIB=${TRNDEV_INSTALL}/lib
MF_SRC_BIN=${TRNDEV_TOP}/mframe/bin
MF_DES_BIN=${TRNDEV_INSTALL}/bin/mframe-${MF_VERSION}
MF_SRC_INC=${TRNDEV_TOP}/mframe/src
MF_DES_INC=${TRNDEV_INSTALL}/include/mframe-${MF_VERSION}

LT_SRC_LIB=${TRNDEV_TOP}/libtrnav/bin
LT_DES_LIB=${TRNDEV_INSTALL}/lib
LT_SRC_BIN=${TRNDEV_TOP}/libtrnav/bin
LT_DES_BIN=${TRNDEV_INSTALL}/bin/libtrnav-${LT_VERSION}
LT_SRC_INC=${TRNDEV_TOP}/libtrnav
LT_DES_INC=${TRNDEV_INSTALL}/include/libtrnav-${LT_VERSION}

MF_BINARIES="\
${MF_SRC_BIN}/medebug-test \
${MF_SRC_BIN}/mkvconf-test \
${MF_SRC_BIN}/mmdebug-test \
${MF_SRC_BIN}/msocket-test \
${MF_SRC_BIN}/mstats-test \
${MF_SRC_BIN}/mtime-test
"

MF_LIBS="\
${MF_SRC_LIB}/libmframe.a
"

MF_HEADERS="\
${MF_SRC_INC}/mbbuf.h \
${MF_SRC_INC}/mcbuf.h \
${MF_SRC_INC}/mconfig.h \
${MF_SRC_INC}/mdebug.h \
${MF_SRC_INC}/medebug.h \
${MF_SRC_INC}/merror.h \
${MF_SRC_INC}/mfile.h \
${MF_SRC_INC}/mframe.h \
${MF_SRC_INC}/mkvconf.h \
${MF_SRC_INC}/mlist.h \
${MF_SRC_INC}/mlog.h \
${MF_SRC_INC}/mmdebug.h \
${MF_SRC_INC}/mmem.h \
${MF_SRC_INC}/mqueue.h \
${MF_SRC_INC}/mserial.h \
${MF_SRC_INC}/msocket.h \
${MF_SRC_INC}/mstats.h \
${MF_SRC_INC}/mswap.h \
${MF_SRC_INC}/mthread.h \
${MF_SRC_INC}/mtime.h \
${MF_SRC_INC}/mutils.h \
${MF_SRC_INC}/sysq.h
"

LT_BINARIES="\
${LT_SRC_BIN}/trnucli-test \
${LT_SRC_BIN}/trnusvr-test \
${LT_SRC_BIN}/mcpub \
${LT_SRC_BIN}/mcsub \
${LT_SRC_BIN}/mmcpub \
${LT_SRC_BIN}/mmcsub \
${LT_SRC_BIN}/trn-server \
${LT_SRC_BIN}/otree
"

LT_LIBS="\
${LT_SRC_LIB}/libgeolib.a \
${LT_SRC_LIB}/libmb1.a \
${LT_SRC_LIB}/libnetif.a \
${LT_SRC_LIB}/libnewmat.a \
${LT_SRC_LIB}/libqnx.a \
${LT_SRC_LIB}/libtrn.a \
${LT_SRC_LIB}/libtrnw.a \
${LT_SRC_LIB}/libtrncli.a \
${LT_SRC_LIB}/libtrnucli.a
"

LT_HEADERS="\
${LT_SRC_INC}/qnx-utils/AngleData.h \
${LT_SRC_INC}/qnx-utils/AsciiFile.h \
${LT_SRC_INC}/qnx-utils/BinaryFile.h \
${LT_SRC_INC}/qnx-utils/CharData.h \
${LT_SRC_INC}/qnx-utils/DataField.h \
${LT_SRC_INC}/qnx-utils/DataFieldFactory.h \
${LT_SRC_INC}/qnx-utils/DataLog.h \
${LT_SRC_INC}/qnx-utils/DataLogReader.h \
${LT_SRC_INC}/qnx-utils/DataLogWriter.h \
${LT_SRC_INC}/qnx-utils/DoubleData.h \
${LT_SRC_INC}/qnx-utils/DynamicArray.h \
${LT_SRC_INC}/qnx-utils/Exception.h \
${LT_SRC_INC}/qnx-utils/ExternalData.h \
${LT_SRC_INC}/qnx-utils/FileData.h \
${LT_SRC_INC}/qnx-utils/FloatData.h \
${LT_SRC_INC}/qnx-utils/IntegerData.h \
${LT_SRC_INC}/qnx-utils/LogFile.h \
${LT_SRC_INC}/qnx-utils/MathP.h \
${LT_SRC_INC}/qnx-utils/NavUtils.h \
${LT_SRC_INC}/qnx-utils/ShortData.h \
${LT_SRC_INC}/qnx-utils/StringConverter.h \
${LT_SRC_INC}/qnx-utils/StringData.h \
${LT_SRC_INC}/qnx-utils/TimeIF.h \
${LT_SRC_INC}/qnx-utils/TimeP.h \
${LT_SRC_INC}/qnx-utils/TimeTag.h \
${LT_SRC_INC}/qnx-utils/ourTypes.h \
${LT_SRC_INC}/terrain-nav/Octree.hpp \
${LT_SRC_INC}/terrain-nav/OctreeSupport.hpp \
${LT_SRC_INC}/terrain-nav/OctreeTestCode.h \
${LT_SRC_INC}/terrain-nav/PositionLog.h \
${LT_SRC_INC}/terrain-nav/TNavBankFilter.h \
${LT_SRC_INC}/terrain-nav/TNavConfig.h \
${LT_SRC_INC}/terrain-nav/TNavExtendKalmanFilter.h \
${LT_SRC_INC}/terrain-nav/TNavFilter.h \
${LT_SRC_INC}/terrain-nav/TNavPFLog.h \
${LT_SRC_INC}/terrain-nav/TNavParticleFilter.h \
${LT_SRC_INC}/terrain-nav/TNavPointMassFilter.h \
${LT_SRC_INC}/terrain-nav/TNavSigmaPointFilter.h \
${LT_SRC_INC}/terrain-nav/TRNUtils.h \
${LT_SRC_INC}/terrain-nav/TerrainMap.h \
${LT_SRC_INC}/terrain-nav/TerrainMapDEM.h \
${LT_SRC_INC}/terrain-nav/TerrainMapOctree.h \
${LT_SRC_INC}/terrain-nav/TerrainNav.h \
${LT_SRC_INC}/terrain-nav/TerrainNavLog.h \
${LT_SRC_INC}/terrain-nav/genFilterDefs.h \
${LT_SRC_INC}/terrain-nav/mapio.h \
${LT_SRC_INC}/terrain-nav/matrixArrayCalcs.h \
${LT_SRC_INC}/terrain-nav/myOutput.h \
${LT_SRC_INC}/terrain-nav/particleFilterDefs.h \
${LT_SRC_INC}/terrain-nav/structDefs.h \
${LT_SRC_INC}/terrain-nav/trn_common.h \
${LT_SRC_INC}/terrain-nav/trn_log.h \
${LT_SRC_INC}/trnw/mb1_dmsg.h \
${LT_SRC_INC}/trnw/mb1_msg.h \
${LT_SRC_INC}/trnw/mb1rs.h \
${LT_SRC_INC}/trnw/netif.h \
${LT_SRC_INC}/trnw/trn_cli.h \
${LT_SRC_INC}/trnw/trn_msg.h \
${LT_SRC_INC}/trnw/trnif_msg.h \
${LT_SRC_INC}/trnw/trnif_proto.h \
${LT_SRC_INC}/trnw/trnu_cli.h \
${LT_SRC_INC}/trnw/trnw.h
"

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
    shift
    targets=$*
    cd $loc
    ${MAKE_CMD} ${targets}
}

########################################
# name: install_mkdir
# description: make installation directories
# args:none
# returnCode: none
########################################
install_mkdir(){

    if [ ! -d ${MF_DES_LIB} ]
    then
        mkdir -p ${MF_DES_LIB}
    fi
    if [ ! -d ${MF_DES_BIN} ]
    then
        mkdir -p ${MF_DES_BIN}
    fi
    if [ ! -d ${MF_DES_INC} ]
    then
        mkdir -p ${MF_DES_INC}
    fi

    if [ ! -d ${LT_DES_LIB} ]
    then
        mkdir -p ${MF_DES_LIB}
    fi
    if [ ! -d ${LT_DES_BIN} ]
    then
        mkdir -p ${LT_DES_BIN}
    fi
    if [ ! -d ${LT_DES_INC} ]
    then
        mkdir -p ${LT_DES_INC}
    fi
}

########################################
# name: install_x
# description: install package files to dest
# args:
#   src: space-delimited list (quoted)
#   dest: directory path
# returnCode: none
########################################
install_x(){
    src=$1
    dest=$2

    IFS=' ' read -ra arr <<< ${src}
    for x in ${arr[*]}
    do
    vout "copying ${x} to ${dest}"
    cp $x ${dest}/.
    done

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

vout "building mframe"
build_pkg ${TRNDEV_TOP}/mframe all

vout "building libtrnav"
build_pkg ${TRNDEV_TOP}/libtrnav all trnc


if [ ${DO_INSTALL} == "Y" ]
then
    if [ ! -d ${TRNDEV_INSTALL} ]
    then
        mkdir -p ${TRNDEV_INSTALL}
    fi
    vout "installing to ${TRNDEV_INSTALL}"

    install_mkdir
    install_x "${MF_LIBS}" ${MF_DES_LIB}
    install_x "${MF_BINARIES}" ${MF_DES_BIN}
    install_x "${MF_HEADERS}" ${MF_DES_INC}
    install_x "${LT_LIBS}" ${LT_DES_LIB}
    install_x "${LT_BINARIES}" ${LT_DES_BIN}
    install_x "${LT_HEADERS}" ${LT_DES_INC}
fi
