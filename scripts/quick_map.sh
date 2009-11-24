#!/usr/bin/bash
#
#  Command to automatically generate a plot from data held in /swath4/raw
#
#  usage: quick_map.sh region [ mbm_plot_options ]
#                      region is the argument to the -R option
#
#  Easier usage is to use map_swath and select map type 7
#
#  This is a cut down version of swath_plot.sh optimised to minimise time 
#
#  Author Gordon Keith - CSIRO Marine - 20 April 2004
#
#  Revision History
#    20040420 GJK Initial version of swath_process.sh
#    20040427 GJK Cut down to minimum for faster processing.

REGION=$1
PLOT_OPTIONS=$2
CLEAN_OPTIONS=$3

SRC=$4
DST=$5

if [ -z "$REGION" ] ; then
	echo
	echo usage $0 west/east/south/north [ \"mbm_plot options\" ]
	echo example:
	echo $0 149/150/-38.5/-38 \"-G2 -C100 -PA3\"
	exit
fi


[ -z "$MAPNAME" ] && MAPNAME=map
mapname=${MAPNAME}_`echo $REGION | sed 's:/:_:g'`

DATALIST=${mapname}_dataset.mb-1

MB_PS_VIEWER=gv
export MB_PS_VIEWER

[ -z "$PLOT_OPTIONS" ] && PLOT_OPTIONS="-G 2 -P A4"

[ -z "$SRC" ] && SRC=/swath4/raw
[ -z "$DST" ] && DST=$SWATHDIR
[ -z "$DST" ] && DST=/swath4/mbproc/raw

# Create list of lines to process

processDatalist=$DATALIST

echo $DST/datalist.mb-1 -1 1 > $DATALIST

if [ ! -z $REGION ] ; then
    echo `date` Compiling data list...
    processDatalist=${mapname}_datalist.mb-1
    echo '$PROCESSED' > $processDatalist
    nice mbdatalist -I $DATALIST -O 
    nice mbdatalist -I $DATALIST -R$REGION >> $processDatalist 
fi

echo `date` Preparing plot...
nice mbm_plot -F-1 -I $processDatalist -O $mapname -R$REGION $PLOT_OPTIONS


echo `date` Plotting...
./$mapname.cmd

sleep 1

echo
echo Map $mapname.ps should have been created.

echo
echo Press Enter to continue
read end


