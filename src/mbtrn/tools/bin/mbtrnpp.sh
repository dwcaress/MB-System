#!/bin/bash

#########################################
# Name: mbtrnpp-run.sh
#
# Summary: Run mbtrnpp
#          (TRN pre-process)
#
# Description: runs mbtrnpp
# - automatically restarts after delay
# - may specify number of cycles (retries)
#
# See README
#
# Author: k headley
#
# Copyright MBARI 2018
#
#########################################

#########################################
# Script configuration defaults
# casual users should not need to change
# anything below this section
#########################################
DESCRIPTION="Run mbtrnpp; restart on exit"

LOOP_DELAY_SEC_DFL=5
let "CYCLES_DFL=-1"
SESSION_ID=`date +"%Y%m%d-%H%M%S"`
SESSION_NAME="mbtrnpp-console-${SESSION_ID}"
CONLOG="${SESSION_NAME}.log"
CONLOG_PATH_DFL="./"
unset MBTRNPP_ENV

# set environment defaults
# variables use environment values if set
# and may be overridden on the command line
# using an environment file or options
TRN_RESON_HOST=${TRN_RESON_HOST:-"localhost"}
TRN_HOST=${TRN_HOST:-"localhost"}
TRN_MBTRNDIR=${TRN_MBTRNDIR:-"/usr/local/bin"}
TRN_MAPFILES=${TRN_MAPFILES:-"./maps"}
TRN_DATAFILES=${TRN_DATAFILES:-"./config"}
TRN_LOGFILES=${TRN_LOGFILES:-"./logs/mbtrnpp"}

#################################
# Script variable initialization
#################################

