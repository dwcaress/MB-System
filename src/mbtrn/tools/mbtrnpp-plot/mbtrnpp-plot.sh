#!/usr/local/bin/bash

#########################################
# Name: mbtrnpp-plot
#
# Summary: plot mbtrnpp log data sets
#
# Description:
# mbtrnpp-plot is a front-end for qplot
# that generates plots using mbtrnpp log data.
# mbtrnpp-plot reads mbtrnpp log files to
# produce CSV input for qplot.
#
# qplot generates gnuplot scripts and PDF files
# that use gnuplot and ghostscript APIs.
# qplot uses configuration files to define
# one or more plot images or to combine a set of
# plot images into a PDF file.
#
# NOTE:
# Requires bash >=4.x, which implements associative arrays
# Typically, bash is located in /usr/bin.
# On OSX, default (/usr/bin) bash is 3.x,
# which does not implement associative arrays.
# Macports or Brew may be used to install
# bash v4.x, which has associative arrays.
#
# File Naming Conventions:
# qp-*.conf : qplot job configuration/job definitionos
# qu-*.conf : mbtrnpp configuration/job definitions
#
# Variable Naming Conventions:
# QU_ prefix for user environment prevents naming conflicts
# QP_ prefix denotes qplot environment/variables
# QX_ prefix denotes local environment/variables
#
# Author: k headley
#
# Copyright 2019 MBARI
#
#########################################

#########################################
# Script configuration defaults
# casual users should not need to change
# anything below this section
#########################################
 description="mbtrnpp log plot utility"

#################################
# Script variable initialization
#################################
VERBOSE="N"
unset DO_LIST
unset DO_HELP
QX_PLOTSET_INDEX=-1
QX_PLOTSET_KEY=""
QX_PLOTSET_DEF=""
QX_LOG_PATH="."
QX_SESSION_ID="-"
QX_DATA_SET_ID="undefined"
QX_JOB_DIR=${QX_JOB_DIR:-"jobs"}
QX_PLOTSET_COUNT=0
declare -a QX_LOG_PATHS
declare -a QX_SESSION_IDS
declare -a QX_DATA_SET_IDS

# set QPLOT_HOME at this scripts location
# (unless set externally)
QP_PLOT_HOME_DFL="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
QP_PLOT_HOME=${QP_PLOT_HOME:-${QP_PLOT_HOME_DFL}}

# source plotset definitions
source ${QP_PLOT_HOME}/qu-plotsets-conf.sh

# source shared definitions
source ${QP_PLOT_HOME}/qp-shared.conf.sh

#################################
# Function Definitions
#################################

