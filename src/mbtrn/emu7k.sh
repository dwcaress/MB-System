#!/bin/bash

#########################################
# Name: emu7k.sh
#
# Summary: Run emu7k
#
# Description: runs 7k Center emulator
# - automatically restarts after delay [optional]
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
DESCRIPTION="Run emu7k; optionally restart on exit"

LOOP_DELAY_SEC_DFL=5
let "CYCLES_DFL=1"
SESSION_ID=`date +"%Y%m%d-%H%M%S"`
SESSION_NAME="emu7k-console-${SESSION_ID}"
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
APP_CMD="/usr/local/bin/emu7k.exe"

# 7k center host
# localhost     : if clients are on this host
# this host IP  : if clients are on remote host
OPT_HOST="--host=localhost"

# 7k center port
# uses 7000 by default
# must change if 7k center
# running on this host
OPT_PORT="" #"--port=7001"

# stream contains net frames
# use with mbin log input
# don't use with s7k input
OPT_NF="" #"--nf"
# minimum frame delay (ms)
OPT_MINDELAY="--min-delay=0"
# restart at end of file/list
# default is no restart
OPT_RESTART="" #"--restart"
# exit at end of file/list [default]
OPT_NORESTART="" #"--no-restart"
# show stats every n frames
OPT_STATN="" #"--statn=10000"
# verbose output
OPT_VERBOSE="--verbose=0"
# print version number and exit
OPT_VERSION="" #"--verbsion"
# print help info and exit
OPT_HELP="" #"--help"

# delay output (n/m)
# wait n sec every m sec
# [debug only]
OPT_XDELAY="" #"--xdelay=5/20"

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
    echo " use: `basename $0` [options] -- [--option=value...] <file> [<file>...]"
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
#     args:       positional parameters
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
# Command line settings override config
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
shift
    fi

    if [ ${a:2:7} == "verbose" ]
    then
    OPT_VERBOSE=$a
    vout "ovr OPT_VERBOSE: $OPT_VERBOSE"
shift
    fi

    if [ ${a:2:9} == "min-delay" ]
    then
    OPT_MINDELAY=$a
    vout "ovr OPT_MINDELAY: $OPT_MINDELAY"
shift
    fi

    if [ ${a:2:4} == "host" ]
    then
    OPT_HOST=$a
    vout "ovr OPT_HOST: $OPT_HOST"
shift
    fi

    if [ ${a:2:4} == "port" ]
    then
    OPT_PORT=$a
    vout "ovr OPT_PORT: $OPT_PORT"
shift
    fi

    if [ ${a:2:2} == "nf" ]
    then
    OPT_NF=$a
    vout "ovr OPT_NF: $OPT_NF"
shift
    fi

    if [ ${a:2:7} == "restart" ]
    then
    OPT_RESTART=$a
    vout "ovr OPT_RESTART: $OPT_RESTART"
shift
    fi

    if [ ${a:2:10} == "no-restart" ]
    then
    OPT_NORESTART=$a
    vout "ovr OPT_NORESTART: $OPT_NORESTART"
shift
    fi

    if [ ${a:2:5} == "statn" ]
    then
    OPT_STATN=$a
    vout "ovr OPT_STATN: $OPT_STATN"
shift
    fi

    if [ ${a:2:6} == "xdelay" ]
    then
    OPT_XDELAY=$a
    vout "ovr OPT_XDELAY: $OPT_XDELAY"
shift
    fi

	if [ ${a:2:4} == "help" ]
    then
    OPT_HELP=$a
    vout "ovr OPT_HELP: $OPT_HELP"
shift
    fi
done


# set cmdline options
APP_OPTS="$OPT_VERBOSE $OPT_LOGDIR $OPT_HOST $OPT_PORT $OPT_MINDELAY $OPT_NF  $OPT_RESTART $OPT_STATN $OPT_XDELAY $OPT_HELP $@"

if [ ${DO_TEST} ]
then
    echo
    echo  $APP_CMD $APP_OPTS
    echo
    exit 0
fi

vout "Using options:"
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

    # check stop condition (if not <0)
    if [ ${LOOP_COUNT} -ge 0 ]
    then
	    let "LOOP_COUNT=${LOOP_COUNT}-1"
        if [ ${LOOP_COUNT} -eq 0 ]
        then
            vout "app - completed $CYCLES cycles - exiting"
            exit 0
        fi
    fi

    # wait and restart
    sleep ${LOOP_DELAY_SEC}
    vout "restarting $APP_CMD"
    vout
done