# init_vars()
# initialize variables/set default options
# called *after* command line processing
# to support value overrides
init_vars(){
    # verbose wrapper script output
    VERBOSE="N"
    # delay before restarting app (s)
    LOOP_DELAY_SEC=${LOOP_DELAY_SEC_DFL}
    # console log path
    CONLOG_PATH=${CONLOG_PATH:-"$CONLOG_PATH_DFL"}
    # number of cycles (app restarts)
    # use -1 to loop indefinitely
    let CYCLES="${CYCLES_DFL}"

    # application path
	APP_CMD="${APP_CMD:-${TRN_MBTRNDIR}/mbtrnpp}"

#    # mbtrnpp config file
#    OPT_CONFIG=""

#    # log directory
#    #    OPT_LOGDIR="--log-directory=/home/mappingauv/mbtrnpptest"
#    OPT_LOGDIR="--log-directory=${TRN_LOGFILES}"
#
#    # verbose level (-5 to +2)
#    # +2 is a LOT of info
#    # -2 to 0 recommended for missions
#    # <= -3 a lot of mbtrn output
#    OPT_VERBOSE="--verbose=0"
#
#    # input source
#    # Define socket input
#    #   example: --input=socket:192.168.100.113:239.255.0.1:6020
#    #              IP of host running mbtrnpp : 192.168.100.113
#    #              Broadcast group : 239.255.0.1
#    #              Multibeam multicast port : 6020
#    # KONGSBERG_HOST="192.168.100.113"
#    # KONGSBERG_BCAST="239.255.0.1"
#    # KONGSBERG_PORT="6020"
#    # OPT_INPUT="--input=socket:${KONGSBERG_HOST}:${KONGSBERG_BCAST}:${KONGSBERG_PORT}"
#    # RESON1  134.89.32.107
#    # RESON2  134.89.32.108
#    # RESON3  134.89.32.110
#    # RESON_PORT 7000
#    # RESON_SIZE 0:default or size in bytes
#    OPT_INPUT="--input=socket:${TRN_RESON_HOST}:7000:0"
#
#    # output destination
#    # [alternatively, use --mb-out]
#    # --output     select mb1 output (similar to mb-out file, mb1svr)
#    #   options: file,file:<name>,socket,socket:<host>:<port> [1]
#    #    file                  - mb1 data out, mbsys default file name
#    #    file:<name>           - mb1 data out, specified file name
#    #    socket                - mb1 socket output, default host:port
#    #    socket:<host>:<port>  - mb1 socket output, specified host:port
#    OPT_OUTPUT="--output=file:mbtrnpp_${SESSION_ID}.mb1"
#    # beam swath (deg)
#    OPT_SWATH="--swath=90"
#    # number of beams to publish
#    OPT_SOUNDINGS="--soundings=11"
#    # input data format
#    # [do not use with datalist input]
#    # FORMAT_RESON 88
#    # FORMAT_EMKM 261
#    OPT_FORMAT="--format=88"
#    # median filter settings
#    OPT_MFILTER="--median-filter=0.10/9/3"
#
#
#    # enable/disable TRN processing
#    # (requires map,par,log,cfg)
#    OPT_TRN_EN="--trn-en"
#    OPT_TRN_DIS="--trn-dis"
#    OPT_TRN_SEL=${OPT_TRN_EN}
#
#    # set TRN UTM zone
#    # Monterey Bay 10
#    # Axial        12? 9?
#    OPT_TRN_UTM="--trn-utm=10"
#    # set TRN map file (required w/ TRN_EN)
#    OPT_TRN_MAP="--trn-map=${TRN_MAPFILES}/PortTiles"
#    # set TRN particles file (required w/ TRN_EN)
#    OPT_TRN_PAR="--trn-par=${TRN_DATAFILES}/particles.cfg"
#    # set TRN config file (required w/ TRN_EN)
#    OPT_TRN_CFG="--trn-cfg=${TRN_DATAFILES}/mappingAUV_specs.cfg"
#    # set TRN log directory prefix (required w/ TRN_EN)
#    #OPT_TRN_LOG="--trn-log=mbtrnpp"
#    OPT_TRN_MID="--trn-mid=mb-TRN_SESSION"
#    # TRN processing decimation (cycles)
#    OPT_TRN_DECN="--trn-decn=9"
#    # TRN processing decimation (sec)
#    #OPT_TRN_DECS="--trn-decs=0"
#    # set TRN map type
#    # TRN_MAP_DEM  1
#    # TRN_MAP_BO   2 (dfl)
#    OPT_TRN_MTYPE="--trn-mtype=2"
#    # set TRN map type
#    # TRN_FILT_NONE       0
#    # TRN_FILT_POINTMASS  1
#    # TRN_FILT_PARTICLE   2 (dfl)
#    # TRN_FILT_BANK       3
#    OPT_TRN_FTYPE="--trn-ftype=2"
#    OPT_TRN_FGRADE="--trn-fgrade=1"
#    OPT_TRN_MWEIGHT="--trn-mweight=4"
#    # set TRN valid covariance limits
#    OPT_TRN_NCOV="--trn-ncov=49.0"
#    OPT_TRN_NERR="--trn-nerr=200.0"
#    OPT_TRN_ECOV="--trn-ecov=49.0"
#    OPT_TRN_EERR="--trn-eerr=200.0"
#
#    OPT_TRN_REINIT="--trn-reinit=1"
#    OPT_REINIT_GAIN="" #--reinit-gain"
#    # ignore multibeam gain threshold
#    #OPT_TRN_NOMBGAIN="--trn-nombgain"
#    OPT_REINIT_FILE="--reinit-file=1"
#    OPT_REINIT_XYOFFSET="--reinit-xyoffset=150.0"
#    OPT_REINIT_ZOFFSET="--reinit-zoffset=2.0/2.0"
#    OPT_COVARIANCE_MAGNITUDE_MAX="--covariance-magnitude-max=5.0"
#    OPT_CONVERGENCE_REPEAT_MIN="--convergence-repeat-min=200"
#
#    # TRN output selection
#    #--trn-out=[options] select trn update output channels
#    #  options: trnsvr:<host>:<port>,mlog,stdout,stderr,debug [3]
#    #   trnsvr:<host>:<port>  - trn_server socket
#    #   trnusvr:<host>:<port> - trnu_server socket
#    #   trnu                  - trn estimate log
#    #   sout|serr             - console (independent of debug settings)
#    #   debug                 - per debug settings
#    # RESON1  134.89.32.107
#    # RESON2  134.89.32.108
#    # RESON3  134.89.32.110
#    # TRNSVR_PORT  28000
#    # TRNUSVR_PORT 8000
#    OPT_TRNOUT="--trn-out=trnsvr:${TRN_HOST}:28000,trnu"
#    OPT_TRNOUT="--trn-out=trnsvr:${TRN_HOST}:28000,trnu,trnusvr:${TRN_HOST}:8000"
#    # drop TRN clients after OPT_MBHBT seconds
#    OPT_TRNHBT="--trnhbt=30"
#    # drop TRNU clients after OPT_MBHBT seconds
#    OPT_TRNUHBT="--trnuhbt=30"
#
#    # MB1 output selection
#    # --mb-out=[options]  select mb1 output (mbsvr:host:port,mb1,reson/file)
#    #   options: mb1svr:host:port - MB1 socket (e.g. MbtrnRecv) [2]
#    #    mb1              - mb1 data log
#    #    file             - mb1 data log, use mbsys default file name
#    #    file:<name>      - mb1 data log, use specified file name
#    #    reson            - reson frame (s7k)log
#    #    nomb1            - disable mb1 log (if enabled by default in mbtrnpp)
#    #    noreson          - disable reson log
#    # RESON1  134.89.32.107
#    # RESON2  134.89.32.108
#    # RESON3  134.89.32.110
#    # MB1SVR_PORT 27000
#    OPT_MBOUT="--mb-out=mb1svr:${TRN_HOST}:27000"
#    # drop MB1 clients after hbeat messages
#    # if they haven't renewed
#    #OPT_MBHBN="--mbhbn=0"
#    # drop MB1 clients after OPT_MBHBT seconds
#    OPT_MBHBT="--mbhbt=30"
#
#    # delay between TRN messages (msec)
#    #OPT_DELAY="--delay=0"
#    # statistics logging interval (decimal sec)
#    OPT_STATSEC="--statsec=30"
    # statistics flags
    # enable stats processing options
    # may include
    # MSF_STATUS : status counters
    # MSF_EVENT  : event/error counters
    # MSF_ASTAT  : aggregate stats
    # MSF_PSTAT  : periodic stats
    # MSF_READER : r7kr reader stats
    #OPT_STATFLAGS="--statflags=MSF_STATUS:MSF_EVENT:MSF_ASTAT:MSF_PSTAT"
    #OPT_DELAY="--delay=0"

	# print help and exit
    OPT_HELP="" #"--help"
}

