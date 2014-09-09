#!/bin/bash  -x

# Script to process swath data to generate a mosaic of geotiffs 

# usage: 1degree.sh west east south north [step [ workdir ] ]
#       west - western limit of area to process in integer degrees.
#       east - eastern limit of area to process in integer degrees.
#       south - southern limit of area to process in integer degrees.
#       north - northern limit of area to process in integer degrees.
#       step - number of units to step (= pyramid level) default:1.
#       workdir - directory to do the work in - this will be cluttered with intermediate files.
#

WEST=66
SOUTH=-64
FACTOR=1
PIXELS=4096
SWATHDIR=/home/acoustics_swath/mbproc/em300/public_datalist.mb-1
TYPE=bathy

west=$1
east=$2
south=$3
north=$4
step=$5
workdir=$6

export WEST SOUTH FACTOR PIXELS SWATHDIR TYPE west east south north step workdir

/home/acoustics/swath/software/bin/swath_mosaic.sh