#################################
# name: show_help
# description: print use message
# args: none
#################################
show_help(){
	echo ""
    echo " Description: $description"
    echo
    echo " Use: `basename $0` [options]"
    echo " Options:"
    echo "  -l         : list plotset keys"
    echo "  -f file    : use plotset config file   [1,2]"
	echo "  -j dir     : job output directory"
	echo "  -i n       : generate plotset by index [1]"
    echo "  -k s       : generate plotset by key   [1]"
	echo "  -s \"p,i,d\" : generate plotset using path,id,description [1]"
    echo "  -v         : verbose output"
    echo "  -h         : print use message"
    echo ""
    echo "  [1] may be used more than once"
	echo "  [2] applies to subsequent plotsets, until changed"
    echo ""
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
exitError(){
    echo >&2
    echo "`basename $0`: error - $1" >&2
    echo >&2
    exit $2
}

########################################
# name: list_keys
# description: list plot set keys
# args: none
# returnCode: none
########################################
list_keys(){
    let "i=0";
	echo ""
    echo " index key"

    while [ ${i} -lt ${#QU_PLOTSET_KEYS[*]} ]
    do
        printf "%4d   %s\n" $i ${QU_PLOTSET_KEYS[$i]}
		key=${QU_PLOTSET_KEYS[$i]}
        vout "       LOG_PATH   : ${QU_LOG_PATHS[$key]}"
        vout "       SESSION_ID : ${QU_SESSION_IDS[$key]}"
        vout "       DATA_SET_ID: ${QU_DATA_SET_IDS[$key]}"
        vout ""
        let "i=$i+1"
    done
	echo ""
}

########################################
# name: parse_key
# description: use key to add job
# args:
#      key - plot set key (indexes QU_PLOTSET_KEYS in qu-plotsets-conf)
# returnCode: none
########################################
parse_key(){
    QX_LOG_PATHS[$QX_PLOTSET_COUNT]=${QU_LOG_PATHS[$key]}
    QX_SESSION_IDS[$QX_PLOTSET_COUNT]=${QU_SESSION_IDS[$key]}
    QX_DATA_SET_IDS[$QX_PLOTSET_COUNT]=${QU_DATA_SET_IDS[$key]}
    let "QX_PLOTSET_COUNT=$QX_PLOTSET_COUNT+1"
}

########################################
# name: process_cmdline
# description: do command line processsing
# args:
#     args:       positional paramters
# returnCode: none
########################################
process_cmdline(){
    OPTIND=1
    vout "`basename $0` all args[$*]"

while getopts hf:i:j:k:ls:v Option
    do
        vout "processing $Option[$OPTARG]"
        case $Option in
            f ) if [ -f $OPTARG ]
				then
                source $OPTARG
				else
				echo "WARN - could not source plotset config $OPTARG"
				fi
            ;;
            i ) QX_PLOTSET_INDEX=$OPTARG
				if [ $QX_PLOTSET_INDEX -lt ${#QU_PLOTSET_KEYS[*]} ]
				then
				key=${QU_PLOTSET_KEYS[$QX_PLOTSET_INDEX]}
				parse_key $key
				else
				echo "WARN - invalid index [$QX_PLOTSET_INDEX] - must be < ${#QU_PLOTSET_KEYS[*]}"
				fi
            ;;
            j ) QX_JOB_DIR=$OPTARG
            ;;
			k ) key=$OPTARG
             	key_valid=""
				let "z=0"
				while [ $z -lt ${#QU_PLOTSET_KEYS[*]} ]
				do
					if [ $key == ${QU_PLOTSET_KEYS[$z]} ]
					then
						parse_key $key
						key_valid=="Y"
						break;
					fi
					let "z=$z+1"
                done
				if [ -z $key_valid ]
				then
	                echo "WARN - invalid key [$key]"
				fi
			;;
            l ) DO_LIST="Y"
            ;;
            s ) QU_PLOTSET_DEF=$OPTARG
                IFSO=$IFS
                IFS=',' read -r lp sid dsid <<< ${QU_PLOTSET_DEF}
				IFS=$IFSO
                if [ ! -z "${lp}" ] && [ ! -z "${sid}" ] && [ ! -z "${dsid}" ]
                then
                    if [ -d ${lp} ]
                    then
                        QX_LOG_PATHS[$QX_PLOTSET_COUNT]=$lp
                        QX_SESSION_IDS[$QX_PLOTSET_COUNT]=$sid
                        QX_DATA_SET_IDS[$QX_PLOTSET_COUNT]=$dsid
                        let "QX_PLOTSET_COUNT=$QX_PLOTSET_COUNT+1"
					else
						echo "WARN - directory does not exist [$lp]"
                    fi
                fi
            ;;
            v ) VERBOSE="TRUE"
            ;;
            h) DO_HELP="Y"
            ;;
            *) exit 0 # getopts outputs error message
            ;;
        esac
    done
}

