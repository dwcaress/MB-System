#!/usr/bin/bash
#
#  Command to ensure a copy of all files under em300:/data1/raw
#  is kept in /swath4/raw
#
#  usage: copy_raw.sh [ SRC [DST [OLD]]]
#			SRC - directory to copy from - default /em300/data1/raw
#			DST - directory to copy to - default /swath4/raw
#			OLD - copy files newer than this file - $DEST/lastupdate
#
#  Author Gordon Keith - CSIRO Marine - 8 April 2004

SRC=$1
DST=$2
OLD=$3

[ -z "$SRC" ] && SRC=/em300/data1/raw
[ -z "$DST" ] && DST=/swath4/raw
[ -z "$OLD" ] && OLD=${DST}/lastupdate
[ -f $OLD ] && touch $OLD

cd $SRC
ls -d * */* */*/* */*/*/* 2> /dev/null | (
	while read file; do
	
		# if the file is a directory make sure we copy the directory structure
		if [ -d $SRC/$file -a ! -d $DST/$file ] ; then
			mkdir $DST/$file
		fi
	
		# if it is a real file and it's old enough and it's not already there
		# copy it
		if [ -f $SRC/$file ] ; then
			if [ $SRC/$file -ot $OLD ] ; then
				if [ ! -f $DST/$file -o $DST/$file -ot $SRC/$file ] ; then
					cp -p $SRC/$file $DST/$file
				fi
			fi 
		fi
	done
)

# This time is old enough for next run.
touch $OLD