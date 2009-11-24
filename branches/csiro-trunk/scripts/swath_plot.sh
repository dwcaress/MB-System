#!/bin/bash
#
#  Command to automatically generate a plot from data held in /swath4/raw
#
#  usage: swath_plot.sh region mbm_plot_options mbclean_options
#                       region is the argument to the -R option
#
#  Easier usage is to run map_swath and select map type 6
#
#  Author Gordon Keith - CSIRO Marine - 20 April 2004
#
#  Revision History
#
#    20040420 GJK Initial version
#    20040428 GJK Found mbm_grdplot and got it working in swath_process.sh
#                 this is the original swath_process.sh and is now
#                 mostly obsolete. It is still used for vessel track only plots
#

REGION=$1
PLOT_OPTIONS=$2
CLEAN_OPTIONS=$3

SRC=$4
DST=$5

RAWEXT=_raw.all
MBEXT=.mb57

[ -z "$MAPNAME" ] && MAPNAME=map
mapname=${MAPNAME}_`echo $REGION | sed 's:/:_:g'`

MB_PS_VIEWER=gv
export MB_PS_VIEWER

[ -z "$PLOT_OPTIONS" ] && PLOT_OPTIONS="-G 2 -P A4"

[ -z "$SRC" ] && SRC=$SWATHSRC
[ -z "$DST" ] && DST=$SWATHDIR
[ -z "$SRC" ] && SRC=/swath4/raw
[ -z "$DST" ] && DST=/swath4/mbproc/raw
[ ! -d "$DST" ] && DST=/home/swath/mbproc/em300

DATALIST=${mapname}_dataset.mb-1

# Ensure all raw files under $SRC are mbcopy'ed to $DST
swath_update.sh $SRC $DST

# Create list of lines to process

processDatalist=$DATALIST

echo $DST/datalist.mb-1 -1 1 > $DATALIST

if [ ! -z "$REGION" ] ; then
    echo `date` Compiling data list...
    processDatalist=${mapname}_datalist.mb-1
    echo '$PROCESSED' > $processDatalist
    nice mbdatalist -I $DATALIST -O 
    nice mbdatalist -I $DATALIST -R$REGION >> $processDatalist 
fi

if [ "$CLEAN_OPTIONS" != "NONE" ] ; then
    echo `date` Cleaning data...

    if [ -z "$CLEAN_OPTIONS" ] ; then
        # flag data: with a spikes greater than 20 degrees along or across track
        #            kill pings with less than 20 good beams
        nice mbclean -F-1 -I $processDatalist  -S20/3/2 -U20 
    else
	nice mbclean -F-1 -I $processDatalist $CLEAN_OPTIONS
    fi
fi

echo `date` Processing data...
nice mbprocess -I $processDatalist

echo `date` Preparing plot...
nice mbm_plot -F-1 -I $processDatalist -O $mapname -R$REGION $PLOT_OPTIONS


echo `date` Plotting...
./$mapname.cmd

sleep 1

echo REGION=$REGION

echo
echo Map $mapname.ps should have been created.

if [ -z "NOWAIT" ] ; then
        echo
        echo Press Enter to continue
        read end
fi