########################################
# name: plot_mbtrnpp
# description: generate mbtrnpp plot images
# args:
#     QU_LOG_PATH    - log file directory
#     QU_SESSION_ID  - log session id (yyyymmdd-hhmmss)
#     QU_DATA_SET_ID - data set ID (description)
# returnCode: none
########################################
plot_mbtrnpp(){
    #####################
    # mbtrnpp metrics
    #####################

    MBTRNPP_LOG="${QU_LOG_PATH}/mbtrnpp-${QU_SESSION_ID}.log"
    MBTRNPP_QPCONF="qp-mbtrnpp.conf.sh"

    # profile timing
    export QU_MBTRNPP_XT_GETALL_FILTER="p,mb_getall_xt"
    export QU_MBTRNPP_XT_GETALL_CSV="xt_mbtrnpp_getall.csv"
    export QU_MBTRNPP_XT_CYCLE_FILTER="p,mb_cycle_xt"
    export QU_MBTRNPP_XT_CYCLE_CSV="xt_mbtrnpp_cycle.csv"
    export QU_MBTRNPP_XT_STATS_FILTER="p,mb_stats_xt"
    export QU_MBTRNPP_XT_STATS_CSV="xt_mbtrnpp_stats.csv"
    export QU_MBTRNPP_XT_PING_FILTER="p,mb_ping_xt"
    export QU_MBTRNPP_XT_PING_CSV="xt_mbtrnpp_ping.csv"
    export QU_MBTRNPP_XT_FWRITE_FILTER="p,mb_fwrite"
    export QU_MBTRNPP_XT_FWRITE_CSV="xt_mbtrnpp_fwrite.csv"
    export QU_MBTRNPP_XT_PROCMB1_FILTER="p,mb_proc_mb1_xt"
    export QU_MBTRNPP_XT_PROCMB1_CSV="xt_mbtrnpp_procmb1.csv"
    export QU_MBTRNPP_XT_TRNUPDATE_FILTER="p,trn_update_xt"
    export QU_MBTRNPP_XT_TRNUPDATE_CSV="xt_mbtrnpp_trnupdate.csv"
    export QU_MBTRNPP_XT_TRNBIASEST_FILTER="p,trn_biasest_xt"
    export QU_MBTRNPP_XT_TRNBIASEST_CSV="xt_mbtrnpp_trnbiasest.csv"
    export QU_MBTRNPP_XT_TRNPROC_FILTER="p,trn_proc_xt"
    export QU_MBTRNPP_XT_TRNPROC_CSV="xt_mbtrnpp_trnproc.csv"
    export QU_MBTRNPP_XT_TRNPROCTRN_FILTER="p,trn_proc_trn_xt"
    export QU_MBTRNPP_XT_TRNPROCTRN_CSV="xt_mbtrnpp_trnproctrn.csv"
    # events
	export QU_MBTRNPP_E_TRNREINIT_FILTER="e,mb_reinit" #"e,mb_trn_reinit"
    export QU_MBTRNPP_E_TRNREINIT_CSV="e_mbtrnpp_reinit.csv"
    export QU_MBTRNPP_E_GAINLO_FILTER="e,mb_gain_lo"
    export QU_MBTRNPP_E_GAINLO_CSV="e_mbtrnpp_gainlo.csv"
    export QU_MBTRNPP_E_MBCON_FILTER="e,mb_con"
    export QU_MBTRNPP_E_MBCON_CSV="e_mbtrnpp_mbcon.csv"
    export QU_MBTRNPP_E_MBDIS_FILTER="e,mb_dis"
    export QU_MBTRNPP_E_MBDIS_CSV="e_mbtrnpp_mbdis.csv"
    export QU_MBTRNPP_E_MBXYOFFSET_FILTER="e,mb_xyoffset"
    export QU_MBTRNPP_E_MBXYOFFSET_CSV="e_mbtrnpp_mbxyoffset.csv"
    export QU_MBTRNPP_E_MBZOFFSET_FILTER="e,mb_offset_z"
    export QU_MBTRNPP_E_MBZOFFSET_CSV="e_mbtrnpp_mbzoffset.csv"
    export QU_MBTRNPP_E_MBTRNUCLI_RESET_FILTER="e,mb_trnucli_reset"
    export QU_MBTRNPP_E_MBTRNUCLI_RESET_CSV="e_mbtrnpp_mbtrnuclireset.csv"
    export QU_MBTRNPP_E_MBEOF_FILTER="e,mb_eof"
    export QU_MBTRNPP_E_MBEOF_CSV="e_mbtrnpp_mbeof.csv"
	export QU_MBTRNPP_E_MBNONSURVEY_FILTER="e,mb_nonsurvey"
	export QU_MBTRNPP_E_MBNONSURVEY_CSV="e_mbtrnpp_mbnonsurvey.csv"
    export QU_MBTRNPP_E_TRNUPUB_FILTER="e,trnu_pub_n"
    export QU_MBTRNPP_E_TRNUPUB_CSV="e_mbtrnpp_trnupub.csv"
    export QU_MBTRNPP_E_TRNUPUBEMPTY_FILTER="e,trnu_pubempty_n"
    export QU_MBTRNPP_E_TRNUPUBEMPTY_CSV="e_mbtrnpp_trnupubempty.csv"
    # errors
    export QU_MBTRNPP_E_EMBGETALL_FILTER="e,e_mbgetall"
    export QU_MBTRNPP_E_EMBGETALL_CSV="e_mbtrnpp_embgetall.csv"
    export QU_MBTRNPP_E_EMBFAIL_FILTER="e,e_mbfailure"
    export QU_MBTRNPP_E_EMBFAIL_CSV="e_mbtrnpp_embfail.csv"
    export QU_MBTRNPP_E_EMBSOCK_FILTER="e,e_mbsocket"
    export QU_MBTRNPP_E_EMBSOCK_CSV="e_mbtrnpp_embsock.csv"
    export QU_MBTRNPP_E_EMBCON_FILTER="e,e_mbcon"
    export QU_MBTRNPP_E_EMBCON_CSV="e_mbtrnpp_embcon.csv"
    export QU_MBTRNPP_E_EMBFRAMERD_FILTER="e,e_mb_frame_rd"
    export QU_MBTRNPP_E_EMBFRAMERD_CSV="e_mbtrnpp_embframerd.csv"
    export QU_MBTRNPP_E_EMBLOGWR_FILTER="e,e_mb_log_wr"
    export QU_MBTRNPP_E_EMBLOGWR_CSV="e_mbtrnpp_emlogwr.csv"
    export QU_MBTRNPP_E_ETRNUPUB_FILTER="e,e_trnu_pub,"
    export QU_MBTRNPP_E_ETRNUPUB_CSV="e_mbtrnpp_etrnupub.csv"
    export QU_MBTRNPP_E_ETRNUPUBEMPTY_FILTER="e,e_trnu_pubempty"
    export QU_MBTRNPP_E_ETRNUPUBEMPTY_CSV="e_mbtrnpp_etrnupubempty.csv"

    if [ -f "${MBTRNPP_LOG}" ] && [ -f "${QP_PLOT_HOME}/${MBTRNPP_QPCONF}" ]
    then
        vout " processing ${MBTRNPP_LOG}"
        grep ${QU_MBTRNPP_XT_GETALL_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_XT_GETALL_CSV}
        grep ${QU_MBTRNPP_XT_CYCLE_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_XT_CYCLE_CSV}
        grep ${QU_MBTRNPP_XT_STATS_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_XT_STATS_CSV}
        grep ${QU_MBTRNPP_XT_PING_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_XT_PING_CSV}
        grep ${QU_MBTRNPP_XT_FWRITE_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_XT_FWRITE_CSV}
        grep ${QU_MBTRNPP_XT_PROCMB1_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_XT_PROCMB1_CSV}
        grep ${QU_MBTRNPP_XT_PROCMB1_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_XT_PROCMB1_CSV}
        grep ${QU_MBTRNPP_XT_TRNUPDATE_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_XT_TRNUPDATE_CSV}
        grep ${QU_MBTRNPP_XT_TRNBIASEST_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_XT_TRNBIASEST_CSV}
        grep ${QU_MBTRNPP_XT_TRNPROC_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_XT_TRNPROC_CSV}
        grep ${QU_MBTRNPP_XT_TRNPROCTRN_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_XT_TRNPROCTRN_CSV}
        grep ${QU_MBTRNPP_E_TRNREINIT_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_E_TRNREINIT_CSV}
        grep ${QU_MBTRNPP_E_GAINLO_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_E_GAINLO_CSV}
        grep ${QU_MBTRNPP_E_MBCON_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_E_MBCON_CSV}
        grep ${QU_MBTRNPP_E_MBDIS_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_E_MBDIS_CSV}
        grep ${QU_MBTRNPP_E_MBNONSURVEY_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_E_MBNONSURVEY_CSV}
        grep ${QU_MBTRNPP_E_TRNUPUB_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_E_TRNUPUB_CSV}
        grep ${QU_MBTRNPP_E_TRNUPUBEMPTY_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_E_TRNUPUBEMPTY_CSV}

        grep ${QU_MBTRNPP_E_EMBGETALL_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_E_EMBGETALL_CSV}
        grep ${QU_MBTRNPP_E_EMBFAIL_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_E_EMBFAIL_CSV}
        grep ${QU_MBTRNPP_E_EMBSOCK_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_E_EMBSOCK_CSV}
        grep ${QU_MBTRNPP_E_EMBCON_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_E_EMBCON_CSV}
        grep ${QU_MBTRNPP_E_EMBFRAMERD_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_E_EMBFRAMERD_CSV}
        grep ${QU_MBTRNPP_E_EMBLOGWR_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_E_EMBLOGWR_CSV}
        grep ${QU_MBTRNPP_E_MBXYOFFSET_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_E_MBXYOFFSET_CSV}
		grep ${QU_MBTRNPP_E_MBZOFFSET_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_E_MBZOFFSET_CSV}
		grep ${QU_MBTRNPP_E_MBTRNUCLI_RESET_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_E_MBTRNUCLI_RESET_CSV}
		grep ${QU_MBTRNPP_E_MBEOF_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_E_MBEOF_CSV}
        grep ${QU_MBTRNPP_E_ETRNUPUB_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_E_ETRNUPUB_CSV}
        grep ${QU_MBTRNPP_E_ETRNUPUBEMPTY_FILTER} ${MBTRNPP_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MBTRNPP_E_ETRNUPUBEMPTY_CSV}

        # use qplot to generate plot set
		${QPLOT_CMD} -f ${QP_PLOT_HOME}/${MBTRNPP_QPCONF}

    else
        echo "ERR - MBTRNPP_LOG log not found ${MBTRNPP_LOG}"
        echo "ERR - MBTRNPP_QPCONF log not found ${QP_PLOT_HOME}/${MBTRNPP_QPCONF}"
    fi

}

