#!/bin/bash

#########################################
# Name: mbtrnpp.sh
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

#################################
# Script variable initialization
#################################
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
APP_CMD="/usr/local/bin/mbtrnpp"
# log directory
OPT_LOGDIR="--log-directory=/home/mappingauv/mbtrnpptest"
# input source
# Define socket input
#   example: --input=socket:192.168.100.113:239.255.0.1:6020
#              IP of host running mbtrnpp : 192.168.100.113
#              Broadcast group : 239.255.0.1
#              Multibeam multicast port : 6020
# OPT_INPUT="--input=socket:192.168.100.113:239.255.0.1:6020"
OPT_INPUT="--input=socket:192.168.100.113:239.255.0.1:6020"

# output destination
# [alternatively, use --mb-out]
# --output     select mb1 output (similar to mb-out file, mb1svr)
#   options: file,file:<name>,socket,socket:<host>:<port> [1]
#    file                  - mb1 data out, mbsys default file name
#    file:<name>           - mb1 data out, specified file name
#    socket                - mb1 socket output, default host:port
#    socket:<host>:<port>  - mb1 socket output, specified host:port
OPT_OUTPUT="--output=file:mbtrnpp_$$.mb1"
# beam swath (deg)
OPT_SWATH="--swath=90"
# number of beams to publish
OPT_SOUNDINGS="--soundings=21"
# input data format
# [do not use with datalist input]
OPT_FORMAT="--format=261"
# median filter settings
OPT_MFILTER="--median-filter=0.10/9/3"

# MB1 output selection
# --mb-out=[options]  select mb1 output (mbsvr:host:port,mb1,reson/file)
#   options: mb1svr:host:port - MB1 socket (e.g. MbtrnRecv) [2]
#    mb1              - mb1 data log
#    file             - mb1 data log, use mbsys default file name
#    file:<name>      - mb1 data log, use specified file name
#    reson            - reson frame (s7k)log
#    nomb1            - disable mb1 log (if enabled by default in mbtrnpp)
#    noreson          - disable reson log
OPT_MBOUT="--mb-out=mb1svr"

# TRN output selection
#--trn-out=[options] select trn update output channels
#  options: trnsvr:<host>:<port>,mlog,stdout,stderr,debug [3]
#   trnsvr:<host>:<port> - trn_server socket
#   mlog                 - message log
#   ulog                 - trn estimate log
#   stdout|stderr        - console (independent of debug settings)
#   debug                - per debug settings
OPT_TRNOUT="--trn-out=trnsvr,trnu"

# enable TRN processing
# (must also specify map,par,log,cfg)
OPT_TRN_EN=""
# set TRN UTM zone
OPT_TRN_UTM=""
# set TRN map file (required w/ TRN_EN)
OPT_TRN_MAP=""
# set TRN particles file (required w/ TRN_EN)
OPT_TRN_PAR=""
# set TRN log directory prefix (required w/ TRN_EN)
OPT_TRN_LOG=""
# set TRN config file (required w/ TRN_EN)
OPT_TRN_CFG=""
# set TRN map type
OPT_TRN_MTYPE=""
# set TRN map type
OPT_TRN_FTYPE=""

# verbose level (-5 to +2)
# +2 is a LOT of info
# -2 to 0 recommended for missions
# <= -3 a lot of mbtrn output
OPT_VERBOSE="--verbose=-2"
# drop MB1 clients after hbeat messages
# if they haven't renewed
OPT_MBHBN=""
# drop MB1 clients after OPT_MBHBT seconds
OPT_MBHBT=""
# drop TRN clients after OPT_MBHBT seconds
OPT_TRNHBT=""
# drop TRNU clients after OPT_MBHBT seconds
OPT_TRNUHBT=""

# delay between TRN messages (msec)
OPT_DELAY="" #"--delay=100"
# statistics logging interval (s)
OPT_STATS="--stats=30"

# TRN processing decimation (cycles)
OPT_TRN_DECN="" #"--trn-decn=3"
# TRN processing decimation (sec)
OPT_TRN_DECS="" #"--trn-decs=1"

# print help and exit
OPT_HELP="" #"--help"

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
    echo "  -t      : test [print cmdine]"
    echo "  -v      : verbose output         [$VERBOSE]"
    echo "  -w n    : loop delay (s)         [$LOOP_DELAY_SEC]"
    echo "  -h      : print use message"
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
# description: do command line processsing
# args:
#     args:       positional paramters
#     returnCode: none
########################################
processCmdLine(){
    OPTIND=1
    #    vout "`basename $0` all args[$*]"

while getopts a:c:d:e:htvw: Option
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
        e ) CFG_SEL=$OPTARG
        ;;
        t ) DO_TEST="Y"
        ;;
        v ) VERBOSE="Y"
        ;;
        w ) LOOP_DELAY_SEC=$OPTARG
        ;;
        h) DO_HELP="Y"
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
# Comand line settings override config
# file settings

