#!/bin/bash

# run-mbsuite.sh - run a dockerized app
# 2023-09-25
# kheadley
#
# Configures environment and volume mapping for TRN.
# Typically used to run SLAM pipeline components, e.g.
# mbtrnpp, mb1stream, lcm-logger, etc.
#
# See showUse function or use -vh for use notes
#

# constants
ATUNCITA_LASS=192.168.168.173
ATUNCITA_SHORE=134.89.13.26
ATUNCITA_MINI=192.168.2.3
RESON_LASS=192.168.168.132
RESON_MAP1=134.89.32.107
RESON_MAP2=134.89.32.108
UBUNTU20_VM=134.89.12.217
UBOAT_VM=134.89.13.165
SIRENA_HOME=192.168.1.101
SIRENA_SHORE=134.89.13.129
KHNET_MB1STREAM=172.18.0.3
KHNET_MBTRNPP=172.18.0.2

# config defaults
CMD_OPTS="d:hH:i:l:n:t:u:vx:"
DOCKER_IMG=${DOCKER_IMG:-"mbsuite"}
DOCKER_APP=":"
vout=":"
declare -a xopts

TOP_DIR=${TOP_DIR:-"$(builtin cd "../.."; pwd)"}
CFG_DIR=${CFG_DIR:-"config"}
MAP_DIR=${MAP_DIR:-"maps"}
DATA_DIR=${DATA_DIR:-"data"}
LOG_DIR=${LOG_DIR:-"logs"}

LCM_TTL=${LCM_TTL:-0}

OPT_RM="--rm"
OPT_USER=ops:ops

# docker host IP
# recommended to override (use -H or set HOST_IP)
HOST_IP=${HOST_IP:-localhost}

# NET: "host", "bridge", custom
NET=${NET:-"host"}
# NET=${NET:-"khnet"}

TRN_SOURCE_DFL=${TRN_SOURCE_DFL:-$HOST_IP}
#TRN_SOURCE_DFL=${TRN_SOURCE_DFL:-$RESON_LASS}

#DISPLAY_DFL=$HOST_IP
#DISPLAY_DFL="host.docker.internal"
#DISPLAY_DFL="$(hostname)"
DISPLAY_DFL="localhost"

LCM_DEFAULT_URL_DFL=${LCM_DEFAULT_URL_DFL:-"udpm://239.255.76.67:12660"}

# show use message
function showUse() {

cat << EOF
 Use Notes

     Typically used to run MB-System components

     The primary environment values exported include

       TRN_HOST         IP of host running mbtrnpp
       TRN_SOURCE_HOST  IP of host providing TRN input data
       TRN_DATAFILES    TRN config file directory
       TRN_MAPFILES     TRN map file directory
       TRN_MBTRNDIR     mbtrnpp executable directory
       DISPLAY          X windows display
       LCM_DEFAULT_URL  LCM URL

     The script uses this directory structure by default

       MSN_TOP           mission top directory
         bin             scripts
         config          TRN config files
         maps            TRN maps
         data            optional data directory (e.g. for test)
         missions        mission directories
           <mission>...
             logs        TRN logs for <mission>

     Creating a new mission directory (e.g. 20231012-m1) for each session
     keeps logs and data sorted for post-processing.

     It is recommended to set environment variable MSN_TOP
     with the absolute path to top directory containing config and maps
     It may also be passed using -m option, though it would not be available
     to reference on the command line.

     It is possible to override the directory structure using a combination
     of command line options, environment variables or script configuration defaults.

     The calling environment will be used to intialize/override default values
     of the primary variables above plus

       DISPLAY             TOP_DIR
       LCM_TTL             CFG_DIR
       HOST_IP             MAP_DIR
       NET                 DATA_DIR
       DOCKER_IMG          LOG_DIR
       LCM_DEFAULT_URL_DFL
       TRN_SOURCE_HOST_DFL

     Modifying these via the command line will also
     update variables that use them

       DISPLAY   NET
       LCM_TTL   DOCKER_IMG
       HOST_IP   TOP_DIR

     Under MacOS, the docker host network driver is not supported.
     Applications requiring network IO may not work, or may only
     work if a docker bridge network is used.

     To keep command lines simple, it may be convenient to
     - modify script config defaults
     - set environment variables before calling
     - copy/paste command lines into the terminal

EOF

}

# show help message
function showHelp() {
cat << EOF

 Summary : Run dockerized app

 Use: $(basename $0) [options] -- app [args...]

 Options:

   -d : set DISPLAY       [$DISPLAY]
   -h : show help message
        (use -vh for use notes)
   -H : set host IP       [$HOST_IP]
        may update
        - TRN_SOURCE_DFL
        - DISPLAY_DFL
   -i : set docker image  [$DOCKER_IMG]
   -l : set LCM TTL       [$LCM_TTL]
   -n : set NET           [$NET]
   -t : set TOP_DIR       [$TOP_DIR]
   -u : user              [$OPT_USER]
   -v : verbose output
   -x : pass docker option

   # Examples

   # Note: pathnames passed to docker apps must be absolute
   # and within the mapped volumes

   # run mbtrnpp from mission directory MSN_TOP/mission/test
   # - MSN_TOP used for mbtrnpp args
   # - --log-directory=$PWD/logs == MSN_TOP/mission/test/logs
   MSN_TOP=\$(readlink -f ../../)
   ../../bin/run-mbsuite.sh  -H 134.89.13.26 -- mbtrnpp --config=\$MSN_TOP/config/mbtrnpp-port.cfg --log-directory=\$PWD/logs --input=\$MSN_TOP/config/dlist.mbx --format=-1

   # run mbtrnpp on linux using host network, with Reson (192.168.168.132)
   # [from outside MSN_TOP, direct mbtrnpp logs in \$PWD/logs]
   TRN_RESON_HOST=192.168.168.132 $(basename $0) -H 192.168.168.173 -t ./tmp/TRN -- mbtrnpp --config=\$PWD/tmp/TRN/config/mbtrnpp.cfg --input=socket:192.168.168.132 --format=89 --log-directory=\$PWD/logs

   # test mbtrnpp on macOS using custom bridge network khnet
   $(basename $0) -n khnet -H 172.18.0.2 -t ./tmp/TRN -- mbtrnpp --config=\$PWD/tmp/TRN/config/mbtrnpp.cfg --input=\$PWD/tmp/TRN/config/dlist.mbx --format=-1 --log-directory=\$PWD/logs


EOF

if [ $vout != ":" ] ; then
    showUse
fi

}

