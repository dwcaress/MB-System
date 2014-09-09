#!/bin/sh  -x

# Script to process swath data to generate a mosaic of geotiffs 

# usage: Called by a script which sets the environment variables listed below.

# run as group echo-hf with group write enabled

umask 002
if [ `id -gn` != echo-hf ] ; then
	sg echo-hf -c "$0 $*"
	exit 0
fi

######################
#
# The following environment variables should all be set by the calling program
# 
# WEST - western edge of data set (integer degrees)
# SOUTH - southern edge of data set (integer degrees)
# FACTOR - number of tiles per degree at level 1 (1 or 8)
# PIXELS - number of pixels per tile edge (integer: 400 for .0025 degree, 512 otherwise)
# SWATHDIR - datalist for data to include (full path or directory or datalist of public, Sourthen Surveyor or all)
# TYPE - type of data ("bathy", "bathslope" or "back")
#
# west - western edge of data to process (integer degrees)
# east - eastern edge of data to process (integer degrees)
# south - southern edge of data to process (integer degrees)
# north - northen edge fo data to process (integer degrees)
# step - level in pyarmid (integer power of 2)
#

[ -z "$WEST" ]		&& WEST=110
[ -z "$SOUTH" ]		&& SOUTH=-64
[ -z "$FACTOR" ]	&& FACTOR=8
[ -z "$PIXELS" ]	&& PIXELS=512
[ -z "$SWATHDIR" ]	&& SWATHDIR=/home/acoustics_swath/datalist.mb-1
[ -z "$TYPE" ]	 	&& TYPE=bathy
[ -z "$west" ] 		&& west=$WEST
[ -z "$east" ] 		&& east=$[ $WEST + 64 ]
[ -z "$south" ] 	&& south=$SOUTH
[ -z "$north" ] 	&& north=[ $SOUTH + 64 ]
[ -z "$step" ] 		&& step=1

######################

factor=$FACTOR

## bathymetry shaded by sun illumination
#
GRIDCMD="nice mbgrid -A1 -N -L1 -X.2 "
GRIDEXTRA="-F5 -C1/1 -X.5"

PREFIX=bathy
SUFFIX1=bathy
SUFFIX2=slope
SUFFIX3=profile
TIFF1="mbm_grdtiff -G2 -MGS-1 -A3.9999/45 -W earth.cpt"
TIFF2="mbm_grdtiff -G4 -W slope.cpt"
TIFF3="mbm_grdtiff -G4 -W profile.cpt"
TIFF1x=
TIFF2x=
TIFF3x=
SAMPLES1=3
SAMPLES2=3
SAMPLES3=3

## bathymetry shaded by slope
#
if [ $TYPE = bathslope ] ; then
	TIFF1="mbm_grdtiff -G5 -D0/1 -MGS-1 -W earth.cpt "
fi

## backscatter
#
if [ $TYPE = back ] ; then
	GRIDCMD="nice mbmosaic -A3F -N -L1 -X.2 -C/1 -F1/1/1 -Ymosaic_weights -S5 "
	GRIDEXTRA="-X.5"

	PREFIX=back300
	SUFFIX1=back
	SUFFIX2=back_3c
	SUFFIX3=
	TIFF1="mbgrdtiff -C bs-40_-20.cpt"
	TIFF2="mbgrdtiff -C hs_30_34.cpt"
	TIFF3=
	TIFF1x=.tif
	TIFF2x=.tif
	TIFF3x=.tif
	SAMPLES1=1
	SAMPLES2=3
	SAMPLES3=
fi

mkdir -p $step

if [ ! -z "$workdir" ] ; then
        if [ ! -d "$workdir" ] ; then
                mkdir -p $workdir
                ln -s `pwd`/$step "$workdir/$step"
                cp -u *.gmt "$workdir"
        fi
        cd "$workdir"
fi

filters=`ls *.gmt | wc -l`

if [ $factor -gt $[ $step + $step ] ] ; then
	grid_extras="$GRIDEXTRA"
else
	grid_extras=""
fi

#
# Create control files if not already present
#

if [ ! -f earth.cpt ] ; then