########################################
# name: plot_trnu
# description: generate trnu plot images
# args:
#     QU_LOG_PATH    - log file directory
#     QU_SESSION_ID  - log session id (yyyymmdd-hhmmss)
#     QU_DATA_SET_ID - data set ID (description)
# returnCode: none
########################################
plot_trnu(){
    #####################
    # trnu TRN output
    #####################

    TRNU_LOG="${QU_LOG_PATH}/trnu-${QU_SESSION_ID}.log"
    TRNU_QPCONF="qp-trnu.conf.sh"

    # TRN output and state
    export QU_TRNU_EST_FILTER="trn_est"
    export QU_TRNU_EST_CSV="trnu_est.csv"
    export QU_TRNU_MLE_FILTER="trn_mle_dat"
    export QU_TRNU_MLEDAT_CSV="trnu_mle.csv"
    export QU_TRNU_MSE_FILTER="trn_mse_dat"
    export QU_TRNU_MSEDAT_CSV="trnu_mmse.csv"
    export QU_TRNU_PT_FILTER="trn_pt_dat"
    export QU_TRNU_PTDAT_CSV="trnu_pt.csv"
    export QU_TRNU_RI_FILTER="trn_reinit"
    export QU_TRNU_RI_CSV="trnu_reinit.csv"
    export QU_TRNU_STATE_FILTER="trn_state"
    export QU_TRNU_STATE_CSV="trnu_state.csv"

    if [ -f "${TRNU_LOG}" ] && [ -f "${QP_PLOT_HOME}/${TRNU_QPCONF}" ]
    then
        vout " processing ${TRNU_LOG}"
        grep ${QU_TRNU_EST_FILTER} ${TRNU_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNU_EST_CSV}
        grep ${QU_TRNU_MLE_FILTER} ${TRNU_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNU_MLEDAT_CSV}
        grep ${QU_TRNU_MSE_FILTER} ${TRNU_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNU_MSEDAT_CSV}
        grep ${QU_TRNU_PT_FILTER} ${TRNU_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNU_PTDAT_CSV}
        grep ${QU_TRNU_RI_FILTER} ${TRNU_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNU_RI_CSV}
        grep ${QU_TRNU_STATE_FILTER} ${TRNU_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNU_STATE_CSV}

        # use qplot to generate plot set
        ${QPLOT_CMD} -f ${QP_PLOT_HOME}/${TRNU_QPCONF}

    else
        echo "ERR - TRNU_LOG log not found ${TRNU_LOG}"
        echo "ERR - TRNU_QPCONF log not found ${QP_PLOT_HOME}/${TRNU_QPCONF}"
    fi

}