unset DO_HELP
unset DO_TEST
unset DO_CONLOG

###############################################
# End user configuration options
###############################################

#################################
# Function Definitions
#################################
#################################
# name: printUsage
# description: print use message
# args: none
#################################
printUsage(){
    echo
    echo " Description: $DESCRIPTION"
    echo
    echo " use: `basename $0` [options] [-- --option=value...]"
    echo " Options:"
    echo "  -a cmd  : app command            [$APP_CMD]"
    echo "  -c n    : cycles (<=0 forever)   [$CYCLES]"
    echo "  -d path : enable console log, set directory"
    echo "  -e path : environment file"
    echo "  -h      : print use message"
    echo "  -m path : override mbtrnpp dir   [$TRN_MBTRNDIR]"
    echo "  -o addr : override local host    [$TRN_HOST]"
    echo "             affects : [--trn-out, --mb-out]"
    echo "  -r addr : override reson host    [$TRN_RESON_HOST]"
    echo "             affects: [--input]"
    echo "  -t      : test [print cmdline]"
    echo "  -v      : verbose output         [$VERBOSE]"
    echo "  -w n    : loop delay (s)         [$LOOP_DELAY_SEC]"
    echo ""
    echo " Example:"
    echo " # typical use [Note '--' marking start of mbtrnpp options]"
    echo "  `basename $0` -e /path/to/environment/file -- --config=/path/to/config/file"
    echo ""
    echo " # log console output [-d <dir> creates <dir>/mbtrnpp-console-<session>.log]"
    echo "  `basename $0` -e /path/to/environment/file -d /path/to/console/output/dir -- --config=/path/to/config/file"
    echo ""
	echo " # test: show environment and mbtrnpp command line exit"
    echo "  `basename $0` -e /path/to/environment/file -t -- --config=/path/to/config/file [options...]"
	echo ""
	echo " # test: show all mbtrnpp parsed command line options and exit"
    echo "  `basename $0` -e /path/to/environment/file -- --config=/path/to/config/file [options...] --help"
    echo ""
    echo " Environment variables:"
    echo "  TRN_MAPFILES   - TRN map directory       [--trn-maps]"
    echo "  TRN_CFGFILES   - TRN config directory    [--trn-cfg, --trn-par]"
    echo "  TRN_LOGFILES   - TRN log directory       [--trn-log]"
    echo "  TRN_RESON_HOST - TRN reson (or emu7k) IP [--input]"
    echo "  TRN_HOST       - TRN server IP           [--trn-out, --mb-out]"
    echo "  TRN_MBTRNDIR   - mbtrnpp directory       [mbtrnpp path]"
    echo ""
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
# name: exitError
# description: print use message to stderr
# args:
#     msg:        error message
#     returnCode: exit status to return
########################################
exitError(){
    echo >&2
    echo "`basename $0`: error - $1" >&2
    echo >&2
    exit $2
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
    #    vout "`basename $0` all args[$*]"

while getopts a:c:d:e:hm:o:r:tvw: Option
    do
        #    vout "processing $Option[$OPTARG]"
        case $Option in
        a ) APP_CMD=$OPTARG
        ;;
        c ) let "CYCLES=$OPTARG"
        ;;
        d ) DO_CONLOG="Y"
        CONLOG_PATH=$OPTARG
        ;;
        e ) MBTRNPP_ENV=$OPTARG
        ;;
        h) DO_HELP="Y"
        ;;
        m ) TRN_MBTRNDIR=$OPTARG
        ;;
        o ) TRN_HOST=$OPTARG
        ;;
        r ) TRN_RESON_HOST=$OPTARG
        ;;
        t ) DO_TEST="Y"
        ;;
        v ) VERBOSE="Y"
        ;;
        w ) LOOP_DELAY_SEC=$OPTARG
        ;;
        *) exit 0 # getopts outputs error message
        ;;
        esac
    done
}