# process command line args
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

    if [ ${a:2:7} == "verbose" ]
    then
    OPT_VERBOSE=$a
    vout "ovr OPT_VERBOSE: $OPT_VERBOSE"
    fi

    if [ ${a:2:13} == "log-directory" ]
    then
    OPT_LOGDIR=$a
    vout "ovr OPT_LOGDIR: $OPT_LOGDIR"
    fi

    if [ ${a:2:5} == "input" ]
    then
    OPT_INPUT=$a
    vout "ovr OPT_INPUT: $OPT_INPUT"
    fi

    if [ ${a:2:5} == "delay" ]
    then
    OPT_DELAY=$a
    vout "ovr OPT_DELAY: $OPT_DELAY"
    fi

    if [ ${a:2:5} == "mbhbn" ]
    then
    OPT_HBEAT=$a
    vout "ovr OPT_MBHBN: $OPT_MBHBN"
    fi

    if [ ${a:2:5} == "mbhbt" ]
    then
    OPT_HBEAT=$a
    vout "ovr OPT_MBHBT: $OPT_MBHBT"
    fi

    if [ ${a:2:6} == "trnhbt" ]
    then
    OPT_HBEAT=$a
    vout "ovr OPT_TRNHBT: $OPT_TRNHBT"
    fi

    if [ ${a:2:7} == "trnuhbt" ]
    then
    OPT_HBEAT=$a
    vout "ovr OPT_TRNUHBT: $OPT_TRNUHBT"
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

    if [ ${a:2:6} == "output" ]
    then
    OPT_OUTPUT=$a
    vout "ovr OPT_OUTPUT: $OPT_OUTPUT"
    fi

    if [ ${a:2:6} == "mb-out" ]
    then
    OPT_MBOUT=$a
    vout "ovr OPT_MBOUT: $OPT_MBOUT"
    fi

    if [ ${a:2:6} == "trn-out" ]
    then
    OPT_TRNOUT=$a
    vout "ovr OPT_TRNOUT: $OPT_TRNOUT"
    fi

    if [ ${a:2:7} == "stat" ]
    then
    OPT_STATS=$a
    vout "ovr OPT_STATS: $OPT_STATS"
    fi

    if [ ${a:2:6} == "trn-en" ]
    then
    OPT_TRN_EN=$a
    vout "ovr OPT_TRN_EN: $OPT_TRN_EN"
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

    if [ ${a:2:7} == "trn-log" ]
    then
    OPT_TRN_LOG=$a
    vout "ovr OPT_TRN_LOG: $OPT_TRN_LOG"
    fi

    if [ ${a:2:8} == "trn-mtype" ]
    then
    OPT_TRN_MTYPE=$a
    vout "ovr OPT_TRN_MTYPE: $OPT_TRN_MTYPE"
    fi

    if [ ${a:2:8} == "trn-ftype" ]
    then
    OPT_TRN_FTYPE=$a
    vout "ovr OPT_TRN_FTYPE: $OPT_TRN_FTYPE"
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

    if [ ${a:2:8} == "trn-ftype" ]
    then
    OPT_TRN_FTYPE=$a
    vout "ovr OPT_TRN_FTYPE: $OPT_TRN_FTYPE"
    fi


    if [ ${a:2:4} == "help" ]
    then
    OPT_HELP=$a
    vout "ovr OPT_HELP: $OPT_HELP"
    fi
done


# set cmdline options
APP_OPTS="$OPT_VERBOSE $OPT_INPUT $OPT_LOGDIR $OPT_SWATH $OPT_SOUNDINGS $OPT_FORMAT $OPT_MFILTER $OPT_OUTPUT $OPT_STATS $OPT_MBHBN $OPT_MBHBT $OPT_TRNHBT $OPT_TRNUHBT $OPT_DELAY $OPT_TRN_EN $OPT_TRN_UTM $OPT_MBOUT $OPT_TRN_MAP $OPT_TRN_PAR $OPT_TRN_CFG $OPT_TRN_MTYPE $OPT_TRN_FTYPE $OPT_TRN_DECN $OPT_TRN_DECS $OPT_TRNOUT $OPT_HELP"

if [ ${DO_TEST} ]
then
    echo
    echo  $APP_CMD $APP_OPTS
    echo
    exit 0
fi

vout "Using options:"
vout "config file  [$CFG_FILE]"
vout "config name  [$CFG_SEL]"
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

# intialize cycle count
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