########################################
# name: plot_trnusvr
# description: generate trnusvr plot images
# args:
#     QU_LOG_PATH    - log file directory
#     QU_SESSION_ID  - log session id (yyyymmdd-hhmmss)
#     QU_DATA_SET_ID - data set ID (description)
# returnCode: none
########################################
plot_trnusvr(){
    #####################
    # trnusvr metrics
    #####################

    TRNUSVR_LOG="${QU_LOG_PATH}/trnusvr-${QU_SESSION_ID}.log"
    TRNUSVR_QPCONF="qp-trnusvr.conf.sh"

    # profile timing
    export QU_TRNUSVR_XT_UDCON_FILTER="p,udcon_xt"
    export QU_TRNUSVR_XT_UDCON_CSV="xt_trnusvr_udcon.csv"
    export QU_TRNUSVR_XT_CHKHB_FILTER="p,chkhb_xt"
    export QU_TRNUSVR_XT_CHKHB_CSV="xt_trnusvr_chkhb.csv"
    export QU_TRNUSVR_XT_READ_FILTER="p,read_xt"
    export QU_TRNUSVR_XT_READ_CSV="xt_trnusvr_read.csv"
    export QU_TRNUSVR_XT_HANDLE_FILTER="p,handle_xt"
    export QU_TRNUSVR_XT_HANDLE_CSV="xt_trnusvr_handle.csv"
    export QU_TRNUSVR_XT_REQRES_FILTER="p,reqres_xt"
    export QU_TRNUSVR_XT_REQRES_CSV="xt_trnusvr_reqres.csv"
    export QU_TRNUSVR_XT_PUB_FILTER="p,pub_xt"

    # client activity
    export QU_TRNUSVR_XT_PUB_CSV="xt_trnusvr_pub.csv"
    export QU_TRNUSVR_CLICON_FILTER="cli_con"
    export QU_TRNUSVR_CLICON_CSV="trnusvr_clicon.csv"
    export QU_TRNUSVR_CLIDIS_FILTER="cli_dis"
    export QU_TRNUSVR_CLIDIS_CSV="trnusvr_clidis.csv"
    export QU_TRNUSVR_CLILL_FILTER="cli_list_len"
    export QU_TRNUSVR_CLILL_CSV="trnusvr_clill.csv"


    if [ -f "${TRNUSVR_LOG}" ] && [ -f "${QP_PLOT_HOME}/${TRNUSVR_QPCONF}" ]
    then
        vout " processing ${TRNUSVR_LOG}"
        grep ${QU_TRNUSVR_XT_UDCON_FILTER} ${TRNUSVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNUSVR_XT_UDCON_CSV}
        grep ${QU_TRNUSVR_XT_CHKHB_FILTER} ${TRNUSVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNUSVR_XT_CHKHB_CSV}
        grep ${QU_TRNUSVR_XT_READ_FILTER} ${TRNUSVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNUSVR_XT_READ_CSV}
        grep ${QU_TRNUSVR_XT_HANDLE_FILTER} ${TRNUSVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNUSVR_XT_HANDLE_CSV}
        grep ${QU_TRNUSVR_XT_REQRES_FILTER} ${TRNUSVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNUSVR_XT_REQRES_CSV}
        grep ${QU_TRNUSVR_XT_PUB_FILTER} ${TRNUSVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNUSVR_XT_PUB_CSV}
        grep ${QU_TRNUSVR_CLICON_FILTER} ${TRNUSVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNUSVR_CLICON_CSV}
        grep ${QU_TRNUSVR_CLIDIS_FILTER} ${TRNUSVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNUSVR_CLIDIS_CSV}
        grep ${QU_TRNUSVR_CLILL_FILTER} ${TRNUSVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNUSVR_CLILL_CSV}

        # use qplot to generate plot set
        ${QPLOT_CMD} -f ${QP_PLOT_HOME}/${TRNUSVR_QPCONF}
    else
        echo "ERR - TRNUSVR_LOG log not found ${TRNUSVR_LOG}"
        echo "ERR - TRNUSVR_QPCONF log not found ${QP_PLOT_HOME}/${TRNUSVR_QPCONF}"
    fi
}