# show settings (if verbose output enabled)
function showSettings() {

if [ $vout != ":" ] ; then

declare -a cmd
IFS=" " read -a cmd <<< "$DOCKER_CMD"

cat << EOF

 TOP_DIR         $TOP_DIR
 DOCKER_APP      $DOCKER_APP
 TRN_HOST        $TRN_HOST
 TRN_SOURCE_HOST $TRN_SOURCE_HOST
 TRN_DATAFILES   $TRN_DATAFILES
 TRN_MAPFILES    $TRN_MAPFILES
 TRN_LOGFILES    $TRN_LOGFILES
 TRN_MBTRNDIR    $TRN_MBTRNDIR
 DISPLAY         $DISPLAY
 LCM_DEFAULT_URL $LCM_DEFAULT_URL

 DOCKER_CMD:     $DOCKER_CMD

EOF

fi

}

function parseDebug() {
    OPTIND=1
    while getopts ${CMD_OPTS} Option
    do
        case $Option in
            v) vout="echo"
            ;;
            *) # ignore all
            ;;
        esac
    done
}

# parse command line options
function parseOptions() {
    OPTIND=1
    while getopts ${CMD_OPTS} Option
    do
        case $Option in
            d) DISPLAY=$OPTARG
            ;;
            h) showHelp
                exit 0
            ;;
            H) if [ $TRN_SOURCE_DFL == $HOST_IP ] ; then
                    TRN_SOURCE_DFL=$OPTARG
                fi
                if [ $DISPLAY_DFL == $HOST_IP ] ; then
                    DISPLAY_DFL=$OPTARG
                fi
                HOST_IP=$OPTARG
            ;;
            i) DOCKER_IMG=$OPTARG
            ;;
            l) LCM_TTL=$OPTARG
            ;;
            n) NET=$OPTARG
            ;;
            t) TOP_DIR=$(readlink -f $OPTARG)
            ;;
            u) OPT_USER=$OPTARG
            ;;
            v) vout="echo"
            ;;
            x) xopts[${#xopts[*]}]=$OPTARG
            ;;
            *) exit 0 # getopts outputs error message
            ;;
        esac
    done
}

###################
# Main entry point

# parse debug options to enable before processing
parseDebug "$@"
# parse command line
parseOptions "$@"

# pop script args
let "i=1"
while [ $i -lt $OPTIND ]
do
    shift
    let "i=$i+1"
done

# apply options
DOCKER_APP=$1
#CONTAINER="mbari_$DOCKER_APP"
CONTAINER="mbari_$$"

if [ $NET == "host" ] ; then
    OPT_NET="--network=$NET"
else
    OPT_NET="--network=$NET --ip=$HOST_IP"
fi

# set environment
DISPLAY=${DISPLAY:-"$DISPLAY_DFL:0"}
LCM_DEFAULT_URL=${LCM_DEFAULT_URL:-"$LCM_DEFAULT_URL_DFL?ttl=$LCM_TTL"}
export TRN_HOST=${TRN_HOST:-$HOST_IP}
export TRN_SOURCE_HOST=${TRN_SOURCE_HOST:-$TRN_SOURCE_DFL}
export TRN_DATAFILES=${TRN_DATAFILES:-${TOP_DIR}/${CFG_DIR}}
export TRN_MAPFILES=${TRN_MAPFILES:-${TOP_DIR}/${MAP_DIR}}
export TRN_MBTRNDIR=${TRN_MBTRNDIR:-"/usr/local/bin"}
# unset TRN_LOGFILES to use mbtrnpp --log-directory
unset TRN_LOGFILES

# build docker command
DOCKER_CMD="docker run -it --name $CONTAINER \
--user $OPT_USER \
$OPT_RM \
$OPT_NET \
--env=DISPLAY \
--env=LIBGL_ALWAYS_INDIRECT=1 \
--env=LCM_DEFAULT_URL \
--env=TRN_SOURCE_HOST \
--env=TRN_HOST \
--env=TRN_DATAFILES \
--env=TRN_MAPFILES \
--env=TRN_MBTRNDIR \
--volume=$HOME/.bash_history:/opt/$DOCKER_APP/.bash_history \
--volume=$PWD:$PWD \
--volume=$TRN_DATAFILES:$TRN_DATAFILES \
--volume=$TRN_MAPFILES:$TRN_MAPFILES \
--volume=$TOP_DIR/$DATA_DIR:$TOP_DIR/$DATA_DIR \
--volume=$PWD/$LOG_DIR:$PWD/$LOG_DIR \
${xopts[*]} \
$DOCKER_IMG \
$*"

showSettings

$DOCKER_CMD
