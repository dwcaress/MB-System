#!/bin/bash

#########################################
# Name: sysmon.sh
#
# Summary: log output of system monitoring utils
#
# Description: logs output of top, vmstat, df, etc.
# to a file. Uses segmented logs.
#
# Author: headley
#
# Copyright MBARI 2017
#
#########################################

#########################################
# Script configuration defaults
# casual users should not need to change
# anything below this section
#########################################
sysmon_description="Log output of system monitoring utils"

#################################
# Script variable initialization
#################################

# verbose output enable/disable
VERBOSE="N"
MSG_INDENT=2

# default log file name
DFL_LOG_FILE="/dev/stdout"

# default cycle count
# -1: loop forever
DFL_CYCLE_COUNT=-1

# default cycle period (s)
DFL_PERIOD_SEC=60

# default seg size
DFL_SEG_SIZE=102400
# default seg count
DFL_SEG_COUNT=10

# default log file name
LOG_FILE_BASE="${DFL_LOG_FILE}"
LAST_SEG=0
LOG_FILE="${LOG_FILE_BASE}"

# cycle delay (seconds)
PERIOD_SEC=${DFL_PERIOD_SEC}
# max segment size
SEG_SIZE=${DFL_SEG_SIZE}
# max number of segments
SEG_COUNT=${DFL_SEG_COUNT}

# last segment number
LAST_SEG=0
# max cycle count
CYCLE_COUNT=${DFL_CYCLE_COUNT}
# cycle count var
CYCLES=0

# output enable/disable defaults
do_vmstat=Y
do_top=Y
do_df=Y

# these will get set below
# with platform-specific options
ARCH="unknown"
STAT=`which stat`
STAT_OPTS=""
TOP=`which top`
TOP_OPTS="-n 1 -l 1"
VMSTAT=`which vmstat`
VMSTAT_OPTS=""
DF=`which df`
DF_OPTS="-h"

TIMESTAMP="date"
OUT_HEADER="out_header"
OUT_FOOTER="echo"

#################################
# name: printUsage
# description: print use message
# args: none
#################################
printUsage(){
    echo
    msg "Description: $sysmon_description"
    echo
    msg "usage: `basename $0` [options]"
	msg "Options:"
    msg " -f <file>   : log file name          [${DFL_LOG_FILE}]"
    msg " -n <cycles> : number of cycles       [${DFL_CYCLE_COUNT}]"
    msg " -p <sec>    : cycle period (sec)     [${DFL_PERIOD_SEC}]"
    msg " -N <seg>    : number of log segments [${DFL_SEG_COUNT}]"
    msg " -s <size>   : log segment size       [${DFL_SEG_SIZE}]"
    msg " -D          : disable df output"
    msg " -T          : disable top output"
    msg " -V          : disable vmstat output"
    msg " -v          : verbose output"
	msg " -h          : print use message"
	echo
	msg "Use Ctrl-C to quit"
    echo
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
# name: imsg
# description: echos with indent
# args:
#     msg: message
#  indent: indent level
########################################
imsg(){
  OIFS=$IFS
  IFS=""
  msg_pad="                                        "
  let "msg_indent=${1}"
  shift
  all="${@}"
  printf "%s%s\n" ${msg_pad:0:${msg_indent}} ${all}
  IFS=$OIFS
}

########################################
# name: msg
# description: echos with default indent
# args:
#     msg: message
########################################
msg(){
 imsg $MSG_INDENT "${*}"
}

########################################
# name: exitError
# description: print use message to stderr
# args:
#     msg:        error message
#     returnCode: exit status to return
########################################
exit_error(){
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
	vout "`basename $0` all args[$*]"

	while getopts p:f:hn:N:o:s:v Option
	do
		vout "processing $Option[$OPTARG]"
		case $Option in
			p) PERIOD_SEC=$OPTARG
			;;
			f) LOG_FILE_BASE=$OPTARG
			   LOG_FILE="${LOG_FILE_BASE}"
			;;
			n) CYCLE_COUNT=$OPTARG
			;;
			N) SEG_COUNT=$OPTARG 
			;;
			s) SEG_SIZE=$OPTARG
			;;
			D) do_df="N"
			;;
			T) do_top="N"
			;;
			V) do_vmstat="N"
			;;
			v ) VERBOSE="Y"
			;;
			h)printUsage
				exit 0
			;;
			*) exit 0 # getopts outputs error message
			;;
		esac
	done
}