########################################
# name: plot_trnsvr
# description: generate trnsvr plot images
# args:
#     QU_LOG_PATH    - log file directory
#     QU_SESSION_ID  - log session id (yyyymmdd-hhmmss)
#     QU_DATA_SET_ID - data set ID (description)
# returnCode: none
########################################
plot_trnsvr(){
    #####################
    # trnsvr metrics
    #####################

    TRNSVR_LOG="${QU_LOG_PATH}/trnsvr-${QU_SESSION_ID}.log"
    TRNSVR_QPCONF="qp-trnsvr.conf.sh"

    # profile timing
    export QU_TRNSVR_XT_UDCON_FILTER="p,udcon_xt"
    export QU_TRNSVR_XT_UDCON_CSV="xt_trnsvr_udcon.csv"
    export QU_TRNSVR_XT_CHKHB_FILTER="p,chkhb_xt"
    export QU_TRNSVR_XT_CHKHB_CSV="xt_trnsvr_chkhb.csv"
    export QU_TRNSVR_XT_READ_FILTER="p,read_xt"
    export QU_TRNSVR_XT_READ_CSV="xt_trnsvr_read.csv"
    export QU_TRNSVR_XT_HANDLE_FILTER="p,handle_xt"
    export QU_TRNSVR_XT_HANDLE_CSV="xt_trnsvr_handle.csv"
    export QU_TRNSVR_XT_REQRES_FILTER="p,reqres_xt"
    export QU_TRNSVR_XT_REQRES_CSV="xt_trnsvr_reqres.csv"
    export QU_TRNSVR_XT_PUB_FILTER="p,pub_xt"
    export QU_TRNSVR_XT_PUB_CSV="xt_trnsvr_pub.csv"

    # client activity
    export QU_TRNSVR_CLICON_FILTER="cli_con"
    export QU_TRNSVR_CLICON_CSV="trnsvr_clicon.csv"
    export QU_TRNSVR_CLIDIS_FILTER="cli_dis"
    export QU_TRNSVR_CLIDIS_CSV="trnsvr_clidis.csv"
    export QU_TRNSVR_CLILL_FILTER="cli_list_len"
    export QU_TRNSVR_CLILL_CSV="trnsvr_clill.csv"


    if [ -f "${TRNSVR_LOG}" ] && [ -f "${QP_PLOT_HOME}/${TRNSVR_QPCONF}" ]
    then
        vout " processing ${TRNSVR_LOG}"
        grep ${QU_TRNSVR_XT_UDCON_FILTER} ${TRNSVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNSVR_XT_UDCON_CSV}
        grep ${QU_TRNSVR_XT_CHKHB_FILTER} ${TRNSVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNSVR_XT_CHKHB_CSV}
        grep ${QU_TRNSVR_XT_READ_FILTER} ${TRNSVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNSVR_XT_READ_CSV}
        grep ${QU_TRNSVR_XT_HANDLE_FILTER} ${TRNSVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNSVR_XT_HANDLE_CSV}
        grep ${QU_TRNSVR_XT_REQRES_FILTER} ${TRNSVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNSVR_XT_REQRES_CSV}
        grep ${QU_TRNSVR_XT_PUB_FILTER} ${TRNSVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNSVR_XT_PUB_CSV}
        grep ${QU_TRNSVR_CLICON_FILTER} ${TRNSVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNSVR_CLICON_CSV}
        grep ${QU_TRNSVR_CLIDIS_FILTER} ${TRNSVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNSVR_CLIDIS_CSV}
        grep ${QU_TRNSVR_CLILL_FILTER} ${TRNSVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNSVR_CLILL_CSV}

        # use qplot to generate plot set
        ${QPLOT_CMD} -f ${QP_PLOT_HOME}/${TRNSVR_QPCONF}
    else
        echo "ERR - TRNSVR_LOG log not found ${TRNSVR_LOG}"
        echo "ERR - TRNSVR_QPCONF log not found ${QP_PLOT_HOME}/${TRNSVR_QPCONF}"
    fi
}