##########################
# Script main entry point
##########################

# Argument processing
# Accepts arguments from command line
# or pipe/file redirect
# Command line settings override config
# file settings

# pre-process cmdline to get env file
processCmdLine "$@"

# apply TRN environment file
if [ "${MBTRNPP_ENV}" ]
then
source ${MBTRNPP_ENV}
fi

# process command line args (may override env)
if [ "$#" -eq 0 ];then
    printUsage
    #  exit -1
    else
    # this ensures that quoted whitespace is preserved by getopts
    # note use of $@, in quotes

    # originally was processCmdLine $*
    processCmdLine "$@"
    let "i=$OPTIND-1"
    while [ "${i}" -gt 0  ]
    do
        shift
        let "i-=1"
    done
fi

echo ""

# call init vars
# delaying variable init until after command line processing
# enables command line overrides (e.g. see TRN_RESON_HOST)
init_vars

if [ ! -z ${DO_HELP} ] && [ ${DO_HELP} == "Y" ]
then
    printUsage
    if [ -f ${APP_CMD} ]
    then
        echo "Application use:"
        ${APP_CMD} --help
    fi
    exit 0
fi


declare -a v=("$@")
for a in "${v[@]}"
do
    if [ ${a:2:3} == "cmd" ]
    then
    APP_CMD=${a:6}
    vout "ovr APP_CMD: $APP_CMD"
    fi

    if [ ${a:2:13} == "log-directory" ]
    then
    OPT_LOGDIR=$a
    vout "ovr OPT_LOGDIR: $OPT_LOGDIR"
    fi

    if [ ${a:2:6} == "config" ]
    then
    OPT_CONFIG=$a
    vout "ovr OPT_CONFIG: $OPT_CONFIG"
    fi

    if [ ${a:2:7} == "verbose" ]
    then
    OPT_VERBOSE=$a
    vout "ovr OPT_VERBOSE: $OPT_VERBOSE"
    fi

    if [ ${a:2:5} == "input" ]
    then
    OPT_INPUT=$a
    vout "ovr OPT_INPUT: $OPT_INPUT"
    fi

    if [ ${a:2:6} == "output" ]
    then
    OPT_OUTPUT=$a
    vout "ovr OPT_OUTPUT: $OPT_OUTPUT"
    fi

    if [ ${a:2:10} == "tide-model" ]
    then
    OPT_TIDE=$a
    vout "ovr OPT_TIDE: $OPT_TIDE"
    fi

    if [ ${a:2:11} == "swath-width" ]
    then
    OPT_SWATH=$a
    vout "ovr OPT_SWATH: $OPT_SWATH"
    fi

    if [ ${a:2:9} == "soundings" ]
    then
    OPT_SOUNDINGS=$a
    vout "ovr OPT_SOUNDINGS: $OPT_SOUNDINGS"
    fi

    if [ ${a:2:6} == "format" ]
    then
    OPT_FORMAT=$a
    vout "ovr OPT_FORMAT: $OPT_FORMAT"
    fi

    if [ ${a:2:13} == "median-filter" ]
    then
    OPT_MFILTER=$a
    vout "ovr OPT_MFILTER: $OPT_MFILTER"
    fi

    if [ ${a:2:6} == "trn-en" ]
    then
    OPT_TRN_SEL=${OPT_TRN_EN}
    vout "ovr OPT_TRN_SEL: $OPT_TRN_SEL"
    fi

    if [ ${a:2:7} == "trn-dis" ]
    then
    OPT_TRN_SEL=${OPT_TRN_DIS}
    vout "ovr OPT_TRN_SEL: $OPT_TRN_SEL"
    fi

    if [ ${a:2:7} == "trn-utm" ]
    then
    OPT_TRN_UTM=$a
    vout "ovr OPT_TRN_UTM: $OPT_TRN_UTM"
    fi

    if [ ${a:2:7} == "trn-map" ]
    then
    OPT_TRN_MAP=$a
    vout "ovr OPT_TRN_MAP: $OPT_TRN_MAP"
    fi

    if [ ${a:2:7} == "trn-par" ]
    then
    OPT_TRN_PAR=$a
    vout "ovr OPT_TRN_PAR: $OPT_TRN_PAR"
    fi

    if [ ${a:2:7} == "trn-cfg" ]
    then
    OPT_TRN_CFG=$a
    vout "ovr OPT_TRN_CFG: $OPT_TRN_CFG"
    fi

    if [ ${a:2:7} == "trn-mid" ]
    then
    OPT_TRN_MID=$a
    vout "ovr OPT_TRN_MID: $OPT_TRN_MID"
    fi


    if [ ${a:2:8} == "trn-decn" ]
    then
    OPT_TRN_DECN=$a
    vout "ovr OPT_TRN_DECN: $OPT_TRN_DECN"
    fi

    if [ ${a:2:8} == "trn-decs" ]
    then
    OPT_TRN_DECS=$a
    vout "ovr OPT_TRN_DECS: $OPT_TRN_DECS"
    fi

    if [ ${a:2:9} == "trn-mtype" ]
    then
    OPT_TRN_MTYPE=$a
    vout "ovr OPT_TRN_MTYPE: $OPT_TRN_MTYPE"
    fi

    if [ ${a:2:9} == "trn-ftype" ]
    then
    OPT_TRN_FTYPE=$a
    vout "ovr OPT_TRN_FTYPE: $OPT_TRN_FTYPE"
    fi

    if [ ${a:2:10} == "trn-fgrade" ]
    then
    OPT_TRN_FGRADE=$a
    vout "ovr OPT_TRN_FGRADE: $OPT_TRN_FGRADE"
    fi

    if [ ${a:2:11} == "trn-mweight" ]
    then
    OPT_TRN_MWEIGHT=$a
    vout "ovr OPT_TRN_MWEIGHT: $OPT_TRN_MWEIGHT"
    fi

    if [ ${a:2:8} == "trn-ncov" ]
    then
    OPT_TRN_NCOV=$a
    vout "ovr OPT_TRN_NCOV: $OPT_TRN_NCOV"
    fi

    if [ ${a:2:8} == "trn-nerr" ]
    then
    OPT_TRN_NERR=$a
    vout "ovr OPT_TRN_NERR: $OPT_TRN_NERR"
    fi

    if [ ${a:2:8} == "trn-ecov" ]
    then
    OPT_TRN_ECOV=$a
    vout "ovr OPT_TRN_ECOV: $OPT_TRN_ECOV"
    fi

    if [ ${a:2:8} == "trn-eerr" ]
    then
    OPT_TRN_EERR=$a
    vout "ovr OPT_TRN_EERR: $OPT_TRN_EERR"
    fi

    if [ ${a:2:10} == "trn-reinit" ]
    then
    OPT_TRN_REINIT=$a
    vout "ovr OPT_TRN_REINIT: $OPT_TRN_REINIT"
    fi

    if [ ${a:2:11} == "reinit-gain" ]
    then
    OPT_REINIT_GAIN=$a
    vout "ovr OPT_REINIT_GAIN: $OPT_REINIT_GAIN"
    fi

    if [ ${a:2:11} == "reinit-file" ]
    then
    OPT_REINIT_FILE=$a
    vout "ovr OPT_REINIT_FILE: $OPT_REINIT_FILE"
    fi

    if [ ${a:2:15} == "reinit-xyoffset" ]
    then
    OPT_REINIT_XYOFFSET=$a
    vout "ovr OPT_REINIT_XYOFFSET: $OPT_REINIT_XYOFFSET"
    fi

    if [ ${a:2:14} == "reinit-zoffset" ]
    then
    OPT_REINIT_ZOFFSET=$a
    vout "ovr OPT_REINIT_ZOFFSET: $OPT_REINIT_ZOFFSET"
    fi

    if [ ${a:2:24} == "covariance-magnitude-max" ]
    then
    OPT_COVARIANCE_MAGNITUDE_MAX=$a
    vout "ovr OPT_COVARIANCE_MAGNITUDE_MAX: $OPT_COVARIANCE_MAGNITUDE_MAX"
    fi

    if [ ${a:2:22} == "convergence-repeat-min" ]
    then
    OPT_CONVERGENCE_REPEAT_MIN=$a
    vout "ovr OPT_CONVERGENCE_REPEAT_MIN: $OPT_CONVERGENCE_REPEAT_MIN"
    fi

    if [ ${a:2:7} == "trn-out" ]
    then
    OPT_TRNOUT=$a
    vout "ovr OPT_TRNOUT: $OPT_TRNOUT"
    fi

    if [ ${a:2:6} == "trnhbt" ]
    then
    OPT_TRNHBT=$a
    vout "ovr OPT_TRNHBT: $OPT_TRNHBT"
    fi

    if [ ${a:2:7} == "trnuhbt" ]
    then
    OPT_TRNUHBT=$a
    vout "ovr OPT_TRNUHBT: $OPT_TRNUHBT"
    fi

    if [ ${a:2:6} == "mb-out" ]
    then
    OPT_MBOUT=$a
    vout "ovr OPT_MBOUT: $OPT_MBOUT"
    fi

    if [ ${a:2:5} == "mbhbt" ]
    then
    OPT_MBHBT=$a
    vout "ovr OPT_MBHBT: $OPT_MBHBT"
    fi


    if [ ${a:2:7} == "statsec" ]
    then
    OPT_STATSEC=$a
    vout "ovr OPT_STATSEC: $OPT_STATSEC"
    fi

    if [ ${a:2:9} == "statflags" ]
    then
    OPT_STATFLAGS=$a
    vout "ovr OPT_STATFLAGS: $OPT_STATFLAGS"
    fi

    if [ ${a:2:5} == "delay" ]
    then
    OPT_DELAY=$a
    vout "ovr OPT_DELAY: $OPT_DELAY"
    fi

    if [ ${a:2:4} == "help" ]
    then
    OPT_HELP=$a
    vout "ovr OPT_HELP: $OPT_HELP"
    fi

