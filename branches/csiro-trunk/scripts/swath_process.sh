#!/bin/bash
#
#  Command to automatically generate a plot from data held in /swath4/raw
#
#  usage: swath_process.sh region mbm_grdplot_options mbclean_options mbgrid_options mbm_grdtiff_options
#                          region is the argument to the -R option
#
#  Author Gordon Keith - CSIRO Marine - 20 April 2004
#
#  Revision history
#    20040420 GJK Inital version
#    20040428 GJK Added griding support, it works, it needs work, not fully tested
#                 End of voyage SS0404 version 
#    20041026 GJK Remove need to change directory

REGION=$1
PLOT_OPTIONS=$2
CLEAN_OPTIONS=$3
GRID_OPTIONS=$4
TIFF_OPTIONS=$5

SRC=$6
DST=$7

[ -z "$MAPNAME" ] && MAPNAME=map
mapname=${MAPNAME}_`echo $REGION | sed 's:/:_:g'`

[ -z "$MB_PS_VIEWER" ] && MB_PS_VIEWER=gv
export MB_PS_VIEWER

[ -z "$PLOT_OPTIONS" ] && PLOT_OPTIONS="-G2 -P A4"
[ -z "$GRID_OPTIONS" ] && GRID_OPTIONS="-D1001/1001 -F5"
[ -z "$LABEL" ] && LABEL=$MAPNAME
[ "map" = "$LABEL" ] && LABEL=$mapname
[ -z "$SRC" ] && SRC=$SWATHSRC
[ -z "$DST" ] && DST=$SWATHDIR
[ -z "$SRC" ] && SRC=/swath4/raw
[ -z "$DST" ] && DST=/swath4/mbproc/raw
[ ! -d "$DST" ] && DST=/home/swath/mbproc/em300


DATALIST=${mapname}_dataset.mb-1

# Convert any new swath data to MBSystem format

swath_update.sh $SRC $DST



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

if [ "$CLEAN_OPTIONS" != "NONE" ] ; then
    echo `date` Cleaning data...

    if [ -z "$CLEAN_OPTIONS" ] ; then
        # flag data: with a spikes of greater than 20 degrees along or across track 
        #            kill pings with less than 20 good beams
        nice mbclean -F-1 -I $processDatalist  -S20/3/2 -U20 
    else
	nice mbclean -F-1 -I $processDatalist $CLEAN_OPTIONS
    fi
fi

echo `date` Processing data...
nice mbprocess -I $processDatalist

echo `date` Preparing plot...
echo mbgrid -I $processDatalist -O $mapname -R$REGION $GRID_OPTIONS
nice mbgrid -I $processDatalist -O $mapname -R$REGION $GRID_OPTIONS

if [ -z "$PLOT3D" ] ; then
	echo mbm_grdplot -I $mapname.grd -O $mapname -R$REGION $PLOT_OPTIONS -L "$LABEL"
	nice mbm_grdplot -I $mapname.grd -O $mapname -R$REGION $PLOT_OPTIONS -L "$LABEL"
else
        echo mbm_grd3dplot -I $mapname.grd -O $mapname -R$REGION $PLOT_OPTIONS -L "$LABEL"
        nice mbm_grd3dplot -I $mapname.grd -O $mapname -R$REGION $PLOT_OPTIONS -L "$LABEL"
fi

if [ ! -z "$TIFF_OPTIONS" ] ; then
	echo mbm_grdtiff  -I $mapname.grd -O ${mapname} -R$REGION $TIFF_OPTIONS
	nice mbm_grdtiff  -I $mapname.grd -O ${mapname} -R$REGION $TIFF_OPTIONS
fi


echo `date` Plotting...
./$mapname.cmd
if [ ! -z "$TIFF_OPTIONS" ] ; then
	./${mapname}_tiff.cmd
        listgeo -tfw ${mapname}.tif
fi

sleep 1

echo REGION=$REGION

echo
echo Map $mapname.ps should have been created.
if [ ! -z "$TIFF_OPTIONS" ] ; then
	echo GeoTiff ${mapname}.tif should have been created.
fi

if [ -z "NOWAIT" ] ; then
	echo
	echo Press Enter to continue
	read end
fi