########################################
# name: plot_mb1svr
# description: generate mb1svr plot images
# args:
#     QU_LOG_PATH    - log file directory
#     QU_SESSION_ID  - log session id (yyyymmdd-hhmmss)
#     QU_DATA_SET_ID - data set ID (description)
# returnCode: none
########################################
plot_mb1svr(){
    #####################
    # mb1svr metrics
    #####################

    MB1SVR_LOG="${QU_LOG_PATH}/mb1svr-${QU_SESSION_ID}.log"
    MB1SVR_QPCONF="qp-mb1svr.conf.sh"

    # profile timing
    export QU_MB1SVR_XT_UDCON_FILTER="p,udcon_xt"
    export QU_MB1SVR_XT_UDCON_CSV="xt_mb1svr_udcon.csv"
    export QU_MB1SVR_XT_CHKHB_FILTER="p,chkhb_xt"
    export QU_MB1SVR_XT_CHKHB_CSV="xt_mb1svr_chkhb.csv"
    export QU_MB1SVR_XT_READ_FILTER="p,read_xt"
    export QU_MB1SVR_XT_READ_CSV="xt_mb1svr_read.csv"
    export QU_MB1SVR_XT_HANDLE_FILTER="p,handle_xt"
    export QU_MB1SVR_XT_HANDLE_CSV="xt_mb1svr_handle.csv"
    export QU_MB1SVR_XT_REQRES_FILTER="p,reqres_xt"
    export QU_MB1SVR_XT_REQRES_CSV="xt_mb1svr_reqres.csv"
    export QU_MB1SVR_XT_PUB_FILTER="p,pub_xt"

    # client activity
    export QU_MB1SVR_XT_PUB_CSV="xt_mb1svr_pub.csv"
    export QU_MB1SVR_CLICON_FILTER="cli_con"
    export QU_MB1SVR_CLICON_CSV="mb1svr_clicon.csv"
    export QU_MB1SVR_CLIDIS_FILTER="cli_dis"
    export QU_MB1SVR_CLIDIS_CSV="mb1svr_clidis.csv"
    export QU_MB1SVR_CLILL_FILTER="cli_list_len"
    export QU_MB1SVR_CLILL_CSV="mb1svr_clill.csv"


    if [ -f "${MB1SVR_LOG}" ] && [ -f "${QP_PLOT_HOME}/${MB1SVR_QPCONF}" ]
    then
        vout " processing ${MB1SVR_LOG}"
        grep ${QU_MB1SVR_XT_UDCON_FILTER} ${MB1SVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MB1SVR_XT_UDCON_CSV}
        grep ${QU_MB1SVR_XT_CHKHB_FILTER} ${MB1SVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MB1SVR_XT_CHKHB_CSV}
        grep ${QU_MB1SVR_XT_READ_FILTER} ${MB1SVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MB1SVR_XT_READ_CSV}
        grep ${QU_MB1SVR_XT_HANDLE_FILTER} ${MB1SVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MB1SVR_XT_HANDLE_CSV}
        grep ${QU_MB1SVR_XT_REQRES_FILTER} ${MB1SVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MB1SVR_XT_REQRES_CSV}
        grep ${QU_MB1SVR_XT_PUB_FILTER} ${MB1SVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MB1SVR_XT_PUB_CSV}
        grep ${QU_MB1SVR_CLICON_FILTER} ${MB1SVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MB1SVR_CLICON_CSV}
        grep ${QU_MB1SVR_CLIDIS_FILTER} ${MB1SVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MB1SVR_CLIDIS_CSV}
        grep ${QU_MB1SVR_CLILL_FILTER} ${MB1SVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_MB1SVR_CLILL_CSV}

        # use qplot to generate plot set
        ${QPLOT_CMD} -f ${QP_PLOT_HOME}/${MB1SVR_QPCONF}
    else
        echo "ERR - MB1SVR_LOG log not found ${MB1SVR_LOG}"
        echo "ERR - MB1SVR_QPCONF log not found ${QP_PLOT_HOME}/${MB1SVR_QPCONF}"
    fi
}