#    if [ ${a:2:7} == "trn-log" ]
#    then
#    OPT_TRN_LOG=$a
#    vout "ovr OPT_TRN_LOG: $OPT_TRN_LOG"
#    fi

#    if [ ${a:2:5} == "mbhbn" ]
#    then
#    OPT_MBHBN=$a
#    vout "ovr OPT_MBHBN: $OPT_MBHBN"
#    fi


#    if [ ${a:2:12} == "trn-nombgain" ]
#    then
#    OPT_TRN_NOMBGAIN=$a
#    vout "ovr OPT_TRN_NOMBGAIN: $OPT_TRN_NOMBGAIN"
#    fi

done

#$OPT_TRN_MID $OPT_MBHBN $OPT_TRN_NOMBGAIN

# set cmdline options
APP_OPTS="$OPT_CONFIG $OPT_LOGDIR  $OPT_VERBOSE $OPT_INPUT $OPT_OUTPUT $OPT_TIDE $OPT_SWATH $OPT_SOUNDINGS $OPT_FORMAT $OPT_MFILTER \
$OPT_TRN_SEL $OPT_TRN_UTM $OPT_TRN_MAP $OPT_TRN_PAR $OPT_TRN_CFG $OPT_TRN_MID $OPT_TRN_DECN $OPT_TRN_DECS \
$OPT_TRN_MTYPE $OPT_TRN_FTYPE $OPT_TRN_FGRADE $OPT_TRN_MWEIGHT $OPT_TRN_NCOV $OPT_TRN_NERR $OPT_TRN_ECOV $OPT_TRN_EERR \
$OPT_TRN_REINIT $OPT_REINIT_GAIN $OPT_REINIT_FILE $OPT_REINIT_XYOFFSET $OPT_REINIT_ZOFFSET \
$OPT_COVARIANCE_MAGNITUDE_MAX $OPT_CONVERGENCE_REPEAT_MIN $OPT_TRNOUT $OPT_TRNHBT $OPT_TRNUHBT $OPT_MBOUT \
$OPT_MBHBT $OPT_STATSEC $OPT_STATFLAGS  $OPT_DELAY $OPT_HELP"

