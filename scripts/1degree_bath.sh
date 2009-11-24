#!/bin/sh
umask 002
if [ `id -gn` != echo-hf ] ; then
	sg echo-hf -c "$0 $*"
	exit 0
fi


west=$1
east=$2
south=$3
north=$4

export SWATHDIR=/home/swath/mbproc/public_datalist.mb-1

lon=$west
while [ $lon -lt $east ] ; do
	nextlon=$[ $lon + 1 ]
	lat=$south
	while [ $lat -lt $north ] ; do
		nextlat=$[ $lat + 1 ]
		export REGION=$lon/$nextlon/$lat/$nextlat
                region=`echo $REGION | sed s:/:_:g`

		grid_swath bathgrid
	
                g=`ls -t *$region*.grd | head -1`

                if [ -z "$g" ] ; then
                        rm *$region*mb-1
                elif [  -f "$g" -a `grdinfo "$g" | grep -c e+308` = 0 ] ; then

                        map_grid bathtif
                        map_grid slopetif

                        region=`echo $REGION | sed s:/:_:g`

                        listgeo bathymetry_${region}_bathy.tif > $region.geo

                        for type in bathy slope ; do

		               gdaladdo --config COMPRESS_OVERVIEW JPEG --config PHOTOMETRIC_OVERVIEW YCBCR --config INTERLEAVE_OVERVIEW PIXEL -r nearest bathy_${region}_${type}.tif 2 4 8 16 32 64
			done
                elif [ ! -z "$g" ] ; then

                        echo rm "$g"
                        f=`basename "$g" .grd`
                        rm "*$f*"
                fi


		lat=$nextlat
	done
	lon=$nextlon
done