########################################
# name: plot_mbtrnpp_logs
# description: generate plots for
#              on one set of mbtrnpp logs
# args:
#     QU_LOG_PATH    - log file directory
#     QU_SESSION_ID  - log session id (yyyymmdd-hhmmss)
#     QU_DATA_SET_ID - data set ID (description)
# returnCode: none
########################################
plot_mbtrnpp_logs(){
    vout " QU_LOG_PATH    - ${QU_LOG_PATH}"
    vout " QU_SESSION_ID  - ${QU_SESSION_ID}"
    vout " QU_DATA_SET_ID - ${QU_DATA_SET_ID}"

	# generate plot CSV and images...

	# plot mbtrnpp metrics
	plot_mbtrnpp
	# plot trnu (TRN pub/sub) output, metrics
	plot_trnu
	# plot trnusvr (TRN pub/sub server) metrics
    plot_trnusvr
	# plot trnsvr (TRN TCP/legacy server) metrics
	plot_trnsvr
	# plot mb1svr (TRN MB1 server) metrics
	plot_mb1svr

    # qplot PDF combiner job configuration
    COMB_QPCONF="qp-comb.conf.sh"

    # generate combined PDF plot set with qplot
    ${QPLOT_CMD} -f ${QP_PLOT_HOME}/${COMB_QPCONF}

} # end function

########################################
# name: run_jobs
# description: run jobs in queue
# args: none
# returnCode: none
########################################

run_jobs(){
    let "i=0"
    echo ""
    while [ $i -lt $QX_PLOTSET_COUNT ]
    do
        # set parameters for this job
        QU_LOG_PATH=${QX_LOG_PATHS[$i]}
        export QU_SESSION_ID=${QX_SESSION_IDS[$i]}
		export QU_DATA_SET_ID="${QX_DATA_SET_IDS[$i]}"

		QX_JOB_N_DIR="${QX_JOB_DIR}/job-${i}-$$"
		vout "running job : [$i]"
		vout "     output : [${QX_JOB_N_DIR}]"
		# generate plot data and plots
        plot_mbtrnpp_logs
        vout ""
        if [ ! -d "${QX_JOB_N_DIR}" ]
        then
            mkdir -p ${QX_JOB_N_DIR}
        fi

        if [ -d ${QX_JOB_N_DIR} ] && [ -d ${QP_OUTPUT_DIR} ]  && [ -d ${QP_PLOT_DATA_DIR} ]
        then
            mv ${QP_OUTPUT_DIR}/*png ${QX_JOB_N_DIR}
            mv ${QP_OUTPUT_DIR}/*pdf ${QX_JOB_N_DIR}
            mv ${QP_PLOT_DATA_DIR}/*csv ${QX_JOB_N_DIR}
            mv ${QP_OUTPUT_DIR}/*gp ${QX_JOB_N_DIR} &> /dev/null
            echo "QU_LOG_PATH    - $QU_LOG_PATH" > ${QX_JOB_N_DIR}/README
            echo "QU_SESSION_ID  - $QU_SESSION_ID" >> ${QX_JOB_N_DIR}/README
            echo "QU_DATA_SET_ID - $QU_DATA_SET_ID" >> ${QX_JOB_N_DIR}/README
        else
            echo "WARN - could not create directory [${QX_JOB_N_DIR}]"
        fi
        let "i=$i+1"
    done
}

##########################
# Script main entry point
##########################

# process command line options
if [ -t 0 ]
then
    if [ "$#" -eq 0 ];then
        show_help
        exit -1
    else
        # this ensures that quoted whitespace is preserved by getopts
        # note use of $@, in quotes

        # originally was process_cmdline $*
        process_cmdline "$@"
        let "i=$OPTIND-1"
        while [ "${i}" -gt 0  ]
        do
            shift
            let "i-=1"
        done
    fi
fi

# show plotset definitions
if [ ! -z $DO_LIST ]
then
	list_keys
fi

# show help and exit
if [ ! -z $DO_HELP ]
then
	show_help
	exit 0
fi

# create plot data directory if it doesn't exist
if [ ! -d "${QP_PLOT_DATA_DIR}" ]
then
	vout "creating plot data dir : ${QP_PLOT_DATA_DIR}"
    mkdir -p ${QP_PLOT_DATA_DIR}
    if [ ! -d "${QP_PLOT_DATA_DIR}" ]
    then
    echo "could not create plot data dir : ${QP_PLOT_DATA_DIR}"
    exit -1
    fi
fi
# create plot output directory if it doesn't exist
if [ ! -d "${QP_OUTPUT_DIR}" ]
then
	vout "creating plot data dir : ${QP_PLOT_DATA_DIR}"
	mkdir -p ${QP_OUTPUT_DIR}
fi
# create plot job output directory if it doesn't exist
if [ ! -d "${QX_JOB_DIR}" ]
then
	vout "creating job dir : ${QP_PLOT_DATA_DIR}"
	mkdir -p ${QX_JOB_DIR}
    if [ ! -d "${QX_JOB_DIR}" ]
    then
    echo "could not create job dir : ${QX_JOB_DIR}"
    exit -1
    fi
fi

# process jobs
run_jobs

vout ""

exit 0
