#!/bin/sh
umask 002
if [ `id -gn` != echo-hf ] ; then
        sg echo-hf -c "$0 $*"
        exit 0
fi


#
# Initialise control files from default if not present
#

if [ ! -f backgrid ] ; then 

cat > backgrid << EOF
gridtype:       5
options:        y
grid:           -D4096/4096
interp:
priorities:     -F1/1 -Ymosaic_weights
mapname:        back300 0002 #
extras:         -S5

EOF
fi

if [ ! -f mosaic_weights ] ; then

cat > mosaic_weights << EOF
-80.0 0.1
-60.0 0.2
-45.0 1.0
-15.0 0.8
-14.9 0.1
14.9 0.1
15.0 0.8
45.0 1.0
60.0 0.2
80.0 0.1
EOF
fi

if [ ! -f backtif ] ; then

cat > backtif << EOF
maptype:        4
grid:
options:        y
tiff:           o
mapname:
backrange:      -40/-20
tiff extras:    -Wbs-40_-20.cpt

EOF
fi

if [ ! -f back3tif ] ; then

cat > back3tif << EOF
maptype:        4
grid:
options:        y
tiff:           o
mapname:        $ 3c
backrange:      d
tiff extras:    -W hs_30_34.cpt

EOF
fi

if [ ! -f bs-40_-20.cpt ] ; then
cat > bs-40_-20.cpt << EOF
N 255
B 254
F 2
-40 254 -38 229
-38 229 -36 204
-36 204 -34 178
-34 178 -32 153
-32 153 -30 127
-30 127 -28 102
-28 102 -26 75
-26 75 -24 51
-24 51 -22 25
-22 25 -20 2
EOF
fi


if [ ! -f hs_30_34.cpt ] ; then

cat > hs_30_34.cpt << EOF
N 255
B 255
F 0
-65 0 255 0 -34 0 255 0
-34 255 255 0 -30 255 255 0
-30 255 0 0 15 255 0 0
EOF
fi

#
# Process
#

west=$1
east=$2
south=$3
north=$4

export SWATHDIR=/home/swath/mbproc/em300/public_datalist.mb-1

lon=$west
while [ $lon -lt $east ] ; do
	nextlon=$[ $lon + 1 ]
	lat=$south
	while [ $lat -lt $north ] ; do
		nextlat=$[ $lat + 1 ]
		export REGION=$lon/$nextlon/$lat/$nextlat
		region=`echo $REGION | sed s:/:_:g `

		if [ -z "$RECOLOUR_ONLY" ] ; then
			grid_swath backgrid
		fi
	

                g=`ls -t *$region*.grd | head -1`
		
		if [ -z "$g" ] ; then
			rm *$region*mb-1
		elif [  -f "$g" -a `grdinfo "$g" | grep -c e+308` = 0 ] ; then
			map_grid backtif
                        gdaladdo --config COMPRESS_OVERVIEW JPEG --config PHOTOMETRIC_OVERVIEW YCBCR --config INTERLEAVE_OVERVIEW PIXEL back300_0002_${region}_back.tif 2 4 8 16 32 64 128 256
			map_grid back3tif
                        gdaladdo --config COMPRESS_OVERVIEW JPEG --config PHOTOMETRIC_OVERVIEW YCBCR --config INTERLEAVE_OVERVIEW PIXEL -r nearest back300_0002_${region}_back_3c.tif 2 4 8 16 32 64 128 256

		elif [ ! -z "$g" ] ; then
		
        		echo rm "$g"
		        f=`basename "$g" .grd`
		        rm "*$f*"
		fi

		lat=$nextlat
	done
	lon=$nextlon
done