# check required TRN options
if [ ! -z "${OPT_TRN_EN}" ]
then

    if [ -z "${OPT_TRN_MAP}" ]
    then
    echo "WARNING: --trn-map required with --trn-en"
    fi
    if [ -z "${OPT_TRN_CFG}" ]
    then
	echo "WARNING: --trn-cfg required with --trn-en"
    fi
    if [ -z "${OPT_TRN_PAR}" ]
    then
    echo "WARNING: --trn-par required with --trn-en"
    fi

fi

if [ ${DO_TEST} ]
then
    echo
	echo  "env:"
	env|grep TRN
	echo
	echo "cmdline:"
    echo  $APP_CMD $APP_OPTS
    echo
    exit 0
fi

vout "Using options:"
vout "config file  [$CFG_FILE]"
vout "env name     [$MBTRNPP_ENV]"
vout "cycles       [$CYCLES]"
vout "loop delay   [$LOOP_DELAY_SEC]"

if [ "${DO_CONLOG}" == "Y" ]
then
  vout "console log  [${CONLOG_PATH}/${CONLOG}]"]
else
  vout "console log  [N]"
fi

vout "cmdline:"
vout "$APP_CMD $APP_OPTS"
vout

# validate application options
if [ ! -f ${APP_CMD} ]
then
  exitError "executable not found [$APP_CMD]" 1