##########################
# User functions
##########################

###################################################
# rotate()
# Rotate log files, per max segment size and number.
# names segments <name>.<number>; overwrites lowest (oldest)
# segment when the last segment is full. 
# If segment size < 0, appends to single segment (grows indefinitely)
rotate()
{
	if [ "${SEG_SIZE}" -le 0 ]
	then
		${NOOP}
	else
		if [ "${LOG_FILE}" != "/dev/stdout" ]
		then
			log_size=`${STAT} ${STAT_OPTS} ${LOG_FILE}`
			vout "log_size[$log_size]"
			if [ "${log_size}" -ge "${SEG_SIZE}" ]
			then
				let "LAST_SEG=1+${LAST_SEG}"
				if [ "${LAST_SEG}" -ge "${SEG_COUNT}" ]
				then
					LAST_SEG=0
				fi
				if [ `basename ${LOG_FILE_BASE}` != "stdout" ]
				then
				LOG_FILE="${LOG_FILE_BASE}.${LAST_SEG}"
				echo "" > ${LOG_FILE}
				fi
			fi
		fi
	fi
}

###################################################
# update_log()
# write latest info to log
# calls vmstat, top, df per settings
# Skips any missing commands
update_log()
{
	if [ "${do_vmstat}" == "Y" ] 
	then
		echo "### VMSTAT"
		${VMSTAT} ${VMSTAT_OPTS}
		echo
	fi
	if [ "${do_top}" == "Y" ] 
	then
		echo "### TOP"
		if [ "${ARCH}" == "darwin" ]
		then
		${TOP} ${TOP_OPTS} | tail -n 22
		else
		${TOP} ${TOP_OPTS} 
		fi

		echo
	fi
	if [ "${do_df}" == "Y" ] 
	then
		echo "### DF"
		${DF} ${DF_OPTS}
		echo
	fi
}

###################################################
# out_header()
# write header to output
# include date/time, cycle status
out_header()
{
	if [ "${CYCLES}" -lt 0 ]
	then
		let "ncycles=${CYCLES}*-1"
	else
		let "ncycles=${CYCLE_COUNT}-${CYCLES}+1"
	fi
	echo "### `date` [${ncycles}/${CYCLE_COUNT}]"
}

###################################################
# check_exit()
# check exit conditions and 
# exit when done
check_exit()
{
	if [ "${CYCLES}" -eq 0 ]
	then
		exit 0
	fi
}

##########################
# Script main entry point
##########################

# command line option processing
processCmdLine "$@"

# validate cycle delay
if [ ${PERIOD_SEC} -le 0 ]
then
	echo
	echo "Delay value must be >0"
	echo
	exit -1
fi

# create log file and dir 
# if it doesn't exist
if [ ! -e ${LOG_FILE} ] 
then	
	log_dir=`dirname ${LOG_FILE}`
	if [ ! -d ${log_dir} ]
	then
		mkdir -p ${log_dir} 
	fi
	LOG_FILE="${LOG_FILE_BASE}.${LAST_SEG}"
	touch ${LOG_FILE}
elif  [ -x ${LOG_FILE} ] || [ -c ${LOG_FILE} ] || [ -b ${LOG_FILE} ]
then
	if [ `basename ${LOG_FILE}` != "stdout" ]
	then
	echo
	echo "Error - name conflict [${LOG_FILE}]"
	echo
	exit 0
	fi
fi

# validate cycle count
# translate all negative
# to -1 so reported counts are correct
if [ "${CYCLE_COUNT}" -lt 0 ]
then
	CYCLE_COUNT=-1
