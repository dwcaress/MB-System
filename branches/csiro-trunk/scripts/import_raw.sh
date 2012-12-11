#!/bin/sh -x

# script to import raw swath data into the mbproc data set.

if [ -z "$1" ] ; then
	echo "Usage: $0 survey [src dest]"
	echo " where survey is the voyage name"
	echo '       src is usually /home/swath/raw/survey or /home/MNF_multibeam/*survey/raw_em300 '
	echo "       dest is usually /home/acoustics_swath/mbproc/em300"
	exit 0
fi

if [ `id -gn` != echo-hf ] ; then
        sg echo-hf -c "$0 $*"
        exit 0
fi
umask 002

survey=$1
source="$2"

if [ -z "$source" ] ; then

	if [ -d /home/swath/raw/$survey ] ; then
		source=/home/swath/raw/$survey/
	else
		for src in /home/MNF_multibeam/*$survey*/raw_em300 ; do
			if [ -z "$source" ] ; then
				source="$src"
			else
				echo multiple directories: $source  $src
				echo "I'm confused, quitting"
				exit 1
			fi
		done
	fi

	if [ -z "$source" ] ; then
		echo Source directory for survey $survey not found
		exit 1
	fi

fi

dest=/home/acoustics_swath/mbproc/em300/
[ -z "$3" ] || dest=$3 

mkdir -p $dest/$survey
grep -q $survey $dest/datalist.mb-1 || echo $survey/datalist.mb-1  -1 1 >> $dest/datalist.mb-1

find $source -iname "*.all" | while read file ; do
	base=`basename $file .all`
	base=`basename $base _raw`
	out=$dest/$survey/${base}.mb57

	mbcopy -F56/57 -I $file -O $out

	echo `basename $out` 57 1  >> $dest/$survey/datalist.mb-1

	nice mbareaclean -I $out -F 57 -N-30 -T1 -D1/5
	nice mbclean -S10/3/2 -D.001/1 -I$out -F 57
done

sort -k2 -t_ datalist.mb-1 > datalist
mv datalist datalist.mb-1

mbdatalist -O -I $dest/$survey/datalist.mb-1

export SET=true
mb_process $dest/$survey/datalist.mb-1