fi

if [ ! -d ${OPT_LOGDIR:15:} ]
then
  exitError "log directory not found [${OPT_LOGDIR:15:}]" 1
fi

if [ "${DO_CONLOG}" == "Y" ] && [ ! -d ${CONLOG_PATH} ]
then
    if [ -f ${CONLOG_PATH} ]
    then
      exitError "ERR - ${CONLOG_PATH} is not a directory"
    else
      vout "creating log dir ${CONLOG_PATH}"
      mkdir -p ${CONLOG_PATH}
        if [ ! -d ${CONLOG_PATH} ]
        then
            exitError "ERR - could not create log dir ${CONLOG_PATH}"
        fi
    fi
fi

# initialize cycle count
# [loop indefinitely if <0]
let "LOOP_COUNT=${CYCLES}"

while [ ${LOOP_COUNT} -ne 0 ]
do
    # run the app
    if [ "${DO_CONLOG}" == "Y" ]
    then
      $APP_CMD $APP_OPTS &> ${CONLOG_PATH}/${CONLOG}
    else
      $APP_CMD $APP_OPTS
    fi

    echo "`date` - $APP_CMD exited"

    if [  ! -z "${OPT_HELP}" ]
    then
	# exit if --help passed to app
    break
    fi

    # check stop condition (if not <0)
    if [ ${LOOP_COUNT} -ge 0 ]
    then
      let "LOOP_COUNT=${LOOP_COUNT}-1"
        if [ ${LOOP_COUNT} -eq 0 ]
        then
            vout "mbtrnpp - completed $CYCLES cycles - exiting"
            exit 0
        fi
    fi

    # wait and restart
    sleep ${LOOP_DELAY_SEC}
    vout "restarting $APP_CMD"
    vout
done