fi

# implementation-specific 
# util commands and options

# read architecture
osname=`uname`
# turn off casematch (remember to restore!)
shopt -s nocasematch
# must use double brackes for shopt/case insensitive
if [[ "${osname:0:6}" == "Darwin" ]]
then
	ARCH="darwin"
	STAT_OPTS="-f %z"
	VMSTAT_OPTS="-tc %s"
	TOP_OPTS="-stats command,pid,time,cpu,cpu_me,cpu_others,vsize,mem -o mem -O cpu -l 2 -n 10 -i 1 "
	DF_OPTS="-h"
elif [[ "${osname:0:6}" == "CYGWIN" ]]
then
	ARCH="cygwin"
	STAT_OPTS="-tc %s"
	TOP_OPTS="-bHn 1 -o %MEM -o %CPU"
	VMSTAT_OPTS="-tan"
	DF_OPTS="-h"
elif [[ "${osname:0:5}" == "LINUX" ]]
then
    ARCH="linux"
    STAT_OPTS="-tc %s"
    TOP_OPTS="-bHn 1 -o %MEM -o %CPU"
    VMSTAT_OPTS="-tan"
    DF_OPTS="-h"
else
	ARCH="linux"
	# TODO: fill in util opts for linux utils
	STAT="stat"
	STAT_OPTS="-h"
	VMSTAT="vmstat"
	VMSTAT_OPTS="-h"
	TOP="top"
	TOP_OPTS="-h"
	DF="df"
	DF_OPTS="-h"
fi
# restore casematch
shopt -u nocasematch

# check/report missing utils

if [  -z "${STAT}" ]
then
	# stat command is required
	exit_error "fatal error - stat not found" -1
else
	STAT=`which stat`
fi

if [  -z "${VMSTAT}" ] && [ "${do_vmstat}" == "Y" ]
then
	msg "warning - vmstat not found"
	# disable output
	do_vmstat="N"
	VMSTAT="vmstat not found"
	VMSTAT_OPTS=""
fi

if [  -z "${TOP}" ] && [ "${do_top}" == "Y" ]
then
	msg "warning - top not found"
	# disable output
	do_top="N"
	TOP="top not found"
	TOP_OPTS=""
fi

if [  -z "${DF}" ] && [ "${do_df}" == "Y" ]
then
	msg "warning - df not found"
	# disable output
	do_df="N"
	DF="df not found"
	DF_OPTS=""
fi

vout "VERBOSE     [$VERBOSE]"
vout "do_vmstat   [$do_vmstat]"
vout "do_top      [$do_top]"
vout "do_df       [$do_df]"
vout "PERIOD_SEC   [$PERIOD_SEC]"
vout "SEG_SIZE    [$SEG_SIZE]"
vout "SEG_COUNT   [$SEG_COUNT]"
vout "CYCLE_COUNT [$CYCLE_COUNT]"
vout "LOG_FILE    [$LOG_FILE]"
vout "ARCH        [$ARCH]"
vout "STAT        [$STAT ${STAT_OPTS}]"
vout "VMSTAT      [$VMSTAT ${VMSTAT_OPTS}]"
vout "TOP         [$TOP ${TOP_OPTS}]"
vout "DF          [$DF ${DF_OPTS}]"

##################
# main loop 

# initialize cycle counter
# (count down from CYCLE_COUNT
# loop forever if CYCLE_COUNT<0)
CYCLES=${CYCLE_COUNT}

while [ "${CYCLES}" -ne 0 ]
do
	# rotate logs if needed
	rotate
	
	# update log output
	${OUT_HEADER} >> ${LOG_FILE}
	update_log >> ${LOG_FILE} 
	${OUT_FOOTER} >> ${LOG_FILE}

	# update cycle counter
	let "CYCLES=${CYCLES}-1"
	
	# check exit conditions
	# (exit if done)
	check_exit
	
	# sleep until next cycle
	sleep ${PERIOD_SEC}
	
done