cat > earth.cpt <<EOF
-12000    0       0       0     -6000    66      65      66
-6000    66      65      66     -2500    99      63     133
-2500    99      63     133     -1500    63      62     175
-1500    63      62     175     -1000   147     217     219
-1000   147     217     219     -700     88     123      75
-700     88     123      75     -400    240     238      90
-400    240     238      90     -200    205     106     105
-200    205     106     105     -100    206     207     128
-100    206     207     128     0.5     252     252     248
0.5     138     236     174     100     172     245     168
100     205     255     162     200     223     245     141
200     240     236     121     500     247     215     104
500     255     189     87      1000    255     160     69
1000    244     117     75      1500    238     80      78
1500    255     90      90      2000    255     124     124
2000    255     158     158     2500    245     179     174
2500    255     196     196     3000    255     215     215
3000    255     235     235     4000    255     255     255
EOF
fi 
 

if [ ! -f slope.cpt ] ; then

cat > slope.cpt << EOF
F 0
N 255
0 255 255 255 0.035 255 255 0
0.035 255 255 0 0.087 0 255 0
0.087 0 255 0 0.176 0 255 255
0.176 0 255 255 0.268 0 0 255
0.268 0 0 255 0.577 255 0 255
0.577 255 0 255 1.192 255 0 0
EOF
fi

if [ ! -f profile.cpt ] ; then

cat > profile.cpt << EOF
F 0
N 255
0 255 255 255 		0.00001 255 255 0
0.00001 255 255 0 	0.0001 0 255 0
0.0001 0 255 0 		0.001 0 255 255
0.001 0 255 255 	0.01 0 0 255
0.01 0 0 255 		0.1 255 0 255
0.1 255 0 255	 	1 255 0 0
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

if [ ! -x maketif.sh ] ; then

cat > maketif.sh << EOF
#!/bin/sh

MAPNAME=\$1
SAMPLES=\$2

if [ -x ./\${MAPNAME}_tiff.cmd ] ; then
	./\${MAPNAME}_tiff.cmd
fi

