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
APP_CMD="/usr/local/bin/mbtrnpp.exe"
# log directory
OPT_LOGDIR="--log-directory=/home/reson/axialgeo/out"
# input source
# [optional if --rhost is used]
OPT_INPUT="" #"--input=socket"
# output destination
# [optional if --thost is used]
OPT_OUTPUT="" #"--output=socket"
# beam swath (deg)
OPT_SWATH="--swath=120"
# number of beams to publish
OPT_SOUNDINGS="--soundings=25"
# input data format
# [do not use with datalist input]
OPT_FORMAT="--format=88"
# median filter settings
OPT_MFILTER="--median-filter=0.25/9/3"

# reson host:port
# localhost     : if running on reson host
# this host IP  : if running on remote host
# use port 7000 for 7k center
# use configured port for emu7k
OPT_RHOST="--rhost=localhost:7000"

# TRN host:port
# localhost     : if clients are on this host
# this host IP  : if clients are on remote host
# Mapper 1
OPT_THOST="--thost=134.89.32.107:27000"
# Mapper 2
#OPT_THOST="--thost=134.89.32.110:27000"

# verbose level (-5 to +2)
# +2 is a LOT of info
# -2 to 0 recommended for missions
# <= -3 a lot of mbtrn output
OPT_VERBOSE="--verbose=-2"
# drop TRN clients after hbeat messages
# if they haven't renewed
OPT_HBEAT="--hbeat=100"
# delay between TRN messages (msec)
OPT_DELAY="" #"--delay=100"
# statistics logging interval (s)
OPT_STATS="--stats=30"
# disable message log
OPT_NOMLOG="" #"--no-mlog"
# disable TRN output log
OPT_NOBLOG="" #"--no-blog"
# disable 7k center frame log
OPT_NORLOG="" #"--no-rlog"
# print help and exit
OPT_HELP="" #"--help"
# log all 7k center frames
# [enabled by default]
OPT_MBRLOG="" #"--mbrlog"
# reader capacity [deprecated]
OPT_RCAP="" #"--rcap=600000"

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
    #	exit -1
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

    if [ ${a:2:5} == "hbeat" ]
    then
    OPT_HBEAT=$a
    vout "ovr OPT_HBEAT: $OPT_HBEAT"
    fi

    if [ ${a:2:5} == "rhost" ]
    then
    OPT_RHOST=$a
    vout "ovr OPT_RHOST: $OPT_RHOST"
    fi

    if [ ${a:2:5} == "thost" ]
    then
    OPT_THOST=$a
    vout "ovr OPT_THOST: $OPT_THOST"
    fi

    if [ ${a:2:4} == "rcap" ]
    then
    OPT_RCAP=$a
    vout "ovr OPT_RCAP: $OPT_RCAP"
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

    if [ ${a:2:7} == "stat" ]
    then
    OPT_STATS=$a
    vout "ovr OPT_STATS: $OPT_STATS"
    fi

    if [ ${a:2:7} == "no-mlog" ]
    then
    OPT_NOMLOG=$a
    vout "ovr OPT_NOMLOG: $OPT_NOMLOG"
    fi

    if [ ${a:2:7} == "no-blog" ]
    then
    OPT_NOBLOG=$a
    vout "ovr OPT_NOBLOG: $OPT_NOBLOG"
    fi

    if [ ${a:2:7} == "no-rlog" ]
    then
    OPT_NORLOG=$a
    vout "ovr OPT_NORLOG: $OPT_NORLOG"
    fi

    if [ ${a:2:4} == "help" ]
    then
    OPT_HELP=$a
    vout "ovr OPT_HELP: $OPT_HELP"
    fi
done


# set cmdline options
APP_OPTS="$OPT_VERBOSE $OPT_LOGDIR $OPT_RHOST $OPT_THOST $OPT_SWATH $OPT_SOUNDINGS $OPT_FORMAT $OPT_MFILTER $OPT_INPUT $OPT_OUTPUT $OPT_MBRLOG $OPT_STATS $OPT_HBEAT $OPT_DELAY $OPT_NOMLOG $OPT_NOBLOG $OPT_NORLOG  $OPT_RCAP $OPT_HELP"

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

if [ ! -z ${MBT_FORMAT} ] && [ ! -z ${MBT_INPUT} ] && [ ! -z `expr "${MBT_INPUT}" : '\(.*mb-1\)'` ]
then
	echo "WARN - format specified for datalist"
fi

if [ ! -d ${OPT_LOGDIR:15:} ]
then
	exitError "log directory not found [${OPT_LOGDIR:15:}]" 1
fi

if [ "${MBT_INPUT:0:6}" != "socket" ] && [ ! -f "${MBT_INPUT}" ] && [ -z ${MBT_RHOST} ]
then
    #exitError "input file not found" 1
    echo "WARN - input file not defined"
    echo "[set MBT_INPUT or use '-- --input=<file>|socket' on cmdline]"
    echo "or"
    echo "[set MBT_RHOST or use '-- --rhost=host:port' on cmdline]"
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