if \`tiffinfo \$MAPNAME.tif  2> /dev/null | grep -q "Samples/Pixel: \$SAMPLES"\` ; then

	listgeo \$MAPNAME.tif > \$MAPNAME.geo

	convert -transparent white -depth 8 \$MAPNAME.tif trans_\$MAPNAME.tif
	geotifcp -c lzw -g \$MAPNAME.geo trans_\$MAPNAME.tif \$MAPNAME.tif
	rm trans_\$MAPNAME.tif 
	#rm \${MAPNAME}_tiff.cmd
else
	rm \$MAPNAME.tif 
fi
EOF

chmod +x maketif.sh
fi



#
# Process
#

dotif=`which convert`

lon=$[ $WEST * $factor ]
while [ $lon -lt $[ $east * $factor ] ] ; do
	nextlon=$[ $lon + $step ]
	if [ $nextlon -gt $[ $west * $factor ] ] ; then
	lat=$[ $SOUTH * $factor ]
	while [ $lat -lt $[ $north * $factor ] ] ; do
		nextlat=$[ $lat + $step ]
		if [ $nextlat -gt $[ $south * $factor ] ] ; then
		export REGION=`echo $lon $nextlon $lat $nextlat $factor |  awk '{ print ($1/$5) "/" ($2/$5) "/" ($3/$5) "/" ($4/$5) }'`
                region=`echo $REGION | sed s:/:_:g`
		prefix=${PREFIX}_${region}

		if [ -z "$RECOLOUR_ONLY" ] ; then
			mbdatalist -P -I$SWATHDIR -R$REGION >  ${prefix}_p_datalist.mb-1
			if [  `wc -l < ${prefix}_p_datalist.mb-1` = 0 ] ; then
 				rm  ${prefix}_p_datalist.mb-1
				[ -f ${prefix}.grd ] && rm ${prefix}.grd
				rm $step/*${region}* 2> /dev/null
			else
				mbdatalist -P -I${prefix}_p_datalist.mb-1 > ${prefix}_datalist.mb-1
				$GRIDCMD -I ${prefix}_datalist.mb-1 -O ${prefix} -R$REGION -D$PIXELS/$PIXELS $grid_extras
				if [ `grdinfo ${prefix}.grd | grep -c e+308` = 1 -o `grdinfo  ${prefix}.grd | grep -c "z_min: 0 z_max: 0 "` = 1 ] ; then
					rm ${prefix}.grd ${prefix}_datalist.mb-1
					rm $step/*${region}* 2> /dev/null
				fi
				rm ${prefix}*.cmd
			fi
		fi

		if [ -f ${prefix}.grd -a ! -z "$dotif" ] ; then

			if [ -z "$MPA_ONLY" ] ; then
				$TIFF1 -I ${prefix}.grd -O ${prefix}_${SUFFIX1}${TIFF1x}
				$TIFF2 -I ${prefix}.grd -O ${prefix}_${SUFFIX2}${TIFF2x}

				./maketif.sh ${prefix}_${SUFFIX1} $SAMPLES1
				./maketif.sh ${prefix}_${SUFFIX2} $SAMPLES2

				if [ ! -z "$TIFF3" ] ; then
					$TIFF3 -I ${prefix}.grd.slope -O ${prefix}_${SUFFIX3}${TIFF3x}
					./maketif.sh ${prefix}_${SUFFIX3} $SAMPLES3
				fi
			fi

			if [ "$filters" != 0 ] ; then
				grdmask *.gmt -m `grdinfo -I ${prefix}.grd` `grdinfo -I- ${prefix}.grd` -Gmpa_mask_${region}.grd -N0/1/1
				if [ `grdinfo mpa_mask_${region}.grd | grep -c "z_min: 0 z_max: 0 "` = 0 ] ; then
					grdmath ${prefix}.grd mpa_mask_${region}.grd DIV = ${PREFIX}_mpa_${region}.grd
					if [ `grdinfo ${PREFIX}_mpa_${region}.grd | grep -c "z_min: 0 z_max: 0 "` = 0 ] ; then
						$TIFF1 -I ${PREFIX}_mpa_${region}.grd -O ${PREFIX}_mpa_${region}_${SUFFIX1}${TIFF1x}
						$TIFF2 -I ${PREFIX}_mpa_${region}.grd -O ${PREFIX}_mpa_${region}_${SUFFIX2}${TIFF2x}
			                        ./maketif.sh ${PREFIX}_mpa_${region}_${SUFFIX1} $SAMPLES1
		        	                ./maketif.sh ${PREFIX}_mpa_${region}_${SUFFIX2} $SAMPLES2
						if [ ! -z "$SUFFIX3" ] ; then
							$TIFF3 -I ${PREFIX}_mpa_${region}.grd.slope -O ${PREFIX}_mpa_${region}_${SUFFIX3}${TIFF3x}
							./maketif.sh ${PREFIX}_mpa_${region}_${SUFFIX3} $SAMPLES3
							rm -f $step/${PREFIX}_mpa_${region}_${SUFFIX3}.tif 2> /dev/null
							ln -f ${PREFIX}_mpa_${region}_${SUFFIX3}.tif $step 2> /dev/null
						fi
						rm -f $step/${PREFIX}_mpa_${region}_${SUFFIX1}.tif 2> /dev/null
						rm -f $step/${PREFIX}_mpa_${region}_${SUFFIX2}.tif 2> /dev/null
						ln -f ${PREFIX}_mpa_${region}_${SUFFIX1}.tif $step 2> /dev/null
						ln -f ${PREFIX}_mpa_${region}_${SUFFIX2}.tif $step 2> /dev/null
					fi
					rm  mpa_mask_${region}.grd ${PREFIX}_mpa_${region}.grd ${PREFIX}_mpa_${region}.grd.int
				else
					rm  mpa_mask_${region}.grd
				fi
			fi

			if [ ! -z "$GDALADDO" ] ; then
	                        for type in ${SUFFIX1} ${SUFFIX2} ${SUFFIX3} ; do
			               gdaladdo --config COMPRESS_OVERVIEW JPEG --config PHOTOMETRIC_OVERVIEW YCBCR --config INTERLEAVE_OVERVIEW PIXEL -r nearest \
						${PREFIX}_${region}_${type}.tif 2 4 8 16 32 64
				done
			fi

			rm -f $step/${PREFIX}_${region}_${SUFFIX1}.tif 2> /dev/null
			rm -f $step/${PREFIX}_${region}_${SUFFIX2}.tif 2> /dev/null
			rm -f $step/${PREFIX}_${region}_${SUFFIX3}.tif 2> /dev/null
			ln -f ${PREFIX}_${region}_${SUFFIX1}.tif $step 2> /dev/null
			ln -f ${PREFIX}_${region}_${SUFFIX2}.tif $step 2> /dev/null
			ln -f ${PREFIX}_${region}_${SUFFIX3}.tif $step 2> /dev/null
			rm  ${prefix}.grd.int

                fi

		fi
		lat=$nextlat
	done
	fi
	lon=$nextlon
done

